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
#include <commons/collections/list.h>
#include <commons/string.h>
#include "Planificador.h"


char* ip_coordinador;
char* puerto_coordinador;
char* puerto_escucha;
int socket_coordinador;
int algoritmo;
double estimacionInicial;
double alfa;
int id_base = 0;
proceso_esi_t * esi_ejecutando;
t_log* logger;
t_config* config;
t_list* ready_q;
t_list* blocked_q;
t_list* rip_q;


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
		finalizar();
		return 0;

	}
	finalizar();
	return EXIT_SUCCESS;
}

void levantoConfig(char* path){

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

	if(config_has_property(config, "PUERTO_ESCUCHA")){
			puerto_escucha= config_get_string_value(config, "PUERTO_ESCUCHA");
		}else{
			log_error(logger, "No se encuentra el puerto_escucha del Coordinador");
			finalizar();
			exit(EXIT_FAILURE);
		}

	if(config_has_property(config, "ALGORITMO_PLANIFICACION")){
		char* algoritmoString = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
		definirAlgoritmo(algoritmoString);
	}
	else{
		log_error(logger, "No se encuentra el algoritmo");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if(config_has_property(config, "ESTIMACION_INICIAL")){
			estimacionInicial= config_get_double_value(config, "ESTIMACION_INICIAL");
		}else{
			log_error(logger, "No se encuentra la estimacion inicial");
			finalizar();
			exit(EXIT_FAILURE);
		}


	log_info(logger, "Se cargó exitosamente la configuración");}

void inicializar(char* path){

	levantoConfig(path);

	ready_q = list_create();
	blocked_q = list_create();

	// Me conecto al Coordinador

	socket_coordinador = conectar_a_server(ip_coordinador,puerto_coordinador);

	enviar(socket_coordinador, HANDSHAKE_PLANIFICADOR, 0, NULL);

	t_paquete* respuesta = recibir(socket_coordinador);

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
	list_destroy_and_destroy_elements(ready_q);
	list_destroy_and_destroy_elements(rip_q);
	list_destroy_and_destroy_elements(blocked_q);
}

int procesar_mensaje(int socket) {
		t_paquete* paquete = recibir(socket);

		switch (paquete->codigo_operacion) {

			case HANDSHAKE_ESI: {
				enviar(socket, HANDSHAKE_PLANIFICADOR, 0, NULL);
				proceso_esi_t* nuevo_esi = nuevo_processo_esi(socket);
				planificar(nuevo_esi);
				break;
			}

			case STRING_SENT: {
				char* recibido = (char*) (paquete->data);
				printf("%s", recibido);
				break;
			}

			case ESI_BLOQUEADO: {
				//bloqueado por que recurso?
				list_add(blocked_q, esi_ejecutando);
				esi_ejecutando = (proceso_esi_t *)list_get(ready_q, 0);
				list_remove(ready_q,0);
				break;
			}

			case EXITO_OPERACION: {
				enviar(esi_ejecutando->socket,EJECUTAR_LINEA,0,NULL);
				//y si es el primer esi?
				break;
			}
			case ESI_FINALIZADO:{
				list_add(rip_q, esi_ejecutando);
				//El esi que finaliza es el que estaba ejecutando actualmente?
				break;
			}

			default:
 				destruir_paquete(paquete);
				return -1;
		}

		destruir_paquete(paquete);
		return 1;
}


void planificar (proceso_esi_t* nuevo_esi){

	switch(algoritmo){

		case FIFO:{
			list_add(ready_q,(void*) nuevo_esi);
			break;
		}

		case SJFCD:{
		break;
		}


		case SJFSD:{
		break;
		}

		case HRRN:{
		break;
		}
	}

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

void definirAlgoritmo(char* algoritmoString){
	if(string_equals_ignore_case(algoritmoString,"FIFO"))
		algoritmo = FIFO;
	else if(string_equals_ignore_case(algoritmoString,"SJF-CD"))
			algoritmo = SJFCD;
		else if(string_equals_ignore_case(algoritmoString,"SJF-SD"))
				algoritmo = SJFSD;
			else if(string_equals_ignore_case(algoritmoString,"HRRN"))
					algoritmo = HRRN;
			else {
				log_error(logger, "El algoritmo no esta contemplado");
				finalizar();
				exit(EXIT_FAILURE);
			}
}

void estimar_proxima_rafaga(proceso_esi_t* esi){
	esi->estimacion_ant = alfa*esi->duracion_raf_ant + (1-alfa)*esi->estimacion_ant;
}

bool bloqueadoPorRecurso(proceso_esi_t* esi,char* recurso){
	return string_equals_ignore_case(esi->recursoBloqueante,recurso);
}

proceso_esi_t* nuevo_processo_esi(int socket_esi){
	proceso_esi_t* nuevo_esi; //= malloc(sizeof(proceso_esi_t));
	id_base++;
	nuevo_esi->ID = id_base;
	nuevo_esi->estimacion_ant = estimacionInicial;
	nuevo_esi->duracion_raf_ant = 0;
	nuevo_esi->recursoBloqueante = NULL;
	nuevo_esi->socket = socket_esi;
	return nuevo_esi;
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
