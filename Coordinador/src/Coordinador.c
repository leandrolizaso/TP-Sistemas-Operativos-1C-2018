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
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include "Coordinador.h"
#include "msghandlers.h"

t_log* log_operaciones;
t_log* log_app;

int socket_planificador = -1;
char* ip_planificador;
char* puerto_planificador;

t_config* config;
t_dictionary* claves;
t_list* instancias;

int main(int argc, char* argv[]) {
	//TODO: el log level seria parametro (por archivo de config?)
	log_app = log_create("coordinador.log", "COORDINADOR", true,
			LOG_LEVEL_TRACE);
	log_operaciones = log_create("operaciones.log", "COORDINADOR", false,
			LOG_LEVEL_INFO);

	puerto_planificador = malloc(20 * sizeof(char));
	claves = dictionary_create();
	instancias = list_create();

	config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
		config_destroy(config);
		return EXIT_FAILURE;
	}

	char* port = config_get_string_value(config, CFG_PORT);
	log_info(log_app, "Comenzando a atender peticiones en el puerto %s", port);
	multiplexar(port, recibir_mensaje);

	log_destroy(log_app);
	log_destroy(log_operaciones);
	free(puerto_planificador);
	free(ip_planificador);
	free(port);
	dictionary_destroy(claves);
	list_destroy_and_destroy_elements(instancias, destruir_meta_instancia);
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
			log_info(log_app, "Levantando config: %s", optarg);
			config = config_create(optarg);
			break;
		case ':':
			log_error(log_app, "El parametro '-%c' requiere un argumento.",
					optopt);
			break;
		case '?':
		default:
			log_error(log_app, "El parametro '-%c' es invalido. Ignorado.",
					optopt);
			break;
		}
	}

	return config;
}

int config_incorrecta(t_config* config) {
	if (config == NULL) {
		log_warning(log_app,
				"El parametro -c <config_file> es obligatorio. Se intentará cargar la configuracion por defecto.");
		config = config_create("./src/coord.cfg");
	}

	int failures = 0;

	void validar(char* key) { //TODO validar tipos?
		if (!config_has_property(config, key)) {
			log_error(log_app, "Se requiere configurar \"%s\"", key);
			failures++;
		}
	}

	validar(CFG_PORT);
	validar(CFG_ALGO);
	validar(CFG_ENTRYCANT);
	validar(CFG_ENTRYSIZE);
	validar(CFG_DELAY);

	if (failures > 0) {
		log_error(log_app, "Por favor revisar el archivo \"%s\"", config->path);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int recibir_mensaje(int socket) {
	static t_dictionary* conexiones = NULL;
	if (conexiones == NULL) {
		conexiones = dictionary_create();
	}

	t_paquete* paquete = recibir(socket);

	switch (paquete->codigo_operacion) {
	case HANDSHAKE_ESI:
	case HANDSHAKE_INSTANCIA:
	case HANDSHAKE_PLANIFICADOR:
		do_handhsake(socket, paquete);
		break;
	case OPERACION:
		do_esi_request(socket, deserializar_mensaje_esi(paquete->data));
		break;
	default:
		//TODO tratar mejor las desconexiones (si es instancia marcar desconectada)
		destruir_paquete(paquete);
		return END_CONNECTION;
	}
	destruir_paquete(paquete);
	return CONTINUE_COMMUNICATION;
}

