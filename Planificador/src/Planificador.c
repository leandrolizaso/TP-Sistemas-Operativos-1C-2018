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

	char* puerto = "8000";

	int server = crear_server(puerto);

	int acep = aceptar_conexion(server);

	char* recib = recibir_string(acep);

	printf("Recibi: %s",recib);


	return EXIT_SUCCESS;
}
