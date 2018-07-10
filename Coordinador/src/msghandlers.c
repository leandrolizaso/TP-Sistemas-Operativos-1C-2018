#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "Coordinador.h"
#include "msghandlers.h"
#include "shared.h"

extern t_log* log_operaciones;

extern int socket_planificador;
extern char* ip_planificador;
extern char* puerto_planificador;

extern t_config* config;
extern t_dictionary* claves;
extern t_list* instancias;

char* keywordtos(int keyword) {
	switch (keyword) {
	case 0:
		return "GET";
	case 1:
		return "SET";
	case 2:
		return "STORE";
	default:
		return "WTF";
	}
}

t_instancia* key_explicit(char* clave) {
	int cant_instancias = list_size(instancias);
	int caracter = *clave;
	caracter = caracter - 97;
	int rango = (26 / cant_instancias) + 1;
	int n_instancia = caracter / rango;
	return list_get(instancias, n_instancia);
}

t_instancia* equitative_load(char* clave) {
	static int next_instance = 0;
	next_instance++;
	if (next_instance > list_size(instancias)) {
		next_instance = 0;
	}
	return list_get(instancias, next_instance);
}

t_instancia* least_space_used(char* clave) {

	//funcion local
	bool ordenar_libre(void* a, void* b) {
		t_instancia* inst1 = a;
		t_instancia* inst2 = b;
		return inst1->ocupado <= inst2->ocupado;
	}

	list_sort(instancias, ordenar_libre);
	return list_get(instancias, 0);
}

void* do_handhsake(void* args) {
	t_params* params = args;
	int socket = params->socket;
	t_paquete* paquete = params->paquete;
	loggear("info", "Handshake Recibido (%d). Enviando Handshake.\n", socket);

	int tamanio = 0;
	void* data = NULL;

	switch (paquete->codigo_operacion) {
	case HANDSHAKE_ESI:
		break;
	case HANDSHAKE_PLANIFICADOR: {
		ip_planificador = get_ip_socket(socket);
		strcpy(puerto_planificador, paquete->data);
		break;
	}
	case HANDSHAKE_INSTANCIA: {
		registrar_instancia(socket, strdup(paquete->data));

		tamanio = sizeof(int) * 2;
		data = malloc(tamanio);
		int cant_entradas = config_get_int_value(config, CFG_ENTRYCANT);
		int size_entradas = config_get_int_value(config, CFG_ENTRYSIZE);
		memcpy(data, &cant_entradas, sizeof(int));
		memcpy(data + sizeof(int), &size_entradas, sizeof(int));
		break;
	}
	default:
		break;
	}
	enviar(socket, HANDSHAKE_COORDINADOR, tamanio, data);
	destruir_paquete(params->paquete);
	free(params);
	free(data);
	return NULL;
}

void* do_esi_request(void* args) {
	t_params* params = args;
	int socket_esi = params->socket;
	t_mensaje_esi mensaje_esi = deserializar_mensaje_esi(params->paquete->data);
	destruir_paquete(params->paquete);
	free(params);

	char* valor_mostrable = mensaje_esi.clave_valor.valor;
	char* clave = mensaje_esi.clave_valor.clave;
	if (valor_mostrable == NULL) { //hack para no ver "(null)" en los log de operaciones
		valor_mostrable = "";
	}

	loggear("info", "ESI %d\t%s %s %s\n", mensaje_esi.id_esi,
			keywordtos(mensaje_esi.keyword), mensaje_esi.clave_valor.clave,
			valor_mostrable);

	/* Este logger no debe ser wrappeado por loggear*/
	log_info(log_operaciones, "ESI %d\t%s %s %s\n", mensaje_esi.id_esi,
			keywordtos(mensaje_esi.keyword), mensaje_esi.clave_valor.clave,
			valor_mostrable);
	/* Asi me aseguro de tener un archivo limpio*/

	loggear("trace", "Simulamos espera para ESI%d...", mensaje_esi.id_esi);
	sleep(config_get_int_value(config, CFG_DELAY) / 1000 + 1);
	loggear("trace", "Seguimos!");

	int operacion; //Este switch es hasta que el planificador pueda usar el mensaje polimorficamente
	switch (mensaje_esi.keyword) {
	case 0:
		operacion = GET_CLAVE;
		break;
	case 1:
		operacion = SET_CLAVE;
		break;
	case 2:
		operacion = STORE_CLAVE;
		break;
	default:
		loggear("error", "Se recibio una operacion invalida del ESI %d: %d",
				mensaje_esi.id_esi, mensaje_esi.keyword);
	}

	if (!dictionary_has_key(claves, clave) && mensaje_esi.keyword != 0) {
		enviar(socket_esi, ERROR_OPERACION, 41,
				"No podes hacer eso si la clave no existe");
	} else if (!dictionary_has_key(claves, clave) && mensaje_esi.keyword == 0) {
		dictionary_put(claves, clave, NULL);
	}

	if (socket_planificador == -1) { //solo la primera conexion al planificador
		socket_planificador = conectar_a_server(ip_planificador,
				puerto_planificador);
	}

	int tamanio = strlen_null(clave);
	enviar(socket_planificador, operacion, tamanio, clave);

	t_paquete* paquete = recibir(socket_planificador);
	switch (paquete->codigo_operacion) {
	case OPERACION_ESI_VALIDA: {
		if (mensaje_esi.keyword == 0) {
			enviar(socket_esi, EXITO_OPERACION, 0, NULL);
			break;
		}

		int resultado_error = instancia_guardar(mensaje_esi.keyword,
				mensaje_esi.clave_valor);

		if (resultado_error) {
			//TODO: que mensaje mandar?
			enviar(socket_esi, ERROR_OPERACION, 0, NULL);
		} else {
			enviar(socket_esi, EXITO_OPERACION, 0, NULL);
		}
		break;
	}
	case OPERACION_ESI_INVALIDA:
		enviar(socket_esi, ERROR_OPERACION, paquete->tamanio, paquete->data);
		break;
	}
	destruir_paquete(paquete);
	return NULL;
}

int instancia_guardar(int keyword, t_clavevalor cv) {
	t_clave* clave = dictionary_get(claves, cv.clave);

	if (clave->instancia == NULL) {
		if (list_size(instancias) <= 0) {
			loggear("warning",
					"Todavia no se registraron instancias, por lo que no se puede atender al ESI.");
			return EXIT_FAILURE;
		}

		char* algoritmo = config_get_string_value(config, CFG_ALGO);

		if (strcmp(algoritmo, "LSU")) {
			clave->instancia = least_space_used(cv.clave);
		} else if (strcmp(algoritmo, "EL")) {
			clave->instancia = equitative_load(cv.clave);
		} else if (strcmp(algoritmo, "KE")) {
			clave->instancia = key_explicit(cv.clave);
		}
	}

	if (keyword == 1) { //SET

		enviar(clave->instancia->fd, SAVE_CLAVE, sizeof_clavevalor(cv),
				serializar_clavevalor(cv));
		t_paquete* paquete = recibir(clave->instancia->fd);

		switch (paquete->codigo_operacion) {
		case NEED_COMPACTAR: {
			//levantar un hilo para cada instancia
			//enviarles "COMPACTA"
			//join de todos esos hilos
			return instancia_guardar(keyword, cv);
		}
		case RESPUESTA_INTANCIA: {
			int entradas_ocupadas = strlen_null(cv.valor)
					/ config_get_int_value(config, CFG_ENTRYSIZE);
			clave->entradas = entradas_ocupadas;
			clave->instancia->ocupado += entradas_ocupadas;

			if (paquete->tamanio > 0) {
				//hubo reemplazos
				//deserializar las claves, separando por '\0'
				//por cada unas de esas
				//    clave_reemplazada = list_find(claves,...);
				//    clave->instancia->ocupado -= clave_reemplazada->entradas
				//    clave->instancia = NULL;
			}
			return EXIT_SUCCESS;
		}
		}
	} else { //STORE
		enviar(clave->instancia->fd, DUMP_CLAVE, strlen_null(cv.clave),
				cv.clave);
		t_paquete* paquete = recibir(clave->instancia->fd);
		destruir_paquete(paquete);
		return EXIT_SUCCESS;
	}

}
