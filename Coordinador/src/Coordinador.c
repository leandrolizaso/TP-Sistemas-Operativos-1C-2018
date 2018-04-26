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
#include <sockets.h>
#include <protocolo.h>
#include <commons/config.h>


#define CFG_PORT  "listen_port"
#define CFG_ALGO  "distribution_algorithm"
#define CFG_ENTRYCANT  "entry_cant"
#define CFG_ENTRYSIZE  "entry_size"
#define CFG_DELAY  "delay"

 t_config* leer_config(int argc, char* argv[]) {
	int opt;
	t_config* config = NULL;
	opterr = 1; //ver getopt()

	while ((opt = getopt(argc, argv, "c:")) != -1) {
		switch (opt) {
		case 'c':
			printf("Levantando config... %s\n", optarg);
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

int main(int argc, char* argv[]) {
	t_config* config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
		config_destroy(config);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
int old(int argc, char* argv[]) {
	char* puerto = "9999";
	char* handshake = "coord_says_hi";
	char* mensaje_recibido;

	puts("Levantando..");

	puts("Esperando conexion");
	int client_socket = aceptar_conexion(server_socket);

	puts("Conexion recibida, enviando handshake");
	int chars_sent = enviar_string(client_socket, handshake);
	if (strlen(handshake) != chars_sent) {
		puts("Opa! algo sucedio y no pude mandar handshake");
		return EXIT_FAILURE;
	}
	mensaje_recibido = recibir_string(client_socket);
	printf("Acabo de recibir %s... Eso quiere decir que estamos?\n",
			mensaje_recibido);

	free(mensaje_recibido);

	return EXIT_SUCCESS;
}
*/
