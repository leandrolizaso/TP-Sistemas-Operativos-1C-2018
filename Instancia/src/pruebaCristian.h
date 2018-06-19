/*
 * pruebaCritian.h
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#ifndef SRC_PRUEBACRISTIAN_H_
#define SRC_PRUEBACRISTIAN_H_

#include <commons/log.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>

#define CIRC 700
#define LRU 701
#define BSU 702

typedef struct {
	char* clave;
	char* valor;
	int id;
}t_espacio_memoria;

typedef struct {
	char* ip_coordinador;
	char* puerto_coordinador;
	char* algoritmo;
	char* point_mount;
	char* nombre;
	int intervalo;
} t_config_instancia;

t_list* memoria;
t_config_instancia config;
t_log* logger;
t_config* config_aux;

int algoritmo;
int socket_coordinador;
int cantidad_entradas;
int tamanio_entradas;
int* indiceMemoria;
int id;

void inicializar();
void definirAlgoritmo();
void crearLog();
void levantarConfig();
void conectarCoordinador();
void atenderConexiones();
void liberarRecursos();
void destructorEspacioMemoria(void* elem);

void notificarCoordinador(int respuesta);

void guardarPisandoClaveValor(t_clavevalor claveValor,int *indice);

void guardarCIRC(t_clavevalor claveValor,int *indice);
void guardarLRU(t_clavevalor claveValor,int *indice);
void guardarBSU(t_clavevalor claveValor,int *indice);

bool tengoLaClave(char* clave);
t_espacio_memoria* conseguirEspacioMemoria(char* clave);

int entradasQueOcupa(char* valor);

bool tengoEntradas(int cantidad);
bool tengoLibres(int entradas,int *indice);
bool tengoAtomicas(int entradas,int *indice);
bool esAtomica(int indice);
int cantidadEntradasOcupadas(int indiceAux);

t_espacio_memoria* nuevoEspacioMemoria(t_clavevalor claveValor);
void registrarNuevoEspacio(t_clavevalor claveValor,int* indice,int entradas);

void incrementarIndice(int *indice);
void avanzarIndice(int *indice,int veces);

int* compactar(int* indice);
void agregarNoAtomicos(int* nuevoIndiceMemoria,int* indiceNuevo);
void asignar(int* unIndiceMemoria,int* indice,int valor,int cantidad);
void agregarAtomicos(int* nuevoIndiceMemoria,int*indiceNuevo);

void reemplazarValor(t_espacio_memoria* espacio,char* valor);
void liberarSobrantes(int id,int cantidadNecesaria);
int posicion(int id);

void reemplazarValorLimpiandoIndice(t_espacio_memoria* espacio,char* valor, int* indice,int entradasNuevas);
void registrarEnIndiceMemoria(int id,int* indice,int entradas);



#endif /* SRC_PRUEBACRISTIAN_H_ */
