#include "shared.h"
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>

extern t_dictionary* claves;
extern t_list* instancias;

void registrar_instancia(int socket, char* nombre_instancia){
	t_instancia* instancia_nueva = malloc(sizeof(t_instancia));
	instancia_nueva->fd = socket;
	instancia_nueva->nombre = nombre_instancia;
	list_add(instancias, instancia_nueva);
}

void destruir_instancia(t_instancia* instancia){
	free(instancia->nombre);
	free(instancia);
}
