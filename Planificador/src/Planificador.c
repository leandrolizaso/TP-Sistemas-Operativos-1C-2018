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
#include <semaphore.h>

/*config*/

char* ip_coordinador;
char* puerto_coordinador;
char* puerto_escucha;
int socket_coordinador;

int algoritmo;
double estimacionInicial;
double alfa;

t_log* logger;
t_log* rip_q;
t_log* estimaciones;
t_config* config;

/*otras variables*/
int id_base = 1;
int system_clock=0;  
int cant_claves;
_Bool block_config = true;
_Bool ejecutando = false;
_Bool pausado;
char* recurso_bloqueante;
_Bool flag_esi_muerto = false;
char* clave_status;
proceso_esi_t * esi_ejecutando;

/*Listas*/

t_list* ready_q;
t_list* blocked_q;
t_list* blocked_key;

/*semaforos*/
sem_t* m_esi;
sem_t* m_rip;
sem_t* m_ready;
sem_t* m_blocked;
sem_t* m_key;
sem_t* bin_status;


int main(int argc, char*argv[]) {

	stop_multiplexar=false;

	puts("Hola, soy el planificador ;)");

	inicializar(argv[1]);

	//Creo el hilo de la consola
	pthread_t consola_thread;

	if (pthread_create(&consola_thread, NULL, &consola, NULL)) {
		puts("Error al crear el hilo");
		return 0;
	}

	multiplexar(puerto_escucha, (void *) procesar_mensaje);

	if (pthread_join(consola_thread, NULL)) {

		puts("Error joining");
		finalizar();
		return 0;

	}
	finalizar();
	return EXIT_SUCCESS;
}


void levantoConfig(char* path) {

	//Creo log y cargo configuracion inicial

	logger = log_create("planificador.log", "PLANIFICADOR", false,LOG_LEVEL_TRACE);
	rip_q = log_create("RIP_QUEUE.log", "PLANIFICADOR", false,LOG_LEVEL_TRACE);
	estimaciones = log_create("Estimaciones.log", "PLANIFICADOR", false,LOG_LEVEL_TRACE);

	config = config_create(path);

	if (config_has_property(config, "IP_COORDINADOR")) {
		ip_coordinador = config_get_string_value(config, "IP_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config, "PUERTO_COORDINADOR")) {
		puerto_coordinador = config_get_string_value(config,
				"PUERTO_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config, "PUERTO_ESCUCHA")) {
		puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	} else {
		log_error(logger, "No se encuentra el puerto_escucha");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config, "ALGORITMO_PLANIFICACION")) {
		char* algoritmoString = config_get_string_value(config,
				"ALGORITMO_PLANIFICACION");
		definirAlgoritmo(algoritmoString);
	} else {
		log_error(logger, "No se encuentra el algoritmo");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config, "ESTIMACION_INICIAL")) {
		estimacionInicial = config_get_double_value(config,
				"ESTIMACION_INICIAL");
	} else {
		log_error(logger, "No se encuentra la estimacion inicial");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config, "ALFA")) {
			alfa = config_get_double_value(config,
					"ALFA");
		} else {
			log_error(logger, "No se encuentra el alfa");
			finalizar();
			exit(EXIT_FAILURE);
	}

	if (config_has_property(config, "CANTIDAD_CLAVES")) {
				cant_claves = config_get_int_value(config,"CANTIDAD_CLAVES");
				log_debug(logger,"cantidad de claves leidas %d",cant_claves);
				if(cant_claves>0){
				char** valor_leido = config_get_array_value(config,"CLAVE_TOMADA");
				for(int i=0;i < cant_claves; i++){
					if (config_has_property(config, "CLAVE_TOMADA")) {
						char clave[40];
						strcpy(clave,valor_leido[i]);
						bloquear_key(clave);
						puts(clave);
					} else {
						log_error(logger, "No se encuentra la clave");
						finalizar();
						exit(EXIT_FAILURE);
					}
				}
				}
			} else {
				log_error(logger, "No se encuentra la cantidad de claves");
				finalizar();
				exit(EXIT_FAILURE);
		}
	block_config=false;

	log_info(logger, "Se cargó exitosamente la configuración");
}

void inicializar(char* path) {

	blocked_key = list_create();
	ready_q = list_create();
	blocked_q = list_create();
	levantoConfig(path);

	recurso_bloqueante = malloc(sizeof(char)*40);
	clave_status = malloc(sizeof(char)*40);

	init_semaphores();

	// Me conecto al Coordinador
	socket_coordinador = conectar_a_server(ip_coordinador, puerto_coordinador);
	enviar(socket_coordinador, HANDSHAKE_PLANIFICADOR,strlen_null(puerto_escucha), puerto_escucha);

	t_paquete* respuesta = recibir(socket_coordinador);

	if (respuesta->codigo_operacion != HANDSHAKE_COORDINADOR) {
		log_error(logger, "Fallo comunicaacion con el coordinador");
		destruir_paquete(respuesta);
		finalizar();
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Conexión exitosa al Coordinador");
	destruir_paquete(respuesta);

}

void init_semaphores(){
	m_ready = malloc(sizeof(sem_t));
	m_key = malloc(sizeof(sem_t));
	m_esi= malloc(sizeof(sem_t));
	m_blocked = malloc(sizeof(sem_t));
	m_rip = malloc(sizeof(sem_t));
	bin_status = malloc(sizeof(sem_t));
	
	if(sem_init(m_ready,0,1)){
		log_info(logger, "Error al inicializar mutex ready");
	}
	if(sem_init(m_esi,0,1)){
		log_info(logger, "Error al inicializar mutex esi");
	}
	if(sem_init(m_rip,0,1)){
		log_info(logger, "Error al inicializar mutex rip");
	}
	if(sem_init(m_blocked,0,1)){
		log_info(logger, "Error al inicializar mutex blocked");
	}
	if(sem_init(m_key,0,1)){
		log_info(logger, "Error al inicializar mutex key");
	}
	if(sem_init(bin_status,0,1)){
		log_info(logger, "Error al inicializar bin de status");
	}
}

void finalizar() {
	log_info(logger, "Fin ejecución");
	config_destroy(config);
	log_destroy(logger);
	log_destroy(rip_q);
	log_destroy(estimaciones);

	free(recurso_bloqueante);

	list_destroy_and_destroy_elements(ready_q, &destructor_esi);
	list_destroy_and_destroy_elements(blocked_q, &destructor_esi);
	list_destroy_and_destroy_elements(blocked_key, &destructor_key);

	free(m_ready);
	free(m_key);
	free(m_esi);
	free(m_blocked);
	free(m_rip);
	free(bin_status);
	free(clave_status);
}

void definirAlgoritmo(char* algoritmoString) {
	if (string_equals_ignore_case(algoritmoString, "FIFO"))
		algoritmo = FIFO;
		else if (string_equals_ignore_case(algoritmoString, "SJFCD"))
			algoritmo = SJFCD;
			else if (string_equals_ignore_case(algoritmoString, "SJFSD"))
				algoritmo = SJFSD;
				else if (string_equals_ignore_case(algoritmoString, "HRRN"))
					algoritmo = HRRN;
	else {
		log_error(logger, "El algoritmo no esta contemplado");
		finalizar();
		exit(EXIT_FAILURE);
	}
}

int procesar_mensaje(int socket) {

	//ver logica repetida


	t_paquete* paquete = recibir(socket);
	switch (paquete->codigo_operacion) {

	case HANDSHAKE_ESI: {
		usleep(300000);
		proceso_esi_t* nuevo_esi = nuevo_processo_esi(socket);
		enviar(socket, HANDSHAKE_PLANIFICADOR, sizeof(int),&(nuevo_esi->ID));
		sem_wait(m_ready);
		sem_wait(m_esi);
		list_add(ready_q, (void*) nuevo_esi);
		planificar();
		sem_post(m_esi);
		sem_post(m_ready);
		break;
	}



	case GET_CLAVE: {
		char* recurso = string_duplicate(paquete->data);

		sem_wait(m_esi);
		sem_wait(m_key);
		if (esta_clave(recurso)) {
			sem_wait(m_blocked);
			bloquear(esi_ejecutando, recurso);
			sem_post(m_blocked);
			enviar(socket, OPERACION_ESI_VALIDA,0,NULL);
		} else {
			bloquear_key(recurso);
			enviar(socket, OPERACION_ESI_VALIDA, 0, NULL);
		}
		sem_post(m_esi);
		sem_post(m_key);

		free(recurso);
		break;
	}

	case STORE_CLAVE: {
		char* recurso = string_duplicate(paquete->data);

		_Bool key_equals(void* clave) {
					return string_equals_ignore_case(((t_clave*) clave)->valor, recurso);
		}

		sem_wait(m_esi);

		if(!hizo_get(esi_ejecutando,recurso)){
			char* mensaje = "El esi no hizo GET";
			error_de_esi(mensaje, socket);
			free(recurso);
			sem_post(m_esi);
			break;
		}

		sem_post(m_esi);


		sem_wait(m_ready);
		sem_wait(m_blocked);
		if (esi_esperando(recurso)) {

			desbloquear(recurso);
		}
		sem_post(m_blocked);
		sem_post(m_ready);

		sem_wait(m_key);
		list_remove_and_destroy_by_condition(blocked_key, &key_equals,&destructor_key);
		sem_post(m_key);

		enviar(socket, OPERACION_ESI_VALIDA, 0, NULL);

		free(recurso);
		break;
	}

	case SET_CLAVE:{
		char* recurso = string_duplicate(paquete->data);

		sem_wait(m_esi);
		if(!hizo_get(esi_ejecutando,recurso)){
			char* mensaje = "El esi no hizo GET";
			error_de_esi(mensaje, socket);

		} else {
			enviar(socket, OPERACION_ESI_VALIDA, 0, NULL);
		}
		sem_post(m_esi);

		free(recurso);
		break;
	}

	case EXITO_OPERACION: {
		if(pausado){
			if(flag_esi_muerto){
				sem_wait(m_esi);
				matar_esi(esi_ejecutando);
				esi_ejecutando = NULL;
				flag_esi_muerto = false;
				sem_post(m_esi);
				break;
			}
		//break;
		} else{
			if(flag_esi_muerto){
				sem_wait(m_ready);
				sem_wait(m_esi);
				matar_esi(esi_ejecutando);
				esi_ejecutando = NULL;
				flag_esi_muerto = false;
				sem_post(m_esi);
				planificar();
				sem_post(m_ready);
				break;
			}
		}

		sem_wait(m_esi);


		if(is_in_list(esi_ejecutando->ID,blocked_q)){
			enviar(esi_ejecutando->socket,VOLVE,0,NULL);
			esi_ejecutando =NULL;
			sem_wait(m_ready);
			planificar();
			sem_post(m_ready);
			sem_post(m_esi);
			break;
		}

		if(!pausado){
			sem_wait(m_ready);
			if(!esi_ejecutando->a_blocked){
				if(algoritmo!=SJFCD){
					enviar(esi_ejecutando->socket, EJECUTAR_LINEA, 0, NULL);
					aumentar_rafaga(esi_ejecutando);
				} else {
					
					planificar();
				}
			}else{
				bloquear(esi_ejecutando, recurso_bloqueante);
				esi_ejecutando = NULL;
				
				planificar();
			}
			sem_post(m_ready);
		}
		sem_post(m_esi);
		break;
	}

	case ESI_FINALIZADO: {
		sem_wait(m_esi);
		sem_wait(m_rip);
		char* esi_finaliza_msg=malloc(sizeof(char)*20);
		strcpy(esi_finaliza_msg,"Finalizo ESI ");
		string_append(&esi_finaliza_msg,string_itoa(esi_ejecutando->ID));
		log_debug(rip_q,esi_finaliza_msg);
		free(esi_finaliza_msg);
		destructor_esi((void*)esi_ejecutando);
		sem_post(m_rip);
		esi_ejecutando = NULL;
		sem_wait(m_ready);
		planificar();
		sem_post(m_ready);
		sem_post(m_esi);
		break;
	}

	case ESI_ABORTADO:{
		sem_wait(m_esi);
		sem_wait(m_rip);
		char* esi_finaliza_msg=malloc(sizeof(char)*20);
		strcpy(esi_finaliza_msg,"Finalizo abortado ESI ");
		string_append(&esi_finaliza_msg,string_itoa(esi_ejecutando->ID));
		log_debug(rip_q,esi_finaliza_msg);
		free(esi_finaliza_msg);
		destructor_esi((void*)esi_ejecutando);
		sem_post(m_rip);
		esi_ejecutando = NULL;
		sem_wait(m_ready);
		planificar();
		sem_post(m_ready);
		sem_post(m_esi);
		break;
	}

	case RESPUESTA_STATUS: {
		t_status_clave* status = deserializar_status_clave(paquete->data);
		if(string_equals_ignore_case("\0",status->instancia)){
			printf("La clave no se encuentra en ninguna instancia, en este momento entraria en %s",status->instancia_now);
			puts("No tiene valor");

		}else{
			printf("La clave esta en %s",status->instancia);
			puts(status->valor);
		}
		listar(clave_status);
		sem_post(bin_status);
		break;
	}
	
	default:
		log_debug(logger,string_from_format("Mensaje erroneo. Recibi el codigo de operacion %d",paquete->codigo_operacion));
		//find_esi_dead(socket);
		destruir_paquete(paquete);
		return -1;
	}

	destruir_paquete(paquete);
	return 1;
}


void planificar() {
	if (pausado || (list_is_empty(ready_q)&&esi_ejecutando==NULL)){
		return;
	}



	if (algoritmo!=SJFCD) {
		if(esi_ejecutando == NULL){
			switch (algoritmo) {

			case FIFO: {
				esi_ejecutando = list_get(ready_q, 0);
				list_remove(ready_q, 0);
				enviar(esi_ejecutando->socket, EJECUTAR_LINEA, 0, NULL);
				logguear_estimaciones();
				break;
			}

			case SJFSD: {
				list_sort(ready_q,&menor_tiempo);
				esi_ejecutando = list_get(ready_q, 0);
				list_remove(ready_q, 0);
				enviar(esi_ejecutando->socket, EJECUTAR_LINEA, 0, NULL);
				aumentar_rafaga(esi_ejecutando);
				logguear_estimaciones();
				break;
			}

			case HRRN: {
				list_sort(ready_q,&mayor_ratio);
				esi_ejecutando = list_get(ready_q, 0);
				list_remove(ready_q, 0);
				enviar(esi_ejecutando->socket, EJECUTAR_LINEA, 0, NULL);
				aumentar_rafaga(esi_ejecutando);
				logguear_estimaciones();
				break;
			}
			}
			}
			}	else if(algoritmo==SJFCD) {
				if(esi_ejecutando!=NULL){list_add(ready_q,esi_ejecutando);}
					list_sort(ready_q,&menor_tiempo);
					esi_ejecutando = list_get(ready_q, 0);
					list_remove(ready_q, 0);
					enviar(esi_ejecutando->socket, EJECUTAR_LINEA, 0, NULL);
					aumentar_rafaga(esi_ejecutando);
					logguear_estimaciones();
				} else {
					perror("No se conoce el algoritmo de planificacion");
				}
}

//consola planificador
void* consola(void* no_use) {
	puts("Bienvenido a la consola. Los comandos son:");
	puts(" pausar");
	puts(" continuar");
	puts(" bloquear <clave> <ID esi>");
	puts(" desbloquear <clave>");
	puts(" listar <clave>");
	puts(" deadlock");
	puts(" kill <ID>");
	
	
	char* buffer;

	char** token;

	buffer = (char *) calloc(100, sizeof(char));

	size_t tamanio = 100;

	getline(&buffer, &tamanio, stdin);
	agregar_espacio(buffer);
	token = string_split(buffer, " ");

	while (!string_equals_ignore_case(token[0], "salir")) {

		if (string_equals_ignore_case(token[0], "pausar")) {
			pausado = true;
		}

		if (string_equals_ignore_case(token[0], "continuar")) {
			sem_wait(m_ready);
			sem_wait(m_esi);
			pausado = false;
			if(algoritmo!=SJFCD&&esi_ejecutando!=NULL){
				enviar(esi_ejecutando->socket, EJECUTAR_LINEA, 0, NULL);
			} else{
				planificar(); //como se que volvio el ultimo esi? si habia un esi en ejecucion
			}
			sem_post(m_esi);
			sem_post(m_ready);
		}

		if (string_equals_ignore_case(token[0], "bloquear")) {

			_Bool id_equals(void* pointer) {
				if (pointer != NULL) {
					char* id = string_itoa(((proceso_esi_t*) pointer)->ID);
					return string_equals_ignore_case(id, token[2]);
				} else return false;
			}

			if(token[1]!=NULL && token[2]!=NULL){

				sem_wait(m_esi);
				sem_wait(m_ready);
				if (id_equals(esi_ejecutando)) {
					esi_ejecutando->a_blocked = true;
					strcpy(recurso_bloqueante,token[1]);
				}else {
					proceso_esi_t* esi_a_bloquear = list_find(ready_q, &id_equals);
					if(esi_a_bloquear==NULL){
						puts("El esi no esta ejecutando ni en ready");
					}else{
						list_remove_by_condition(ready_q, &id_equals);
						sem_wait(m_blocked);
						bloquear(esi_a_bloquear, token[1]);
						sem_post(m_blocked);
					}
				}
				sem_post(m_ready);
				sem_post(m_esi);
			}else{
				puts("Se necesita ingresar una clave y un ESI para utilizar este comando");
			}
		}

		if (string_equals_ignore_case(token[0], "desbloquear")) {

			_Bool key_equals(void* pointer) {
				proceso_esi_t* esi = (proceso_esi_t*) pointer;
				return esi->ID!=0 &&
						string_equals_ignore_case(esi->recurso_bloqueante,token[1]);

			}


			_Bool find_key(void* pointer){
				if(pointer!=NULL){
					t_clave* clave = (t_clave*) pointer;
					return string_equals_ignore_case(clave->valor,token[1]);
				}else{
					return false;
				}
			}

			if(token[1]!=NULL){
				sem_wait(m_blocked);
				sem_wait(m_ready);
				if(list_find(blocked_q, &key_equals)!=NULL){
					proceso_esi_t* esi = list_remove_by_condition(blocked_q, &key_equals);
					esi->a_blocked=false;
					list_add(ready_q, esi);

				}
				sem_post(m_blocked);

				sem_wait(m_key);
				if(list_find(blocked_q, &key_equals)==NULL){
					list_remove_and_destroy_by_condition(blocked_key,&find_key,&destructor_key);
				}
				sem_post(m_key);

				sem_wait(m_esi);
				planificar();
				sem_post(m_esi);
				sem_post(m_ready);
			}else{
				puts("Se necesita ingresar una clave para utilizar este comando");
			}



		}

		if (string_equals_ignore_case(token[0], "listar")){

			if(token[1]!=NULL){
				listar(token[1]);
			}else{
				puts("Se necesita ingresar una clave para utilizar este comando");
			}



		}

		if (string_equals_ignore_case(token[0], "kill")) {
			if(token[1]!=NULL){
				kill(atol(token[1]));
			} else{
				puts("Se necesita ingresar un ESI para utilizar este comando");
			}
		}

		if (string_equals_ignore_case(token[0], "deadlock")) {
			puts("Los siguientes ESIs estan en deadlock :(");
			puts("Solucionar manualmente con kill <ID esi>");
			sem_wait(m_blocked);
			sem_wait(m_key);
			t_list* esis_a_imprimir = list_filter(blocked_q,&tiene_asginado);
			sem_post(m_key);
			sem_post(m_blocked);
			imprimir(esis_a_imprimir);
			list_destroy(esis_a_imprimir);
		}

		if (string_equals_ignore_case(token[0], "status")) {
			if(token[1]!=NULL){
				sem_wait(bin_status);
				strcpy(clave_status,token[1]);
				enviar(socket_coordinador,STATUS,0,NULL);
				//901 hasta modificar lib y tener STATUS
				//No hago el post para hacer un "Productor consumidor"
			} else{
				puts("Se necesita ingresar una clave para utilizar este comando");
			}
		}

		getline(&buffer, &tamanio, stdin);
		agregar_espacio(buffer);
		token = string_split(buffer, " ");

	}

	puts("salida con exito");
	stop_multiplexar=true;
	free(buffer);

	return NULL;
}

proceso_esi_t* nuevo_processo_esi(int socket_esi) {
	proceso_esi_t* nuevo_esi = malloc(sizeof(proceso_esi_t));
	nuevo_esi->ID = id_base;
	id_base++;
	nuevo_esi->estimacion_ant = estimacionInicial;
	nuevo_esi->duracion_raf_ant = 0;
	nuevo_esi->ejecuto_ant = 0;
	nuevo_esi->recurso_bloqueante = malloc(sizeof(char)*40);
	strcpy(nuevo_esi->recurso_bloqueante,"");
	nuevo_esi->socket = socket_esi;
	nuevo_esi->viene_de_blocked = false;
	nuevo_esi->a_blocked = false;
	nuevo_esi->waiting_time=system_clock;
	return nuevo_esi;
}

void imprimir(t_list* esis_a_imprimir) {
	if (!list_is_empty(esis_a_imprimir)) {
		for (int i = 0; i < list_size(esis_a_imprimir); i++) {
			printf("\t ESI%i\n", ((proceso_esi_t*) list_get(esis_a_imprimir, i))->ID);
		}
	} else {
		puts("No hay esis bloqueados por ese recurso");
	}
}

void bloquear(proceso_esi_t* esi, char* recurso) {
	esi->viene_de_blocked = true;
	esi->duracion_raf_ant=esi->ejecuto_ant;
	esi->ejecuto_ant=0;
	estimar_proxima_rafaga(esi);
	strcpy(esi->recurso_bloqueante,recurso);
	esi->waiting_time = system_clock;
	list_add(blocked_q, esi);
}

void desbloquear(char* recurso) {
	_Bool recurso_eq(void* unEsi) {
		return ((proceso_esi_t*) unEsi)->ID!=0 &&
				string_equals_ignore_case(((proceso_esi_t*) unEsi)->recurso_bloqueante, recurso);
	}
	proceso_esi_t* esi = list_find(blocked_q, &recurso_eq);
	if (esi != NULL) {
		esi->a_blocked = false;
		strcpy(esi->recurso_bloqueante, "");
		list_add(ready_q, esi);
		list_remove_by_condition(blocked_q, &recurso_eq);
	}
	planificar(); //es necesario?
}

void bloquear_key(char* clave) {
	t_clave* nueva_clave = malloc(sizeof(t_clave));
	if(block_config){
		nueva_clave->ID_esi = 0;
	}else{
		nueva_clave->ID_esi = esi_ejecutando->ID;
	}
	nueva_clave->valor = malloc(sizeof(char)*40);
	strcpy(nueva_clave->valor,clave);
	list_add(blocked_key, nueva_clave);
}

_Bool esi_esperando(char* recurso) {
	_Bool esta(void* esi) {
		return string_equals_ignore_case(
				((proceso_esi_t*) esi)->recurso_bloqueante, recurso);
	}

	return list_any_satisfy(blocked_q, &esta);

}

_Bool hizo_get(proceso_esi_t* esi, char* recurso){
	_Bool mismo_id(void* pointer){
		t_clave* clave = (t_clave*) pointer;
		return string_equals_ignore_case(recurso, clave->valor) && (esi->ID == clave->ID_esi) && esi->ID!=0;
	}
	return list_any_satisfy(blocked_key,&mismo_id);
}

_Bool menor_tiempo(void* pointer1, void* pointer2){

	int sort_number(void* pointer){
		return ((proceso_esi_t*)pointer)->estimacion_ant - ((proceso_esi_t*)pointer)->ejecuto_ant;

	}

	return sort_number(pointer1) <= sort_number(pointer2);
}

_Bool mayor_ratio(void* ptr1, void* ptr2){

	float ratio_calc(proceso_esi_t* esi){
		return 1 + (system_clock - esi->waiting_time)/(esi->estimacion_ant);
	}

	return ratio_calc((proceso_esi_t*)ptr1) >= ratio_calc((proceso_esi_t*)ptr2);
}

_Bool is_in_list(int id,t_list* lista){

	_Bool mismo_id(void* un_esi){
		return id == ((proceso_esi_t*)un_esi)->ID;
	}

	return list_find(lista,&mismo_id);
}

_Bool is_in_list_socket(int socket,t_list* lista, int* id){

	_Bool mismo_id(void* un_esi){
		if(socket == ((proceso_esi_t*)un_esi)->socket){
			*id = ((proceso_esi_t*)un_esi)->ID;
			return true;
		}
		return false;
	}

	return list_find(lista,&mismo_id);
}

// Para la consola

void kill(int id){
	_Bool mismo_id(void* un_esi){
		return id == ((proceso_esi_t*)un_esi)->ID;
	}

	void finalizar_esi(t_list* lista){
		proceso_esi_t* esi =list_find(lista,&mismo_id);
		enviar(esi->socket,FINALIZAR,0,NULL);
		char* esi_finaliza_msg = string_from_format("Finalizo ESI %s",string_itoa(esi->ID));
		liberar(esi); //mmm
		list_remove_and_destroy_by_condition(lista,&mismo_id,&destructor_esi);
		sem_wait(m_rip);
		log_debug(rip_q,esi_finaliza_msg);
		sem_post(m_rip);
		free(esi_finaliza_msg);  //Hay que liberarlo?
	}

	sem_wait(m_esi);
	if(esi_ejecutando != NULL){
	if(id==esi_ejecutando->ID){
		flag_esi_muerto=true;
		sem_post(m_esi);
		return;
	}}
	sem_post(m_esi);

	sem_wait(m_blocked);
	sem_wait(m_ready);
	if (is_in_list(id,blocked_q)){
		finalizar_esi(blocked_q);

	}else if(is_in_list(id,ready_q)){
		finalizar_esi(ready_q);
	}else{
		puts("El ESI no se encuentra en el sistema");
	}
	sem_post(m_ready);
	sem_post(m_blocked);
}

void listar(char* recurso){
	
	_Bool bloqueadoPorRecurso(void* parametro) {
		proceso_esi_t* esi = parametro;
		return string_equals_ignore_case(esi->recurso_bloqueante,recurso);
	}

	sem_wait(m_blocked);
	t_list* esis_a_imprimir = list_filter(blocked_q,&bloqueadoPorRecurso);
	sem_post(m_blocked);
	printf("Los esis esperando el recurso %s son:\n",recurso);
	imprimir(esis_a_imprimir);
	list_destroy(esis_a_imprimir);
}

//Funciones auxiliares

void estimar_proxima_rafaga(void* pointer) {
	proceso_esi_t* esi = (proceso_esi_t*) pointer;
	esi->estimacion_ant = (alfa/100) * esi->duracion_raf_ant
			+ (1 - (alfa/100)) * esi->estimacion_ant;
}

void aumentar_rafaga(proceso_esi_t* esi){
	if(esi->viene_de_blocked){
		esi->viene_de_blocked=false;
		esi->ejecuto_ant=1;
	}else{
		esi->ejecuto_ant++;
	}
	//logguear_estimaciones();
	system_clock++;
}


void agregar_espacio(char* buffer) {
	int i = 0;
	while (buffer[i] != '\n') {
		i++;
	}

	buffer[i] = '\0';
}

void destructor_esi(void *elem) {
	proceso_esi_t* esi = (proceso_esi_t*) elem;
	liberar_recursos(elem);
	free(esi->recurso_bloqueante);
	free(esi);
	//Liberar recursos de blocked_key
}

void destructor_key(void *elem) {
	t_clave* key = (t_clave*) elem;
	free(key->valor);
	free(key);
}

bool esta_clave(char* clave) {

	_Bool bloqueadoPorClave(void* clave_bloqueada) {
		return string_equals_ignore_case(((t_clave*) clave_bloqueada)->valor, clave);
	}

	return list_any_satisfy(blocked_key, &bloqueadoPorClave);

}

void error_de_esi(char* mensaje, int socket){
	enviar(socket, OPERACION_ESI_INVALIDA,sizeof(char)*strlen(mensaje)+1,mensaje);
	sem_wait(m_rip);
	char* esi_finaliza_msg=malloc(sizeof(char)*30);
	strcpy(esi_finaliza_msg,"Finalizo ESI con error ");
	string_append(&esi_finaliza_msg,string_itoa(esi_ejecutando->ID));
	log_debug(rip_q,esi_finaliza_msg);
	free(esi_finaliza_msg);
	sem_post(m_rip);
	destructor_esi((void*)esi_ejecutando);
	esi_ejecutando = NULL;
	sem_wait(m_ready);
	planificar();
	sem_post(m_ready);
}


_Bool tiene_asginado(void* pointer){
	proceso_esi_t* esi = (proceso_esi_t*) pointer;

	_Bool id_equals(void* ptr) {
		if (ptr != NULL) {
		int id = ((t_clave*) ptr)->ID_esi;
		return id == esi->ID;
		} else return false;
			}
	return list_any_satisfy(blocked_key,&id_equals);
}

void matar_esi(){
	enviar(esi_ejecutando->socket,FINALIZAR,0,NULL);
	char* esi_finaliza_msg = string_from_format("Finalizo ESI %s",string_itoa(esi_ejecutando->ID));
	destructor_esi((void*)esi_ejecutando);
	sem_wait(m_rip);
	log_debug(rip_q,esi_finaliza_msg);
	sem_post(m_rip);
	free(esi_finaliza_msg);
}

void liberar_recursos(void* pointer){
	_Bool id_equals(void* ptr) {
		if (ptr != NULL) {
			int id = ((t_clave*) ptr)->ID_esi;
			if(id == ((proceso_esi_t*)pointer)->ID){
				desbloquear(((t_clave*) ptr)->valor);
				return true;
			} else return false;
			
		} else return false;
	}
	while(tiene_asginado(pointer)){
		list_remove_and_destroy_by_condition(blocked_key,&id_equals,&destructor_key);
	}
}

void logguear_estimaciones(){
	log_debug(logger,string_from_format("El ESi %d ejecuta linea",esi_ejecutando->ID));
	log_debug(estimaciones,string_from_format("ESI%d Estimacion: %f ratio: %f",esi_ejecutando->ID, esi_ejecutando->estimacion_ant,esi_ejecutando->waiting_time));
	for(int i=0;i<list_size(ready_q);i++){
		proceso_esi_t* esi = list_get(ready_q,i);
		log_debug(estimaciones,string_from_format("ESI%d Estimacion: %f ratio: %f",esi->ID, esi->estimacion_ant,esi->waiting_time));
	}
		log_debug(estimaciones,"");

}

void find_esi_dead(int socket){
	if(esi_ejecutando->socket==socket){
		matar_esi();
		return;
	}
	
	int id_esi=0;
	if(is_in_list_socket(socket,ready_q,&id_esi)||is_in_list_socket(socket,blocked_q,&id_esi)){
		kill(id_esi);
	}

	return;
}
