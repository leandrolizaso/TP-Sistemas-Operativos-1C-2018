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
#include <protocolo.h>

int main(void) {
	char* mensaje_recibido;

	puts("!!!Soy el proceso instancia!!!");


	int conexion_coordinador = conectar_a_server("127.0.0.1","9999");
	mensaje_recibido = recibir_string(conexion_coordinador);
	if(conexion_coordinador == HANDSHAKE_COORDINADOR){
		printf("recibi \"%s\" del coordinador",mensaje_recibido);
		enviar_string(conexion_coordinador, HANDSHAKE_ESI);
	}else{
		printf("recibi \"%s\" del coordinador otro mesaje por error",mensaje_recibido);
		enviar_string(conexion_coordinador, STRING_SENT);
	}

	free(mensaje_recibido);
	return EXIT_SUCCESS;
}
