#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include "Coordinador.h"
#include "msghandlers.h"

extern t_log* log_app;
extern t_log* log_operaciones;

extern int socket_planificador;
extern char* ip_planificador;
extern char* puerto_planificador;

extern t_config* config;
extern t_dictionary* claves;
extern t_list* instancias;

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
	int rango = (26 / cant_instancias) + 1; //TODO: consultar sobre este +1
	return caracter / rango;
}

void do_handhsake(int socket, t_paquete* paquete) {
	log_debug(log_app, "Handshake Recibido (%d). Enviando Handshake.\n",
			socket);

	int tamanio = 0;
	void* data = NULL;

	switch (paquete->codigo_operacion) {
	case HANDSHAKE_ESI:
		break;
	case HANDSHAKE_PLANIFICADOR: {
		ip_planificador = get_ip_socket(socket);
		strcpy(puerto_planificador, paquete->data);
		break;
	}
	case HANDSHAKE_INSTANCIA: {
		t_meta_instancia instancia_nueva;
		instancia_nueva.conectada = true;
		instancia_nueva.fd = socket;
		strcpy(instancia_nueva.nombre, paquete->data);
		list_add(instancias, instancia_nueva);

		tamanio = sizeof(int) * 2;
		data = malloc(tamanio);
		data = config_get_int_value(config, CFG_ENTRYCANT);
		(data + 1) = config_get_int_value(config, CFG_ENTRYSIZE);
		break;
	}
	default:
		break;
	}
	enviar(socket, HANDSHAKE_COORDINADOR, tamanio, data);
}

void do_esi_request(int socket_esi, t_mensaje_esi mensaje_esi) {
	char* valor_mostrable = mensaje_esi.clave_valor.valor;
	char* clave = mensaje_esi.clave_valor.clave;
	if (valor_mostrable == NULL) {
		valor_mostrable = "";
	}
	log_info(log_operaciones, "ESI %d\t%s %s %s\n", mensaje_esi.id_esi,
			keywordtos(mensaje_esi.keyword), mensaje_esi.clave_valor.clave,
			valor_mostrable);

	log_debug(log_app, "Simulamos espera para ESI%d...", mensaje_esi.id_esi);
	sleep(config_get_int_value(config, CFG_DELAY));
	log_debug(log_app, "Seguimos!");

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

	if (!dictionary_has_key(claves, clave) && mensaje_esi.keyword != 0) {
		enviar(socket_esi, ERROR_OPERACION, 41,
				"No podes hacer eso si la clave no existe");
	} else if (mensaje_esi.keyword == 0) {
		dictionary_put(claves, clave, NULL);
	}

	if (socket_planificador == -1) { //solo la primera conexion al planificador
		socket_planificador = conectar_a_server(ip_planificador,
				puerto_planificador);
	}

	int tamanio = strlen(clave) + 1;
	enviar(socket_planificador, operacion, tamanio, clave);

	t_paquete* paquete = recibir(socket_planificador);
	switch (paquete->codigo_operacion) {
	case OPERACION_ESI_VALIDA: {
		//TODO: elegir a que instancia hablarle, o si hablarle
		int resultado_error = instancia_guardar(mensaje_esi.clave_valor);
		//TODO: actualizar la instancia que tiene la clave (si era NULL)
		if (resultado_error) {
			enviar(socket_esi, ERROR_OPERACION, 0, NULL);
		} else {
			enviar(socket_esi, EXITO_OPERACION, 0, NULL);
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
	//TODO: Guardar instancia
	//Busco en que instancia esta la clave "clave_valor.clave"
	//Si esa clave no esta en ninguna instancia -> numero_instancia(lista_instancias.size, clave_valor.clave)
	//Envio a esa instancia el mensaje guardar(clave_valor)
	//Pudo guardar?
	//	retorno exito
	//No pudo guardar... por que?
	//	Sin espacio:
	//  	return fallo //el check es hasta aca
	//  Con espacio pero necesita defrag:
	//  	mando a todas las intancias defrag
	//		return instancia_guardar(clave_valor) //recursividad
	//	Desconexion?
	//		elimino esa clave de la tabla de claves
	//		return fallo
	return EXIT_SUCCESS;
}
