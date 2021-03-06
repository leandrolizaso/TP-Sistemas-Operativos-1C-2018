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
	int pos;
	int tamanio;
	int id;
	int ultima_referencia;
}t_espacio_memoria;

typedef struct {
	char* ip_coordinador;
	char* puerto_coordinador;
	char* algoritmo;
	char* point_mount;
	char* nombre;
	int intervalo;
} t_config_instancia;

t_list* tabla;
t_config_instancia config;
t_log* logger;
t_config* config_aux;

int algoritmo;
int socket_coordinador;
int cantidad_entradas;
int tamanio_entradas;
int* indiceMemoria;
int id;
int time;
char* memoria;

void inicializar();
void definirAlgoritmo();
void crearLog();
void levantarConfig();
void conectarCoordinador();
void atenderConexiones();
void liberarRecursos();
void destructorEspacioMemoria(void* elem);

void notificarCoordinador(int tamanio,char* buffer);

void guardarPisandoClaveValor(t_clavevalor claveValor,int *indice);
void guardar(t_clavevalor claveValor,int *indice);


bool tengoLaClave(char* clave);
t_espacio_memoria* conseguirEspacioMemoria(char* clave);
t_espacio_memoria* conseguirEspacioMemoriaID(int id);

int entradasQueOcupa(char* valor);

bool tengoEntradas(int cantidad);
bool tengoLibres(int entradas,int *indice);
bool tengoAtomicas(int entradas,int *indice);
bool esAtomica(int indice);
int cantidadEntradasOcupadas(int indiceAux);

t_espacio_memoria* nuevoEspacioMemoria(t_clavevalor claveValor);
char* registrarNuevoEspacio(t_clavevalor claveValor,int* indice,int entradas,int* tamanio);


char* registrarEnIndiceMemoria(int id,int* indice,int entradas,int* tamanio);
void agregarClave(char* claves,int idPos,int* largo);
//void reemplazarValorLimpiandoIndice(t_espacio_memoria* espacio,char* valor, int* indice,int entradasNuevas);


void incrementarIndice(int *indice);
void avanzarIndice(int *indice,int veces);

int* compactarIndice(int* indice);
void agregarNoAtomicos(int* nuevoIndiceMemoria,int* indiceNuevo);
void asignar(int* unIndiceMemoria,int* indice,int valor,int cantidad);
void agregarAtomicos(int* nuevoIndiceMemoria,int*indiceNuevo);

void compactar(int* indice);
void actualizarIndiceMemoria();
void actualizarPosicionTabla();
void compactarMemoria();

void moverValor(void* elem);
void actualizarMemoria();

void reemplazarValor(t_espacio_memoria* espacio,char* valor);
void liberarSobrantes(int id,int cantidadNecesaria);
int posicion(int id);

void escribirEnArchivo(void* elem);
char* extraerValor(t_espacio_memoria* espacio);

void mostrar(void* elem);
void mostrarIndiceMemoria();

void dump ();

int entradasLibres();
char* liberar(int entradas,int* tamanio);
t_espacio_memoria* conseguirEspacioMenosUsado();
bool menosUsado(void* elem1,void* elem2);
bool esUnEspacioAtomico(void* elem);


#endif /* SRC_PRUEBACRISTIAN_H_ */
