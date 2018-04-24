/*
 ============================================================================
 Name        : ESI.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "ESI.h"

int main(int argc, char* argv[]) {

	puts("!!!Soy el proceso ESI!!!");

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
