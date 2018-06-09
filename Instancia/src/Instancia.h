#ifndef INSTANCIA_H_INCLUDED
#define INSTANCIA_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
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
#define CFG_TAMANIO (tamanio)


//Leer arcivo de configuracion
t_config* leer_config(int argc, char* argv[]);
int config_incorrecta(t_config* config);
void finalizar( t_config* config,t_log* logger);
void inicializar(t_config* config,t_log* logger);

//estructuras
typedef struct entrada* t_entrada;
typedef int sig_atomic_t;
typedef struct t_paquete* paquete;
typedef struct Nodo* Nodo;

int destruir_entrada (struct t_entrada* bloque);
void push (Nodo* pila, t_entrada* valores);
void crearEntrada (struct t_clavevalor* claveValor);
void verificarOperacion (int codOperacion, struct t_clavevalor* claveValor);
void dump (int intervalo);

#endif // INSTANCIA_H_INCLUDED

#endif // INSTANCIA_H_INCLUDED
