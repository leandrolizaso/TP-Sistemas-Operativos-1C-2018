/*
 ============================================================================
 Name        : Coordinador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sockets.h>

int main(int argc, char* argv[]) {
	char* puerto = "9999";
	char* handshake = "coord_says_hi";
	char* mensaje_recibido;

	puts("Levantando..");
	int server_socket = crear_server(puerto);

	puts("Esperando conexion");
	int client_socket = aceptar_conexion(server_socket);

	puts("Conexion recibida, enviando handshake");
	int chars_sent = enviar_string(client_socket,handshake);
	if (strlen(handshake) != chars_sent) {
		puts("Opa! algo sucedio y no pude mandar handshake");
		return EXIT_FAILURE;
	}
	mensaje_recibido = recibir_string(client_socket);
	printf("Acabo de recibir %s... Eso quiere decir que estamos?\n",mensaje_recibido);

	free(mensaje_recibido);

	return EXIT_SUCCESS;
}
