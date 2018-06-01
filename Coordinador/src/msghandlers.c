#include <stdbool.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/log.h>
#include <commons/collections/dictionary.h>

extern t_log* log_app;
extern t_log* log_operaciones;

void do_handhsake(int socket) {
	log_debug(log_app,"Handshake Recibido (%d). Enviando Handshake.\n", socket);
	enviar(socket, HANDSHAKE_COORDINADOR, 0, NULL);
}

void do_esi_request(t_dictionary* conexiones, int socket, t_paquete* paquete) {
	//TODO: Implementar
	//Recibo "get/set/store
	//Si es "get" o "store" primero le aviso al planificador con GET_CLAVE/STORE_CLAVE
	//Si el planificador me dice que estamos bien (CLAVE_TOMADA/CLAVE_LIBERADA?), se continua. Sino, se devuelve error al esi (ERROR_OPERACION).
	//Si es GET, no hago nada mas.
	//Si es Set o Store, tengo que buscar en "mi estructura" para saber a que instancia hablarle
	//Validar algoritmo (equitative load por el momento)
	//Hablar con instancia, recibir resultado
	//Si la instancia me dice que estamos bien, se devuelve exito al esi
	//Si instancia me dice que no tiene idea de que le estoy hablando, la clave se perdio. Devolvemos error.
}
