#include "Coordinador.h"
#include "shared.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

extern t_dictionary* claves;
extern t_list* instancias;
extern t_config* config;

pthread_mutex_t mutex_claves;
pthread_mutex_t mutex_instancias;

void inicializar_sincronizacion(){
	pthread_mutex_init(&mutex_claves, NULL);
	pthread_mutex_init(&mutex_instancias, NULL);
}

void finalizar_sincronizacion(){
	pthread_mutex_destroy(&mutex_claves);
	pthread_mutex_destroy(&mutex_instancias);
}

void registrar_instancia(int socket, char* nombre_instancia) {
	pthread_mutex_lock(&mutex_instancias);
	t_instancia* instancia_nueva = malloc(sizeof(t_instancia));
	instancia_nueva->fd = socket;
	instancia_nueva->nombre = strdup(nombre_instancia);
	instancia_nueva->libre = config_get_int_value(config, CFG_ENTRYCANT);
	list_add(instancias, instancia_nueva);
	pthread_mutex_unlock(&mutex_instancias);
}

void agregar_instancia(t_instancia* instancia){
	pthread_mutex_lock(&mutex_instancias);
	list_add(instancias,instancia);
	pthread_mutex_unlock(&mutex_instancias);
}

t_instancia* quitar_instancia(int indice){
	pthread_mutex_lock(&mutex_instancias);
	t_instancia* instancia = list_remove(instancias,indice);
	pthread_mutex_unlock(&mutex_instancias);
	return instancia;
}

void destruir_instancia(t_instancia* instancia) {
	free(instancia->nombre);
	free(instancia);
}

void registrar_clave(char* clave) {
	pthread_mutex_lock(&mutex_claves);
	t_clave* nueva_clave = malloc(sizeof(t_clave));
	nueva_clave->clave = strdup(clave);
	nueva_clave->entradas = 0;
	nueva_clave->instancia = NULL;
	dictionary_put(claves, clave, nueva_clave);
	pthread_mutex_unlock(&mutex_claves);
}

void destruir_clave(t_clave* clave) {
	free(clave->clave);
	free(clave);
}

bool no_existe_clave(char* clave) {
	pthread_mutex_lock(&mutex_claves);
	bool no_existe = !dictionary_has_key(claves, clave);
	pthread_mutex_unlock(&mutex_claves);
	return no_existe;
}

t_clave* obtener_clave(char* clave) {
	pthread_mutex_lock(&mutex_claves);
	t_clave* clave_obtenida = dictionary_get(claves, clave);
	pthread_mutex_unlock(&mutex_claves);
	return clave_obtenida;
}

int cant_instancias(void) {
	pthread_mutex_lock(&mutex_instancias);
	int cant_instancias = list_size(instancias);
	pthread_mutex_unlock(&mutex_instancias);
	return cant_instancias;
}

t_instancia* obtener_instancia(int indice) {
	pthread_mutex_lock(&mutex_instancias);
	t_instancia* instancia = list_get(instancias, indice);
	pthread_mutex_unlock(&mutex_instancias);
	return instancia;
}

void ordenar_instancias_segun(bool (*criterio)(void *, void *)) {
	pthread_mutex_lock(&mutex_instancias);
	list_sort(instancias, criterio);
	pthread_mutex_unlock(&mutex_instancias);
}

int config_entry_cant(void) {
	return config_get_int_value(config, CFG_ENTRYCANT);
}

int config_entry_size(void) {
	return config_get_int_value(config, CFG_ENTRYSIZE);
}

int config_delay(void) {
	return config_get_int_value(config, CFG_DELAY);
}

char* config_dist_algo(void) {
	return config_get_string_value(config, CFG_ALGO);
}

