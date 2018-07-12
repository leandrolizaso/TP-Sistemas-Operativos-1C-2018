#include <string.h>
#include <stdlib.h>
#include "protocolo.h"

int strlen_null(char* str) {
	if (str == NULL) {
		return 0;
	}
	return strlen(str) + 1;
}

int sizeof_clavevalor(t_clavevalor cv) {
	return sizeof(int) * 2 + (strlen_null(cv.clave) + strlen_null(cv.valor)) * sizeof(char);
}

void* serializar_clavevalor(t_clavevalor cv) {
	int l_clave = strlen_null(cv.clave);
	int l_valor = strlen_null(cv.valor);
	int size_cv = sizeof_clavevalor(cv);
	void* buffer = malloc(size_cv);

	memcpy(buffer, &l_clave, sizeof(int));
	memcpy(buffer + sizeof(int), &l_valor, sizeof(int));
	memcpy(buffer + sizeof(int) * 2, cv.clave, l_clave * sizeof(char));
	if (l_valor > 0) {
		memcpy(buffer + sizeof(int) * 2 + l_clave * sizeof(char), cv.valor,l_valor * sizeof(char));
	}

	return buffer;
}

t_clavevalor deserializar_clavevalor(void* buffer) {
	int* l_clave = buffer;
	int* l_valor = l_clave + 1;
	char* clave = (char*) (l_valor + 1);
	char* valor;

	if (*l_valor > 0) {
		valor = clave + *l_clave;
	} else {
		valor = NULL;
	}

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

	void* subbuffer = serializar_clavevalor(mensaje_esi.clave_valor);
	memcpy(buffer + sizeof(int) * 2, subbuffer,sizeof_clavevalor(mensaje_esi.clave_valor));
	free(subbuffer);

	return buffer;
}

t_mensaje_esi deserializar_mensaje_esi(void* buffer) {
	int* id_esi = buffer;
	int* keyword = id_esi + 1;
	t_clavevalor* clave_valor = (t_clavevalor*) (keyword + 1);

	t_mensaje_esi mensaje_esi;
	mensaje_esi.id_esi = *id_esi;
	mensaje_esi.keyword = *keyword;
	mensaje_esi.clave_valor = deserializar_clavevalor(clave_valor);

	return mensaje_esi;
}


void* serializar_status_clave(t_status_clave status_clave, int* tamanio){
	int len_valor = strlen_null(status_clave.valor);
	*tamanio = sizeof(t_status_clave)+len_valor - 4;
	void* buffer = malloc(*tamanio);
	memcpy(buffer,&status_clave.instancia,sizeof(int));
	memcpy(buffer+sizeof(int),&status_clave.instancia_now,sizeof(int));
	memcpy(buffer+sizeof(int)*2,status_clave.valor,sizeof(char)*len_valor);
	return buffer;
}

t_status_clave deserializar_status_clave(void* buffer){
	int* instancia = buffer;
	int* instancia_now = instancia + 1;
	char* valor = (char*) (instancia_now + 1);

	t_status_clave status_clave;
	status_clave.instancia = *instancia;
	status_clave.instancia_now = *instancia_now;
	status_clave.valor = strdup(valor);
	free(buffer);
	return status_clave;
}
