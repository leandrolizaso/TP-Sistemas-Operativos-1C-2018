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
#include <pthread.h>
#include "protocolo.h"

char* ip_coordinador = "127.0.0.1";
int puerto_coordinador = 8000;
int socket_coordinador = conectar_a_server((char *)ip_coordinador,(char *) puerto_coordinador);

char* puerto = "8010";


void agregar_espacio(char* buffer){
	int i=0;
	while(buffer[i]!='\n'){
		i++;
	}

	buffer[i]='\0';
}


void* consola(void* no_use){
	puts("Bienvenido a la consola");

    char* buffer;

    char** token;

    buffer = (char *) calloc(32, sizeof(char));

    size_t tamanio = 32;

    getline(&buffer,&tamanio,stdin);
    agregar_espacio(buffer);
    token = string_split(buffer, " ");


	while(!string_equals_ignore_case(token[0],"salir")){

		if(string_equals_ignore_case(token[0],"pausar")){
			//pausar(token[1]);
		}

		if(string_equals_ignore_case(token[0],"continuar")){
			//continuar(token[1]);
		}
		if(string_equals_ignore_case(token[0],"bloquear")){
			//bloquear_por_consola(token[1],token[2]);
		}
		if(string_equals_ignore_case(token[0],"desbloquear")){
			//desbloquear_por_consola(token[1]);
		}
		if(string_equals_ignore_case(token[0],"listar")){

		}
		if(string_equals_ignore_case(token[0],"kill")){
			//kill(token[1]);
		}
		if(string_equals_ignore_case(token[0],"status")){
			//token[1]
		}
		if(string_equals_ignore_case(token[0],"deadlock")){
			puts("mostrando deadlock");
		}

		getline(&buffer,&tamanio,stdin);
		agregar_espacio(buffer);
		token = string_split(buffer, " ");

	}

	puts("salida con exito");
	free(buffer);

	return NULL;
}


int main(void) {

	puts("Hola, soy el planificador ;)");

	empezar_comunicacion_ESI();

	pthread_t consola_thread;

	if(pthread_create(&consola_thread, NULL, &consola,NULL)){
		puts("Error al crear el hilo t.t");
		return 0;
	}

	if(pthread_join(consola_thread, NULL)) {

		puts("Error joining");
		return 0;

	}

	return EXIT_SUCCESS;
}



void empezar_comunicacion_ESI(){

	int server = crear_server(puerto);

	puts("Server creado");

	int cli = aceptar_conexion(server);

	char* recib = recibir_string(cli);

	printf("Recibi: %s \n",recib);

	free(recib);

}


void empezar_comunicacion_coordinador(){

	int result = enviar(socket_coordinador,HANDSHAKE_PLANIFICADOR,0,NULL);

	t_paquete* paquete = recibir(socket_coordinador);

	if(paquete->codigo_operacion == HANDSHAKE_COORDINADOR)
		puts(" Conexion con el coordinador exitosa.");
	destruir_paquete(paquete);
}

void enviar_paquete_coordinador(){

	char * mensaje = "S1N V0C4L3S P4P4 XD";
	enviar(socket_coordinador, 500, strlen(mensaje), (void *)mensaje); // STRING_SENT = 500 ; en el protocolo del coord :3

}
