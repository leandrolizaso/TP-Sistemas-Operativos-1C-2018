#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>

extern t_log* log_app;
extern t_log* log_operaciones;
extern int socket_planificador;

char* keywordtos(int keyword) {
	switch (keyword) {
	case 0:
		return "GET";
	case 1:
		return "SET";
	case 2:
		return "STORE";
	default:
		return "WTF";
	}
}

void do_handhsake(int socket) {
	log_debug(log_app, "Handshake Recibido (%d). Enviando Handshake.\n",
			socket);
	enviar(socket, HANDSHAKE_COORDINADOR, 0, NULL);
}

int do_planificador_config(int socket) {
	char* ip_puerto = recibir_string(socket);
	char* separador = strchr(ip_puerto, ':');
	*separador = '\0'; //al reemplazar el separador, tengo 2 strings null-terminadas
	int planificador = conectar_a_server(ip_puerto, (separador + 1));
	//TODO: Definir si le tengo que mandar algo o no.
	return planificador;
}

void do_esi_request(int socket_esi, t_mensaje_esi mensaje_esi) {
	log_info(log_operaciones, "ESI %d\t%s %s %s\n", mensaje_esi.id_esi,
			keywordtos(mensaje_esi.keyword), mensaje_esi.clave_valor.clave,
			mensaje_esi.clave_valor.valor);

	int operacion; //Este switch es hasta que el planificador pueda usar el mensaje polimorficamente
	switch (mensaje_esi.keyword) {
	case 0:
		operacion = GET_CLAVE;
		break;
	case 1:
		operacion = SET_CLAVE;
		break;
	case 2:
		operacion = STORE_CLAVE;
		break;
	default:
		log_error(log_app, "Se recibio una operacion invalida del ESI %d: %d",
				mensaje_esi.id_esi, mensaje_esi.keyword);
	}

	int tamanio = sizeof_mensaje_esi(mensaje_esi);
	enviar(socket_planificador, operacion, tamanio, &mensaje_esi);

	t_paquete* paquete = recibir(socket_planificador);
	switch (paquete->codigo_operacion) {
	case OPERACION_ESI_VALIDA:
		//Aca hablo con las instancias
		enviar(socket_esi,EXITO_OPERACION,0,NULL);
		break;
	case OPERACION_ESI_INVALIDA:
		enviar(socket_esi, ERROR_OPERACION, paquete->tamanio, paquete->data);
		break;
	}
	destruir_paquete(paquete);
}
