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
bool imRunning;

void log_mensaje(char* mensaje);
void validarAperturaScript(FILE* fp);
bool codigoBueno(int codigo_operacion);
void ejecutarMensaje(t_mensaje_esi mensaje_esi,t_paquete* paquete,char* line);

void inicializar(char* path);
void ejecutar(char* script);
void finalizar();
void morir();

void enviar_operacion(t_mensaje_esi mensaje_esi,t_paquete* paquete);//recibe paquete para destruir por si falla
t_mensaje_esi extraer_mensaje_esi(t_esi_operacion operacion, t_paquete* paquete);//recibe  paquete porque usa extraerClaveValor
t_clavevalor extraerClaveValor(t_esi_operacion sentencia,t_paquete* paquete);//recibe paquete para destruir por si falla

void verificarEnvio(int envio,t_paquete* paquete, char* sujeto);

void crearLog();
void levantarConfig(char* path);
void conectarPlanificador();
void conectarCoordinador();

#endif /* ESI_H_ */
