/*
 ============================================================================
 Name        : Instancia.c
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
	char* mensaje_recibido;

	puts("!!!Soy el proceso instancia!!!");


	int conexion_coordinador = conectar_a_server("127.0.0.1","9999");
	mensaje_recibido = recibir_string(conexion_coordinador);
	printf("recibi \"%s\" del coordinador",mensaje_recibido);

	enviar_string(conexion_coordinador, "soy la instancia ameo");

	free(mensaje_recibido);
	return EXIT_SUCCESS;
}
