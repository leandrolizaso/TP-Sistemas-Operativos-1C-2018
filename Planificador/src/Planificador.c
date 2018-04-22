/*
 ============================================================================
 Name        : Planificador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sockets.h>

int main(void) {

	char* puerto = "8001";

	int server = crear_server(puerto);

	int cli = aceptar_conexion(server);

	char* recib = recibir_string(cli);

	printf("Recibi: %s",recib);

	free(recib);

	return EXIT_SUCCESS;
}
