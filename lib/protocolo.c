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
	int* l_clave = (int*)buffer;
	int* l_valor = l_clave+1;
	char* clave = (char*)l_valor+1;
	char* valor = clave+*l_clave;

	t_clavevalor cv;
	cv.clave = clave;
	cv.valor = valor;
	return cv;
}

void* serializar_mensaje_esi(t_mensaje_esi mensaje_esi) {

}

t_mensaje_esi* deserializar_mensaje_esi(void* buffer) {

}
