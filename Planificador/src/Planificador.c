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
#include <commons/config.h>
#include <commons/log.h>

char* ip_coordinador;
char* puerto_coordinador;
char* puerto_escucha;
int socket_coordinador;
t_log* logger;
t_config* config;



void* consola(void* no_use){
	puts("Bienvenido a la consola");

    char* buffer;

    char** token;

    buffer = (char *) calloc(50, sizeof(char));

    size_t tamanio = 50;

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

	inicializar("Planificador.cfg");

	// Me conecto al Coordinador

	socket_coordinador = conectar_a_server(ip_coordinador,puerto_coordinador);

	log_info(logger, "Conexión exitosa al Coordinador");

	multiplexar(puerto_escucha,(void *) procesar_mensaje);
	
	
	//Creo el hilo de la consola
	pthread_t consola_thread;

	if(pthread_create(&consola_thread, NULL, &consola,NULL)){
		puts("Error al crear el hilo");
		return 0;
	}

	if(pthread_join(consola_thread, NULL)) {

		puts("Error joining");
		return 0;

	}
	
	return EXIT_SUCCESS;
}


void inicializar(char* path){

	//Creo log y cargo configuracion inicial

	logger = log_create("planificador.log","PLANIFICADOR",false,LOG_LEVEL_TRACE);

	config = config_create(path);

	if(config_has_property(config, "IP_COORDINADOR")){
		ip_coordinador = config_get_string_value(config, "IP_COORDINADOR");
	}else{
		log_error(logger, "No se encuentra la ip del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if(config_has_property(config, "PUERTO_COORDINADOR")){
		puerto_coordinador = config_get_string_value(config, "PUERTO_COORDINADOR");
	}else{
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if(config_has_property(config, "PUERTO")){
			puerto_escucha= config_get_string_value(config, "PUERTO");
		}else{
			log_error(logger, "No se encuentra el puerto_escucha del Coordinador");
			finalizar();
			exit(EXIT_FAILURE);
		}

	log_info(logger, "Se cargó exitosamente la configuración");

	// Me conecto al Coordinador

	socket_coordinador = conectar_a_server(ip_coordinador,puerto_coordinador);

	log_info(logger, "Conexión exitosa al Coordinador");

}

void finalizar(){
	log_info(logger, "Fin ejecución");
	config_destroy(config);
	log_destroy(logger);
}

int procesar_mensaje(int socket) {
		t_paquete* paquete = recibir(socket);

		switch (paquete->codigo_operacion) {

		case HANDSHAKE_ESI: {
			enviar(socket, HANDSHAKE_PLANIFICADOR, 0, NULL);
			break;
		}
		case STRING_SENT: {
			char* recibido = (char*) (paquete->data);
			printf("%s", recibido);
			break;
		}
		default:
			destruir_paquete(paquete);
			return -1;
		}
		destruir_paquete(paquete);
		return 1;
}


//Funciones auxiliares


void agregar_espacio(char* buffer){
	int i=0;
	while(buffer[i]!='\n'){
		i++;
	}

	buffer[i]='\0';
}


/*

void empezar_comunicacion_ESI(){

	int server = crear_server(puerto);

	puts("Server creado");

	int cli = aceptar_conexion(server);

	char* recib = recibir_string(cli);

	printf("Recibi: %s \n",recib);

	free(recib);

}


void empezar_comunicacion_coordinador(){
	socket_coordinador = conectar_a_server((char *)ip_coordinador,(char *) puerto_coordinador);

	int result = enviar(socket_coordinador,HANDSHAKE_PLANIFICADOR,0,NULL);

	t_paquete* paquete = recibir(socket_coordinador);

	if(paquete->codigo_operacion == HANDSHAKE_COORDINADOR)
		puts(" Conexion con el coordinador exitosa.");
	destruir_paquete(paquete);
}

void enviar_paquete_coordinador(char* mensaje){

	enviar(socket_coordinador, 500, strlen(mensaje), (void *)mensaje); // STRING_S	ENT = 500 ; en el protocolo del coord :3

	t_paquete* paquete = recibir(socket_coordinador);

	if(paquete->tamanio !=strlen((char*)paquete->data) && paquete->codigo_operacion!=500){
		puts("Error al recibir");
	}

	printf("Recibi: %s", (char *)paquete->data);
	destruir_paquete(paquete);
}
*/
