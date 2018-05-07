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
#include <pelao/sockets.h>
#include <pthread.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>
#include "Planificador.h"

char* ip_coordinador;
char* puerto_coordinador;
char* puerto_escucha;
int socket_coordinador;
int algoritmo;
proceso_esi_t esi_ejecutando;
t_log* logger;
t_config* config;


int main(void) {

	puts("Hola, soy el planificador ;)");

	inicializar("Planificador.cfg");

	//Creo el hilo de la consola
	pthread_t consola_thread;

	if(pthread_create(&consola_thread, NULL, &consola,NULL)){
		puts("Error al crear el hilo");
		return 0;
	}


	multiplexar(puerto_escucha,(void *) procesar_mensaje);


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

	enviar(socket_coordinador, HANDSHAKE_PLANIFICADOR, 0, NULL);

	t_paquete respuesta = recibir(socket_coordinador);

	if(respuesta->codigo_operacion!=HANDSHAKE_COORDINADOR){
		log_error(logger, "Fallo comunicaacion con el coordinador");
		destruir_paquete(respuesta);
		finalizar();
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Conexión exitosa al Coordinador");
	destruir_paquete(respuesta);

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
				proceso_esi_t nuevo_esi = nuevo_processo_esi(paquete->data);
				enviar_ready_q(nuevo_esi);
				planificar();
				break;
			}

			case STRING_SENT: {
				char* recibido = (char*) (paquete->data);
				printf("%s", recibido);
				break;
			}

			case ESI_BLOQUEADO: {
				send_waiting_q(esi_ejecutando);
				break;
			}

			case EXITO_OPERACION: {
				//habria que sacar el primero de ready_q y decirle que ejecute
				break;
			}

			default:
				destruir_paquete(paquete);
				return -1;
		}

		destruir_paquete(paquete);
		return 1;
}


void planificar (){

	switch(algoritmo){

		case FIFO:{

		}

		case SJF:{

		}

		case HRRN:{

		}
	}
/*
	enviar(socket,EJECUTAR_LINEA,0,NULL); //ver a cual ESI le estoy enviando
*/

}


//consola planificador
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
