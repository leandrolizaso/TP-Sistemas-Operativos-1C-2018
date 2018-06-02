/*
 * protocolo.h
 *
 *  Created on: 23 abr. 2018
 *      Author: utnso
 */

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

/* Handshakes entre procesos */

#define HANDSHAKE_ESI 100
#define HANDSHAKE_COORDINADOR 200
#define HANDSHAKE_PLANIFICADOR 300
#define HANDSHAKE_INSTANCIA 400

/* ESI a Planificador*/

#define SIGUIENTE 301

/* Planificador a ESI */

#define EJECUTAR_LINEA 302
#define FINALIZAR 303

/* Coordinador a ESI */
#define ERROR_OPERACION 201 // string con fallo
#define EXITO_OPERACION 202 // void
#define VOLVE 203

/* ESI a Coordinador */
#define OPERACION 101 // paquetote

/*ESI a Planificador */
#define ESI_FINALIZADO 102 // renombrar ESI_EOF


/* Coodinador a Planificador */
#define GET_CLAVE 203 //string
#define STORE_CLAVE 204 //string
#define SET_CLAVE 205 //string

/* Planificador a coordinador */
#define CLAVE_TOMADA 114 // <- habria que borrar dado que ESI_INVALIDA generaliza esto
#define OPERACION_ESI_VALIDA 304
#define OPERACION_ESI_INVALIDA 305


/* Serializacion/Desserializacion */

typedef struct {
	char* clave;
	char* valor;
} t_clavevalor;

void* serializar_clavevalor(t_clavevalor clave_valor);
t_clavevalor deserializar_clavevalor(void* buffer);


typedef struct{
	int id_esi;
	int keyword;
	t_clavevalor clave_valor;
} t_mensaje_esi;

void* serializar_mensaje_esi(t_mensaje_esi mensaje_esi);
t_mensaje_esi deserializar_mensaje_esi(void* buffer);

#endif /* PROTOCOLO_H_ */
