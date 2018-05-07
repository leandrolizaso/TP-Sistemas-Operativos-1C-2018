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

/* Planificador a ESI */

#define EJECUTAR_LINEA 101

/* ESI a Coordinador */

#define OPERACION_GET 102
#define OPERACION_SET 103
#define OPERACION_STORE 104

/* Coordinador a ESI y ESI a Planificador */

#define ERROR_OPERACION 105
#define EXITO_OPERACION 106
#define ESI_BLOQUEADO 107

/* OPERACION GENERICA*/

#define STRING_SENT 500

#endif /* PROTOCOLO_H_ */
