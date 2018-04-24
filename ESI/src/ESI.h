/*
 * ESI.h
 *
 *  Created on: 23 abr. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_

#include <stdio.h>
#include <stdlib.h>
#include <sockets.h>
#include <protocolo.h>
#include <commons/config.h>
#include <commons/log.h>

typedef struct {
	char* ip_coordinador;
	char* puerto_coordinador;
	char* ip_planificador;
	char* puerto_planificador;
} t_config_esi;

t_log* logger;
t_config* config_aux;
t_config_esi config;
int socket_coordinador;
int socket_planificador;

void inicializar(char* path);
void finalizar();

#endif /* ESI_H_ */
