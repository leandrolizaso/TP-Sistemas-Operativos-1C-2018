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
bool ejecutoUltima;
char* ultima_linea;
char* motivo;
bool correr;
bool explotoParsi;


void inicializar(char* path);
void ejecutar(char* script);
void finalizar();


void enviar_operacion(t_mensaje_esi mensaje_esi);
void verificarEnvioCoordinador(int envio);
void liberarClaveValor(t_clavevalor claveValor);
t_clavevalor extraerClaveValor(t_esi_operacion sentencia,t_paquete* paquete);// recibe paquete para destruir por si falla

void verificarEnvioPlanificador(int envio,t_paquete* paquete);
void esiFinalizado(ssize_t read, t_paquete* paquete);

void liberarUltimaClaveValor();
void actualizarUltimoMensaje(t_mensaje_esi mensajeEnviado);

// Encapsulamiento

void crearLog();
void levantarConfig(char* path);
void conectarPlanificador();
void conectarCoordinador();

#endif /* ESI_H_ */
