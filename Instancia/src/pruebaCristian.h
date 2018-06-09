/*
 * pruebaCritian.h
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#ifndef SRC_PRUEBACRISTIAN_H_
#define SRC_PRUEBACRISTIAN_H_

#include <stdio.h>
#include <stdlib.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>

typedef struct {
	char* clave;
	char* valor;
	int id;
}t_espacio_memoria;

typedef struct{
	bool esAtomico;
	int idOcupante;
	t_espacio_memoria* espacio;
}t_indice; // ?

typedef struct {
	char* ip_coordinador;
	int puerto_coordinador;
	char* algoritmo;
	char* point_mount;
	char* nombre;
	int intervalo;
} t_config_instancia;

t_indice* tablaIndices;
t_espacio_memoria* memoria;
t_config_instancia config;
t_log* logger;

int socket_coordinador;
int cantidad_entradas;
int tamanio_entradas;

void levantarConfig();
void conectarCoordinador();
void atenderConexiones();
void liberarRecursos();

#endif /* SRC_PRUEBACRISTIAN_H_ */
