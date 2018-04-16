/*
 * sockets.h
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>

#define BACKLOG 100


/* Funciones para sockets */

int crear_server(char* puerto);
int conectar_a_server(char* ip, char* puerto);
int aceptar_conexion(int socketServidor);
char* get_ip_socket(int fd);
void cerrar_socket(int socket);
void multiplexar(char * puerto, int (*procesar_mensaje)(int));

/* Funciones para mandar/recibir mensajes */

int enviar_string(int fd ,char* mensaje);
char* recibir_string(int fd);
int enviar_int(int fd, int32_t numero);
int32_t recibir_int(int fd);

#endif /* SOCKETS_H_ */
