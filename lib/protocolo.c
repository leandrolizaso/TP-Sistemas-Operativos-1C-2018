#include <string.h>
#include <stdlib.h>
#include "protocolo.h"

int sizeof_clavevalor(t_clavevalor cv) {
	return sizeof(int) * 2 + strlen(cv.clave) + strlen(cv.valor);
}

void* serializar_clavevalor(t_clavevalor cv) {
	int l_clave = strlen(cv.clave);
	int l_valor = strlen(cv.valor);
	void* buffer = malloc(sizeof_clavevalor(cv));

	memcpy(buffer, &l_clave, sizeof(int));
	memcpy(buffer + sizeof(int), &l_valor, sizeof(int));
	memcpy(buffer + sizeof(int) * 2, cv.clave, l_clave);
	memcpy(buffer + sizeof(int) * 2 + l_clave, cv.valor, l_valor);

	return buffer;
}

t_clavevalor deserializar_clavevalor(void* buffer) {
	int* l_clave = buffer;
	int* l_valor = l_clave + 1;
	char* clave = (char*) l_valor + 1;
	char* valor = clave + *l_clave;

	t_clavevalor cv;
	cv.clave = clave;
	cv.valor = valor;
	return cv;
}

int sizeof_mensaje_esi(t_mensaje_esi mensaje_esi) {
	return sizeof(int) * 2 + sizeof_clavevalor(mensaje_esi.clave_valor);
}

void* serializar_mensaje_esi(t_mensaje_esi mensaje_esi) {
	void* buffer = malloc(sizeof_mensaje_esi(mensaje_esi));

	memcpy(buffer, &mensaje_esi.id_esi, sizeof(int));
	memcpy(buffer + sizeof(int), &mensaje_esi.keyword, sizeof(int));
	memcpy(buffer + sizeof(int) * 2, serializar_clavevalor(mensaje_esi.clave_valor), sizeof_clavevalor(mensaje_esi.clave_valor));

	return buffer;
}

t_mensaje_esi deserializar_mensaje_esi(void* buffer) {
	int* id_esi = buffer;
	int* keyword = id_esi+1;
	t_clavevalor* clave_valor = (t_clavevalor*) keyword+1;

	t_mensaje_esi mensaje_esi;
	mensaje_esi.id_esi = *id_esi;
	mensaje_esi.keyword = *keyword;
	mensaje_esi.clave_valor = deserializar_clavevalor(clave_valor);

	return mensaje_esi;
}
