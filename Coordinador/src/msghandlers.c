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

void destruir_instancia(t_instancia* instancia) {
	free(instancia->nombre);
	free(instancia);
}

t_instancia* key_explicit(char* clave) {
	int cant_instancias = list_size(instancias); //TODO: contar solo las intancias conectadas
	if (cant_instancias > 0) {
		int caracter = *clave;
		caracter = caracter - 97;
		int rango = (26 / cant_instancias) + 1; //TODO: consultar sobre este +1
		int n_instancia = caracter / rango;
		return list_get(instancias, n_instancia);
	}
	return NULL;
}

void do_handhsake(int socket, t_paquete* paquete) {
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
		t_instancia* instancia_nueva = malloc(sizeof(t_instancia));
		instancia_nueva->fd = socket;
		instancia_nueva->nombre = strdup(paquete->data);
		registrar_instancia(instancia_nueva);

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
	free(data);
}

void do_esi_request(int socket_esi, t_mensaje_esi mensaje_esi) {
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
}

int instancia_guardar(int keyword, t_clavevalor cv) {
	t_instancia* instancia = dictionary_get(claves, cv.clave);
	if (instancia == NULL) {
		//TODO: seleccion de algoritmo
		instancia = key_explicit(cv.clave);
		if (instancia == NULL) {
			//no hay instancia para atender esta peticion;
			return EXIT_FAILURE;
		}
		//if instancia desconectada
		//then eliminar clave
		//return fallo
	}

	int tamanio;
	void* buff;
	int operacion;
	switch (keyword) {
	case 1: {
		tamanio = sizeof_clavevalor(cv);
		buff = serializar_clavevalor(cv);
		operacion = SAVE_CLAVE;
		break;
	}
	case 2: { //store
		tamanio = strlen_null(cv.clave);
		buff = cv.clave;
		operacion = DUMP_CLAVE;
		break;
	}
	}

	enviar(instancia->fd, operacion, tamanio, buff);
	t_paquete* paquete = recibir(instancia->fd);
	//TODO: validar si guardo o no
	//if guardo exitosamente
	dictionary_put(claves, cv.clave, instancia);
	//else if no guardo
	// if no guardo por falta de espacio
	// then return fallo;
	// else if no guardo por tener espacio pero necesitar defrag
	// then enviar defrag a todas las intancias y retornar "instancia_guardar" (recursivo)
	destruir_paquete(paquete);
	return EXIT_SUCCESS;
}
