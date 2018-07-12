#include <stdbool.h>
#include <pthread.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/string.h>
#include "Coordinador.h"
#include "msghandlers.h"
#include "shared.h"

extern t_log* log_operaciones;

extern int socket_planificador;
extern char* ip_planificador;
extern char* puerto_planificador;

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
	int caracter = *clave;
	caracter = caracter - 97;
	int rango = (26 / cant_instancias()) + 1;
	int n_instancia = caracter / rango;
	return obtener_instancia(n_instancia);
}

t_instancia* equitative_load(char* clave) {
	static int next_instance = 0;
	next_instance++;
	if (next_instance > cant_instancias()) {
		next_instance = 0;
	}
	return obtener_instancia(next_instance);
}

t_instancia* least_space_used(char* clave) {

	//funcion local
	bool ordenamiento_espacio_libre(void* a, void* b) {
		t_instancia* inst1 = a;
		t_instancia* inst2 = b;
		return inst1->libre >= inst2->libre;
	}

	ordenar_instancias_segun(ordenamiento_espacio_libre);
	return obtener_instancia(0);
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
		registrar_instancia(socket, paquete->data);

		tamanio = sizeof(int) * 2;
		data = malloc(tamanio);
		int cant_entradas = config_entry_cant();
		int size_entradas = config_entry_size();
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
	usleep(config_delay() * 1000);
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
		enviar(socket_esi, ERROR_OPERACION, 34,
				"Se recibio una operacion invalida");
		return NULL;
	}

	if (no_existe_clave(clave) && mensaje_esi.keyword != 0) {
		enviar(socket_esi, ERROR_OPERACION, 41,
				"No podes hacer eso si la clave no existe");
	} else if (no_existe_clave(clave) && mensaje_esi.keyword == 0) {
		registrar_clave(clave);
	}

	if (socket_planificador == -1) { //solo la primera conexion al planificador
		socket_planificador = conectar_a_server(ip_planificador,
				puerto_planificador);
	}

	int tamanio = strlen_null(clave);
	loggear("trace", "enviando al planificador operacion %d clave \"%s\"",
			operacion, clave);
	enviar(socket_planificador, operacion, tamanio, clave);
	t_paquete* paquete = recibir(socket_planificador);
	loggear("trace",
			"recibido de planificador status %d (%d esi_valida, %d esi_no_valida).",
			paquete->codigo_operacion, OPERACION_ESI_VALIDA,
			OPERACION_ESI_INVALIDA);
	switch (paquete->codigo_operacion) {
	case OPERACION_ESI_VALIDA: {
		if (mensaje_esi.keyword == 0) {
			enviar(socket_esi, EXITO_OPERACION, 0, NULL);
			break;
		}

		char* resultado_error = instancia_guardar(mensaje_esi.keyword,
				mensaje_esi.clave_valor);
		loggear("trace", "el resultado de insancia fue: %s", resultado_error);

		if (resultado_error) {
			loggear("debug", "enviando al ESI ese error");
			enviar(socket_esi, ERROR_OPERACION, strlen_null(resultado_error),
					resultado_error);
		} else {
			loggear("debug", "enviando al ESI un exito");
			enviar(socket_esi, EXITO_OPERACION, 0, NULL);
		}
		free(resultado_error);
		break;
	}
	case OPERACION_ESI_INVALIDA:
		loggear("trace", "mandando ERROR_OPERACION por OPERACION_ESI_INVALIDA");
		enviar(socket_esi, ERROR_OPERACION, paquete->tamanio, paquete->data);
		break;
	}
	destruir_paquete(paquete);
	loggear("debug", "do_esi_request finalizado");
	return NULL;
}

char* instancia_guardar(int keyword, t_clavevalor cv) {
	loggear("trace", "Enviando a instancia %s %s %s", keywordtos(keyword),
			cv.clave, cv.valor);
	t_clave* clave = obtener_clave(cv.clave);

	//Si no tiene una instancia, trato de asignarle una
	if (clave->instancia == NULL) {
		loggear("debug", "Eligiendo instancia segun algoritmo");
		if (cant_instancias() <= 0) {
			return string_from_format(
					"Todavia no se registraron instancias, por lo que no se puede atender al ESI.");
		}

		char* algoritmo = config_dist_algo();

		if (strcmp(algoritmo, "LSU")) {
			loggear("debug", "Algoritmo Least Space Used");
			clave->instancia = least_space_used(cv.clave);
		} else if (strcmp(algoritmo, "EL")) {
			loggear("debug", "Algoritmo Equitative Load");
			clave->instancia = equitative_load(cv.clave);
		} else if (strcmp(algoritmo, "KE")) {
			loggear("debug", "Algoritmo Key Explicit");
			clave->instancia = key_explicit(cv.clave);
		}

		if (clave->instancia == NULL) {
			return string_from_format(
					"Error en algoritmo de distribucion. No se pudo asignar una instancia a la clave \"%s\"",
					cv.clave);
		}
	}

	if (keyword == 1) { //SET
		int entradas_ocupadas = (strlen(cv.valor) - 1) / config_entry_size()
				+ 1;

		loggear("trace", "La clave %s ocupa %d entradas. Se le avisa a %s",
				cv.clave, entradas_ocupadas, clave->instancia->nombre);
		enviar(clave->instancia->fd, HAS_ESPACIO, sizeof(int),
				&entradas_ocupadas);
		loggear("debug", "Esperando veredicto. Entra?");
		t_paquete* paquete = recibir(clave->instancia->fd);
		loggear("trace", "Resulta que %d hay espacio (%d Si, %d No).",
		OK_ESPACIO,
		NO_ESPACIO);
		int operacion = paquete->codigo_operacion;
		destruir_paquete(paquete);

		if (operacion == NO_ESPACIO) {
			return string_from_format(
					"La instancia %s no tiene espacio suficiente.\nNecesario: %d",
					clave->instancia->nombre, entradas_ocupadas);

		}

		loggear("trace", "enviando SAVE_CLAVE %s %s.", cv.clave, cv.valor);
		enviar(clave->instancia->fd, SAVE_CLAVE, sizeof_clavevalor(cv),
				serializar_clavevalor(cv));
		loggear("trace",
				"esperando resultado de la instancia (fd:%d, nombre:%s)...",
				clave->instancia->fd, clave->instancia->nombre);
		paquete = recibir(clave->instancia->fd);
		loggear("trace",
				"recibido de instancia status: %d (%d need_compactar, %d OK",
				paquete->codigo_operacion, NEED_COMPACTAR, RESPUESTA_INTANCIA);

		switch (paquete->codigo_operacion) {
		case NEED_COMPACTAR: {
			loggear("debug", "Instancia %s me pide compactar!",
					clave->instancia->nombre);
			pthread_t hilos_instancia[cant_instancias()];

			void* compactar(void* args) {
				int* indice = args;
				t_instancia* inst = obtener_instancia(*indice);
				loggear("trace", "compactando en instancia %s", inst->nombre);
				enviar(inst->fd, COMPACTA, 0, NULL);
				destruir_paquete(recibir(inst->fd));
				return NULL;
			}

			loggear("debug", "iniciando hilos para compactar");
			for (int i = 0; i < cant_instancias(); i++) {
				pthread_create(&hilos_instancia[i], NULL, compactar, &i);
			}
			loggear("debug", "joining hilos de compaco realizado");
			for (int i = 0; i < cant_instancias(); i++) {
				pthread_join(hilos_instancia[i], NULL);
			}
			loggear("debug", "hilos joineados, se intenta guardar nuevamente");
			return instancia_guardar(keyword, cv);
		}
		case RESPUESTA_INTANCIA: {
			loggear("debug", "RESPUESTA_INSTANCIA");
			loggear("trace", "La instancia %s tenia %d libre antes de comenzar.",
					clave->instancia->nombre, clave->instancia->libre);
			clave->entradas = entradas_ocupadas;
			clave->instancia->libre -= clave->entradas;
			loggear("trace", "Al descontarle la clave guardada quedo %d.",
					clave->instancia->libre);

			if (paquete->tamanio > 0) {
				loggear("debug", "hubo reemplazos");

				//deserializar los reemplazos, separando por '\0'
				int leido = 0;
				char* reemplazo = paquete->data;
				while (leido < paquete->tamanio) {
					t_clave* clave_reemplazada = obtener_clave(reemplazo);

					loggear("trace",
							"Descontando en %s %d entradas de clave reemplazada %s.",
							clave_reemplazada->instancia->nombre,
							clave_reemplazada->entradas,
							clave_reemplazada->clave);

					clave_reemplazada->instancia->libre +=
							clave_reemplazada->entradas;
					clave_reemplazada->instancia = NULL;

					int step = strlen_null(reemplazo);
					leido += step;
					reemplazo += step;
				}

			}
			return NULL;
		}
		default:
			return string_from_format(
					"Se obtuvo un mensaje inesperado de la instancia: %d",
					paquete->codigo_operacion);
		}
	} else { //STORE
		loggear("trace", "enviando STORE_CLAVE \"%s\" a instancia.", cv.clave);
		enviar(clave->instancia->fd, DUMP_CLAVE, strlen_null(cv.clave),
				cv.clave);
		loggear("trace", "esperando respuesta de instancia...");
		destruir_paquete(recibir(clave->instancia->fd));
		loggear("trace", "listo, seguimos.");
		return NULL;
	}

}
