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


t_log* logger;
t_config* config_aux;
int socket_coordinador;
int socket_planificador;

int main(int argc, char* argv[])  {
	char* mensaje_recibido;


	puts("!!!INICIANDO-Soy el proceso instancia!!!");

	// Creo log
	logger = log_create("instancia.log","INSTANCIA",false,LOG_LEVEL_TRACE);

	// Cargo la configuración
	t_config* config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
			config_destroy(config);
			return EXIT_FAILURE;
		}

	if(!config_has_property(config, CFG_IP)){
		log_error(logger, "No se encuentra la ip del Coordinador");
					finalizar();
					exit(EXIT_FAILURE);
		}

	if(!config_has_property(config, CFG_PORT)){
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Se cargó exitosamente la configuración");

	// Me conecto al Coordinador
	socket_coordinador = conectar_a_server(CFG_IP, CFG_PORT);
	log_info(logger, "Conexión exitosa al Coordinador");

	//ENVIAR MENSAJE
	enviar_string(socket_coordinador, STRING_SENT);

	mensaje_recibido = recibir_string(socket_coordinador);
	if(mensaje_recibido == HANDSHAKE_COORDINADOR){
		log_info(logger, "Mensaje recibido");
		}else{
			log_info(logger, "Error al recibir mensaje");
		}
	//finalizar
	log_info(logger, "Fin ejecución");
	config_destroy(config);
	log_destroy(logger);
	free(mensaje_recibido);

	/*int conexion_coordinador = conectar_a_server("127.0.0.1","9999");
	mensaje_recibido = recibir_string(conexion_coordinador);
	if(conexion_coordinador == HANDSHAKE_COORDINADOR){
		printf("recibi \"%s\" del coordinador",mensaje_recibido);
		enviar_string(conexion_coordinador, HANDSHAKE_ESI);
	}else{
		printf("recibi \"%s\" del coordinador otro mesaje por error",mensaje_recibido);
		enviar_string(conexion_coordinador, STRING_SENT);
	}*/


	return EXIT_SUCCESS;
}



