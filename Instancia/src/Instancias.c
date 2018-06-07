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
#include <unistd.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>
#include "Instancia.h"

#define CFG_IP  "ip_coordinador"
#define CFG_PORT  "port_coordinador"
#define CFG_ALGO  "distribution_replacement"
#define CFG_POINT  "point_mount"
#define CFG_NAME_INST  "name_instancia"
#define CFG_INTERVAL  "interval"


int main(int argc, char* argv[])  {

	t_log* logger;

	puts("!!!INICIANDO-Soy el proceso instancia!!!");

	// Creo log
	logger = log_create("instancia.log","INSTANCIA",false,LOG_LEVEL_TRACE);

	// Cargo la configuración
	t_config* config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
		config_destroy(config);
		return EXIT_FAILURE;
	}
	//inicializar
	inicializar(config,logger);

	//finalizar
	finalizar(config,logger);



	return EXIT_SUCCESS;
}



