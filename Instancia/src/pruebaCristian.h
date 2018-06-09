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
	char* puerto_coordinador;
	char* algoritmo;
	char* point_mount;
	char* nombre;
	int intervalo;
} t_config_instancia;

t_list* tablaIndices;
t_list* memoria;
t_config_instancia config;
t_log* logger;
t_config* config_aux;

int socket_coordinador;
int cantidad_entradas;
int tamanio_entradas;


void notificar_coordinador(int respuesta);
void guardarPisandoClaveValor(t_clavevalor claveValor);
void guardarClaveValor(t_clavevalor claveValor);

bool tengoLaClave(char* clave);
t_espacio_memoria* conseguirEspacioMemoria(char* clave);

void inicializar();
void crearLog();
void levantarConfig();
void conectarCoordinador();
void atenderConexiones();

int entradasQueOcupa(char* valor);

void destruirMemoria();
void destruirTablaIndices();
void liberarRecursos();

void crearTablaIndices();
void crearMemoria();

#endif /* SRC_PRUEBACRISTIAN_H_ */
