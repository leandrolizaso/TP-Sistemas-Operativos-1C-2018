#ifndef OPCIONALINSTNCIA_H_INCLUDED
#define OPCIONALINSTNCIA_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>


#define CIRC  345
#define LRU  346
#define BSu 347

typedef struct{

char* ip_coordinador;
int puerto_coordinador;
char* algoritmo;
char* point_mount;
char* nombre;
int intervalo;
}t_config_instancia;

typedef struct {
	char* clave;
	char* valor;
	int pos;
}t_espacio_memoria;

typedef struct{
	int quienOcupa;
	t_espacio_memoria* espacio;
}t_nodo;

void escribirMemoria_Circ(char* valor);
int ocupaValor(int largo);
bool meQuedanEntradas(int largo);

void ejecutar(t_config* config, t_log* logger);
void* crearMemoria();
void inicializarTabla();
void reemplazarAtomico(char* valor);
void escribirMemoria_Circ(char* valor);
bool hayEntradasDisponible(int largo);

void set(t_clavevalor claveValor);
//Leer arcivo de configuracion
t_config* leer_config(int argc, char* argv[]);
int config_incorrecta(t_config* config);
void finalizar( t_config* config,t_log* logger);
void inicializar(t_config* config,t_log* logger);


#endif // INSTANCIA_H_INCLUDED
