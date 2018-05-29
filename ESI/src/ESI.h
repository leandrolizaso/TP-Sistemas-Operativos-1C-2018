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
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <parsi/parser.h>

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
int ID;

void inicializar(char* path);
void ejecutar(char* script);
void finalizar();
int enviar_get(t_esi_operacion sentencia);
int enviar_set(t_esi_operacion sentencia);
int enviar_store(t_esi_operacion sentencia);


// Encapsulamiento

void crearLog();
void levantarConfig(char* path);
void conectarPlanificador();
void conectarCoordinador();


#endif /* ESI_H_ */
