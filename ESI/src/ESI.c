/*
 ============================================================================
 Name        : ESI.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sockets.h>

char* ip_planificador = "127.0.0.1";
char* puerto_planificador = "9001";
//char* ip_coordinador = "127.0.0.1";
//int puerto_coordinador = 8000;

int main(void) {

	puts("!!!Soy el proceso ESI!!!");

	empezar_comunicacion_Planificador();

	return EXIT_SUCCESS;
}

void empezar_comunicacion_Planificador(){
	int socket_cliente = conectar_a_server((char *)ip_planificador,(char *) puerto_planificador);

	puts("Esperando accept del Planificador");

	int result = enviar_string(socket_cliente, "Hola, soy un ESI");

	if(result != strlen("Hola, soy un ESI")){
			puts("Error al enviar");
		};
}
