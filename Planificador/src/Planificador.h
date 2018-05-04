#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

void* consola(void* no_use);
void inicializar(char* path);
void finalizar();
int procesar_mensaje(int socket);
void agregar_espacio(char* buffer);

#endif /* PLANIFICADOR_H_ */
