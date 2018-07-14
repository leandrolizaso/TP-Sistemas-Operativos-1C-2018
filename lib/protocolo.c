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


void* serializar_status_clave(t_status_clave* status_clave, int* tamanio) {
        int len_instancia = strlen_null(status_clave->instancia);
        int len_instancia_now = strlen_null(status_clave->instancia_now);
        int len_valor = strlen_null(status_clave->valor);

        *tamanio = len_instancia + len_instancia_now + len_valor + 3 * sizeof(int);

        void *buffer = malloc(*tamanio);
        void *tmp = buffer;

        memcpy(tmp, &len_instancia, sizeof(int));
        tmp = tmp + sizeof(int);
        memcpy(tmp, &status_clave->instancia, len_instancia);
        tmp = tmp + len_instancia * sizeof(char);

        memcpy(tmp, &len_instancia_now, sizeof(int));
        tmp = tmp + sizeof(int);
        memcpy(tmp, &status_clave->instancia_now, len_instancia_now);
        tmp = tmp + len_instancia_now * sizeof(char);

        memcpy(tmp, &len_valor, sizeof(int));
        tmp = tmp + sizeof(int);
        memcpy(tmp, &status_clave->valor, len_valor);
        tmp = tmp + len_valor * sizeof(char);

        return buffer;
}

t_status_clave* deserializar_status_clave(void* buffer) {
        int offset = 0;

        int *len_instancia = buffer;
        offset += sizeof(int);
        char *instancia = buffer + offset;
        offset += *len_instancia * sizeof(char);
        int *len_instancia_now = buffer + offset;
        offset += sizeof(int);
        char *instancia_now = buffer+offset;
        offset += *len_instancia_now * sizeof(char);
        int *len_valor = buffer+offset;
        offset += sizeof(int);
        char *valor = buffer+offset;

        t_status_clave* status_clave = malloc(sizeof(t_status_clave));
        status_clave->instancia = strdup(instancia);
        status_clave->instancia_now = strdup(instancia_now);
        status_clave->valor = strdup(valor);
        return status_clave;
}

