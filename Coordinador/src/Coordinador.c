/*
 ============================================================================
 Name        : Coordinador.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include "Coordinador.h"
#include "msghandlers.h"
#include "shared.h"

t_log* log_operaciones;
t_log* log_app;
t_log* log_consola;

int socket_planificador = -1;
char* ip_planificador = NULL;
char* puerto_planificador = NULL;

t_config* config;
t_dictionary* claves;
t_list* instancias;

int main(int argc, char* argv[]) {
	log_consola = log_create(NULL, "COORDINADOR", true, LOG_LEVEL_INFO);
	log_app = log_create("coordinador.log", "COORDINADOR", false,
			LOG_LEVEL_TRACE);
	log_operaciones = log_create("operaciones.log", "COORDINADOR", false,
			LOG_LEVEL_DEBUG);

	puerto_planificador = malloc(20 * sizeof(char));
	claves = dictionary_create();
	instancias = list_create();
	inicializar_sincronizacion();

	config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
		config_destroy(config);
		return EXIT_FAILURE;
	}

	loggear("debug","el algoritmo es %s",config_dist_algo());

	char* port = config_get_string_value(config, CFG_PORT);
	loggear("info", "Comenzando a atender peticiones en el puerto %s", port);

	multiplexar(port, recibir_mensaje);

	finalizar_sincronizacion();
	log_destroy(log_app);
	log_destroy(log_operaciones);
	free(puerto_planificador);
	free(ip_planificador);
	free(port);
	dictionary_destroy(claves);
	list_destroy_and_destroy_elements(instancias, (void*) destruir_instancia);
	config_destroy(config);

	printf("Finalizado");
	return EXIT_SUCCESS;
}

t_config* leer_config(int argc, char* argv[]) {
	int opt;
	t_config* config = NULL;
	opterr = 1; //ver getopt()

	while ((opt = getopt(argc, argv, "c:")) != -1) {
		switch (opt) {
		case 'c':
			loggear("info", "Levantando config: %s", optarg);
			config = config_create(optarg);
			break;
		case ':':
			loggear("error", "El parametro '-%c' requiere un argumento.",
					optopt);
			break;
		case '?':
		default:
			loggear("error", "El parametro '-%c' es invalido. Ignorado.",
					optopt);
			break;
		}
	}

	if (config == NULL) {
		loggear("warning",
				"El parametro -c <config_file> es obligatorio. Se cargará la configuracion por defecto.");
		config = config_create("src/coord.cfg");
	}

	return config;
}

int config_incorrecta(t_config* config) {
	int failures = 0;

	void validar(char* key) { //TODO validar tipos?
		if (!config_has_property(config, key)) {
			loggear("error", "Se requiere configurar \"%s\"", key);
			failures++;
		}
	}

	validar(CFG_PORT);
	validar(CFG_ALGO);
	validar(CFG_ENTRYCANT);
	validar(CFG_ENTRYSIZE);
	validar(CFG_DELAY);

	if (failures > 0) {
		loggear("error", "Por favor revisar el archivo \"%s\"", config->path);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int recibir_mensaje(int socket) {
	loggear("debug", "recibiendo mensaje de socket %d", socket);
	static t_dictionary* conexiones = NULL;
	if (conexiones == NULL) {
		conexiones = dictionary_create();
	}

	t_paquete* paquete = recibir(socket);

	t_params* params = malloc(sizeof(t_params));
	params->socket = socket;
	params->paquete = paquete;

	int codigo_operacion = paquete->codigo_operacion;

	switch (codigo_operacion) {
	case HANDSHAKE_ESI:
	case HANDSHAKE_INSTANCIA:
	case HANDSHAKE_PLANIFICADOR: {
		pthread_t hilito;
		pthread_create(&hilito, NULL, do_handhsake, params);
		pthread_detach(hilito);
		if (codigo_operacion == HANDSHAKE_INSTANCIA) {
			return LET_ME_HANDLE_IT;
		} else {
			return CONTINUE_COMMUNICATION;
		}
		break;
	}
	case OPERACION: {
		pthread_t hilito;
		pthread_create(&hilito, NULL, do_esi_request, params);
		pthread_detach(hilito);
		return CONTINUE_COMMUNICATION;
		break;
	}
	case STATUS: {
		pthread_t hilito;
		pthread_create(&hilito, NULL, do_status_request, params);
		pthread_detach(hilito);
		return CONTINUE_COMMUNICATION;
		break;
	}
	default: {
		loggear("warning",
				"Epa! parece que se desconectaron del socket %d?\ncodigo_operacion:%d\ttamaño:%d",
				socket, paquete->codigo_operacion, paquete->tamanio);
		destruir_paquete(paquete);
		free(params);
		return END_CONNECTION;
	}
	}
}

void loggear(char* level_char, char* template, ...) {
	va_list args;
	va_start(args, template);

	char* texto = string_from_vformat(template, args);

	switch (log_level_from_string(level_char)) {
	//Si _log_write_in_level no fuera privada, usaria eso.
	case LOG_LEVEL_ERROR:
		log_error(log_app, texto);
		log_error(log_consola, texto);
		break;
	case LOG_LEVEL_WARNING:
		log_warning(log_app, texto);
		log_warning(log_consola, texto);
		break;
	case LOG_LEVEL_INFO:
		log_info(log_app, texto);
		log_info(log_consola, texto);
		break;
	case LOG_LEVEL_DEBUG:
		log_debug(log_app, texto);
		log_debug(log_consola, texto);
		break;
	case LOG_LEVEL_TRACE:
		log_trace(log_app, texto);
		log_trace(log_consola, texto);
		break;
	}

	free(texto);
	va_end(args);
}

