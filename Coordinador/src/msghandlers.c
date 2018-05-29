#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/collections/dictionary.h>

void registrar_conexion(t_dictionary* conexiones, int socket, int operacion) {
	char str[12];
	sprintf(str, "%d", socket);
	dictionary_put(conexiones, str, (void*) operacion);
}

int conexion_invalida(t_dictionary* conexiones, socket, int tipo_handshake) {
	int hanshake = dictionary_get(conexiones, socket);
	if (hanshake == tipo_handshake) {
		return true;
	}
	return false;
}

void do_dummy_string(int socket, t_paquete* paquete) {
	char* recibido = (char*) (paquete->data);
	printf("Recibido mensaje de prueba: \"%s\". Enviando burla.\n", recibido);

	int len = strlen(recibido);

	char* enviado = malloc(len);
	strcpy(enviado, recibido);

	int i;
	for (i = 0; i < len; i++) {
		switch (enviado[i]) {
		case 'a':
		case 'e':
		case 'o':
		case 'u':
			enviado[i] = 'i'; //ñiñiñiñi
		}
	}
	enviar(socket, STRING_SENT, len, enviado);
	free(enviado);
}

void do_handhsake(int socket) {
	printf("Handshake Recibido (%d). Enviando Handshake.\n", socket);
	enviar(socket, HANDSHAKE_COORDINADOR, 0, NULL);
}

void do_esi_request(t_dictionary* conexiones, int socket, t_paquete* paquete) {
}
