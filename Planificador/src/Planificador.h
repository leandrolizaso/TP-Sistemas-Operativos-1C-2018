#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

typedef struct {
	int ID;
	int estimacion_ant;

} proceso_esi_t;

void* consola(void* no_use);
void inicializar(char* path);
void finalizar();
int procesar_mensaje(int socket);
void agregar_espacio(char* buffer);
proceso_esi_t nuevo_processo_esi(void* data);
void send_ready_q(proceso_esi_t esi);
void send_waiting_q(proceso_esi_t esi);
void planificar();

#endif /* PLANIFICADOR_H_ */
