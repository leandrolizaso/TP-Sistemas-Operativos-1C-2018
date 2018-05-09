#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

typedef struct {
	int socket;
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
void definirAlgoritmo(char* algoritmoString);

const int FIFO = 600;
const int SJFCD = 601;
const int SJFSD = 602;
const int HRRN = 602;

#endif /* PLANIFICADOR_H_ */
