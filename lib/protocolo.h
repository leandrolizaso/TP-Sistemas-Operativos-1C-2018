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

#define SIGUIENTE 111

/* Planificador a ESI */

#define EJECUTAR_LINEA 101
#define FINALIZAR 112

/* Coordinador a ESI */

#define DONE 113

/* ESI a Coordinador */

#define OPERACION_GET 102
#define OPERACION_SET 103
#define OPERACION_STORE 104

/* Coordinador a ESI y ESI a Planificador */

#define ERROR_OPERACION 105
#define EXITO_OPERACION 106
#define ESI_FINALIZADO 107


/* Coodinador a Planificador */
#define GET_CLAVE 108
#define STORE_CLAVE 109

/* Planificador a coordinador */
#define CLAVE_TOMADA 110


/* OPERACION GENERICA*/

#define STRING_SENT 500

#endif /* PROTOCOLO_H_ */
