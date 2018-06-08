#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>
#include "msghandlers.h"

extern t_log* log_app;
extern t_log* log_operaciones;

int socket_planificador =-1;
char* ip_planificador = NULL;
char* puerto_planificador = NULL;

//TODO: lista de instancias {nombre,fd,isConnect}

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
	log_debug(log_app, "Handshake Recibido (%d). Enviando Handshake.\n",socket);

	int tamanio = 0;
	void* data = NULL;

	switch(paquete->codigo_operacion){
	case HANDSHAKE_ESI:
		break;
	case HANDSHAKE_PLANIFICADOR:{
		ip_planificador = get_ip_socket(socket);
		strcpy(puerto_planificador,paquete->data);
		break;
	}
	case HANDSHAKE_INSTANCIA:
		//TODO: guardo el socket en la lista de instancias
		//guardo el nombre de la instancia (paquete->data)
		//data va a ser una estructura de configuracion (nuero_entradas,size_entradas)
		//y tamanio sera sizeof(int)*2
	}
	enviar(socket, HANDSHAKE_COORDINADOR, tamanio, data);
}


void do_esi_request(int socket_esi, t_mensaje_esi mensaje_esi) {
	char* valor_mostrable = mensaje_esi.clave_valor.valor;
	if( valor_mostrable == NULL){
		valor_mostrable = "";
	}
	log_info(log_operaciones, "ESI %d\t%s %s %s\n", mensaje_esi.id_esi,
			keywordtos(mensaje_esi.keyword), mensaje_esi.clave_valor.clave,
			valor_mostrable);

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



	if (socket_planificador == -1){ //solo la primera conexion al planificador
	    socket_planificador = conectar_a_server(ip_planificador, puerto_planificador);
	}
	char* clave = mensaje_esi.clave_valor.clave;
	int tamanio = strlen(clave)+1;
	enviar(socket_planificador, operacion, tamanio, clave);

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
	//TODO: Aca hablo con las instancias
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
