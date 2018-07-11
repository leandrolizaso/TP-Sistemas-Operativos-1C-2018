#include "Coordinador.h"
#include "shared.h"
#include <stdlib.h>
#include <string.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

extern t_dictionary* claves;
extern t_list* instancias;
extern t_config* config;

void registrar_instancia(int socket, char* nombre_instancia){
	t_instancia* instancia_nueva = malloc(sizeof(t_instancia));
	instancia_nueva->fd = socket;
	instancia_nueva->nombre = nombre_instancia;
	instancia_nueva->libre = config_get_int_value(config,CFG_ENTRYCANT);
	list_add(instancias, instancia_nueva);
}

void destruir_instancia(t_instancia* instancia){
	free(instancia->nombre);
	free(instancia);
}

void registrar_clave(char* clave){
	t_clave* nueva_clave = malloc(sizeof(t_clave));
	nueva_clave->clave = strdup(clave);
	nueva_clave->entradas = 0;
	nueva_clave->instancia = NULL;
	dictionary_put(claves, clave, nueva_clave);
}

void destruir_clave(t_clave* clave){
	free(clave->clave);
	free(clave);
}
