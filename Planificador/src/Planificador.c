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

char* puerto = "9001";

int main(void) {

	puts("Hola, soy el planificdor ;)");

	empezar_comunicacion_ESI();

	return EXIT_SUCCESS;
}

void empezar_comunicacion_ESI(){

	int server = crear_server(puerto);

	puts("Server creado");

	int cli = aceptar_conexion(server);

	char* recib = recibir_string(cli);

	printf("Recibi: %s",recib);

	free(recib);

}
