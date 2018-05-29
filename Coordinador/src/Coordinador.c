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
#include "msghandlers.h"

#define CFG_PORT  "listen_port"
#define CFG_ALGO  "distribution_algorithm"
#define CFG_ENTRYCANT  "entry_cant"
#define CFG_ENTRYSIZE  "entry_size"
#define CFG_DELAY  "delay"

#define CONTINUE_COMMUNICATION  1
#define END_CONNECTION -1

t_config* leer_config(int argc, char* argv[]) {
	int opt;
	t_config* config = NULL;
	opterr = 1; //ver getopt()

	while ((opt = getopt(argc, argv, "c:")) != -1) {
		switch (opt) {
		case 'c':
			printf("Levantando config: %s\n", optarg);
			config = config_create(optarg);
			break;
		case ':':
			fprintf(stderr, "El parametro '-%c' requiere un argumento.\n",
					optopt);
			break;
		case '?':
		default:
			fprintf(stderr, "El parametro '-%c' es invalido. Ignorado.\n",
					optopt);
			break;
		}
	}

	return config;
}

int config_incorrecta(t_config* config) {
	if (config == NULL) {
		// PARA CORRER DESDE ECLIPSE
		// AGREGAR EN "Run Configurations.. > Arguments"
		// -c ${workspace_loc:/Coordinador/src/coord.cfg}
		puts("El parametro -c <config_file> es obligatorio.\n");
		return EXIT_FAILURE;
	}

	int failures = 0;
	void validar(char* key) { //TODO validar tipos?
		if (!config_has_property(config, key)) {
			printf("Se requiere configurar \"%s\"\n", key);
			failures++;
		}
	}

	validar(CFG_PORT);
	validar(CFG_ALGO);
	validar(CFG_ENTRYCANT);
	validar(CFG_ENTRYSIZE);
	validar(CFG_DELAY);

	if (failures > 0) {
		printf("Por favor revisar el archivo \"%s\"\n", config->path);
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

	int continue_communication = true;

	switch (paquete->codigo_operacion) {
	case HANDSHAKE_ESI:
	case HANDSHAKE_INSTANCIA:
	case HANDSHAKE_PLANIFICADOR:
		registrar_conexion(conexiones, socket, paquete->codigo_operacion);
		do_handhsake(socket);
		return CONTINUE_COMMUNICATION;
	}

	//Si llegamos hasta aca, no es un handshake. Validamos
	if (conexion_invalida(conexiones, socket, HANDSHAKE_ESI)) {
		return END_CONNECTION;
	}

	//Ok, cool cool. Procesamos mensaje
	switch (paquete->codigo_operacion) {
	case OPERACION_GET:
	case OPERACION_SET:
	case OPERACION_STORE:
		do_esi_request(conexiones, socket, paquete);
		break;
	case STRING_SENT: {
		do_dummy_string(socket, paquete);
		break;
	}
	default: //WTF? no gracias.
		destruir_paquete(paquete);
		return END_CONNECTION;
	}
	destruir_paquete(paquete);
	return CONTINUE_COMMUNICATION;
}

int main(int argc, char* argv[]) {
	t_config* config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
		config_destroy(config);
		return EXIT_FAILURE;
	}

	printf("Comenzando a atender peticiones.");
	multiplexar(config_get_string_value(config, CFG_PORT),
			(void*) recibir_mensaje);
	return EXIT_SUCCESS;
}
