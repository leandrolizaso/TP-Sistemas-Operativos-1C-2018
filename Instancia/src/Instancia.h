/*
 * Instancia.h
 *
 *  Created on: 8 jun. 2018
 *      Author: utnso
 */

#ifndef SRC_INSTANCIA_H_
#define SRC_INSTANCIA_H_

#include <commons/log.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>

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

int socket_coordinador;
int cantidad_entradas;
int tamanio_entradas;
int* indice_memoria;

void notificar_coordinador(int respuesta);
void guardar_pisando_clavevalor(t_clavevalor claveValor);
void guardar_clavevalor(t_clavevalor claveValor);

bool tengo_clave(char* clave);
t_espacio_memoria* conseguir_espacio_memoria(char* clave);

void inicializar();
void crear_log();
void levantar_config();
void conectar_coordinador();
void atender_conexiones();

int entradas_que_ocupa(char* valor);

void liberar_recursos();


#endif /* SRC_INSTANCIA_H_ */
