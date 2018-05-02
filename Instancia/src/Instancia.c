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
#include <commons/config.h>
#include <commons/log.h>


#define CFG_IP  "ip_coordinador"
#define CFG_PORT  "port_coordinador"
#define CFG_ALGO  "distribution_replacement"
#define CFG_POINT  "point_mount"
#define CFG_NAME_INST  "name_instancia"
#define CFG_INTERVAL  "interval"



int main(void) {
	char* mensaje_recibido;

	puts("!!!Soy el proceso instancia!!!");


	/*int conexion_coordinador = conectar_a_server("127.0.0.1","9999");
	mensaje_recibido = recibir_string(conexion_coordinador);
	if(conexion_coordinador == HANDSHAKE_COORDINADOR){
		printf("recibi \"%s\" del coordinador",mensaje_recibido);
		enviar_string(conexion_coordinador, HANDSHAKE_ESI);
	}else{
		printf("recibi \"%s\" del coordinador otro mesaje por error",mensaje_recibido);
		enviar_string(conexion_coordinador, STRING_SENT);
	}*/

	free(mensaje_recibido);
	return EXIT_SUCCESS;
}

void inicializar(char* path){

	// Creo log

	logger = log_create("esi.log","ESI",false,LOG_LEVEL_TRACE);

	// Cargo la configuración

	config_aux = config_create(path);

	if(config_has_property(config_aux, "IP_COORDINADOR")){
		config.ip_coordinador = config_get_string_value(config_aux, "IP_COORDINADOR");
	}else{
		log_error(logger, "No se encuentra la ip del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if(config_has_property(config_aux, "PUERTO_COORDINADOR")){
		config.puerto_coordinador = config_get_string_value(config_aux, "PUERTO_COORDINADOR");
	}else{
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if(config_has_property(config_aux, "IP_PLANIFICADOR")){
		config.ip_planificador = config_get_string_value(config_aux, "IP_PLANIFICADOR");
	}else{
		log_error(logger, "No se encuentra la ip del Planificador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if(config_has_property(config_aux, "PUERTO_PLANIFICADOR")){
		config.puerto_planificador = config_get_string_value(config_aux, "PUERTO_PLANIFICADOR");
	}else{
		log_error(logger, "No se encuentra el puerto del Planificador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Se cargó exitosamente la configuración");

	// Me conecto al Coordinador y Planificador

	socket_coordinador = conectar_a_server(config.ip_coordinador, config.puerto_coordinador);
	socket_planificador = conectar_a_server(config.ip_planificador, config.puerto_planificador);

	log_info(logger, "Conexión exitosa al Coordinador y Planificador");
}

void finalizar(){
	log_info(logger, "Fin ejecución");
	config_destroy(config_aux);
	log_destroy(logger);
}


