#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include "msghandlers.h"

extern t_log* log_app;
extern t_log* log_operaciones;
int socket_planificador;

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

int numero_instancia(int cant_instancias, char* clave) {
	int caracter = *clave;
	caracter = caracter - 97;
	int rango = (26 / cant_instancias) + 1;
	return caracter / rango;
}

void do_handhsake(int socket) {
	log_debug(log_app, "Handshake Recibido (%d). Enviando Handshake.\n",
			socket);
	enviar(socket, HANDSHAKE_COORDINADOR, 0, NULL);
}

void do_instance_config(int socket, char* nombre) {
	//La instancia me pide config (CFG_ENTRYCANT,CFG_ENTRYSIZE)
	//Registro su nombre (el cual me mando en paquete->data) en la lista de instancias.
	//le mando su info
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
	case OPERACION_ESI_VALIDA: {
		int resultado_correcto = instancia_guardar(mensaje_esi.clave_valor);
		//validar el resultado, y decidir sin mandar exito_operacion o error_operacion
		if (resultado_correcto) {
			enviar(socket_esi, EXITO_OPERACION, 0, NULL);
		} else {
			enviar(socket_esi, ERROR_OPERACION, 0, NULL);
		}
		break;
	}
	case OPERACION_ESI_INVALIDA:
		enviar(socket_esi, ERROR_OPERACION, paquete->tamanio, paquete->data);
		break;
	}
	destruir_paquete(paquete);
}

int instancia_guardar(t_clavevalor clave_valor) {
	//Aca hablo con las instancias
	//Busco en que instancia esta la clave "mensaje_esi.clave_valor.clave"
	//Envio a esa instancia el mensaje guardar(clave_valor)
	//Pudo guardar?
	//	retorno exito
	//No pudo guardar... por que?
	//	Sin espacio:
	//  	return fallo
	//  Con espacio pero necesita defrag:
	//  	mando a todas las intancias defrag
	//		return instancia_guardar(clave_valor) //recursividad
	//	Desconexion?
	//		elimino esa clave de la tabla de claves
	//		return fallo
	return EXIT_SUCCESS;
}
