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
#include <commons/collections/list.h>


#define CFG_IP  "ip_coordinador"
#define CFG_PORT  "port_coordinador"
#define CFG_ALGO  "distribution_replacement"
#define CFG_POINT  "point_mount"
#define CFG_NAME_INST  "name_instancia"
#define CFG_INTERVAL  "interval"
#define CFG_TAMANIO (tamanio)
#define CFG_CAPACIDAD (capacidad)

//Leer arcivo de configuracion
t_config* leer_config(int argc, char* argv[]);
int config_incorrecta(t_config* config);
void finalizar( t_config* config,t_log* logger);
void inicializar(t_config* config,t_log* logger);

//estructuras
typedef struct entrada t_entrada;
typedef int sig_atomic_t;
typedef struct t_paquete* paquete;
typedef struct Nodo t_Nodo;
typedef struct t_list listas;

int destruir_entrada (t_entrada* bloque);
void push (t_Nodo* pila, t_entrada* valores);
void crearEntrada (t_clavevalor* claveValor);
void verificarOperacion (int codOperacion, struct t_clavevalor* claveValor);
void dump (int intervalo);
void algoritmoCircular(t_entrada* bloque, t_list listaAlmacenamiento);
void esValorAtomico(t_entrada* lista);
void almacenarClaves (t_entrada* lista);
void guardarClaves(listaAlmacenamiento );
void buscarEntrada(struct t_entrada* listaAlmacenamiento, t_clavevalor* info);

#endif // INSTANCIA_H_INCLUDED
