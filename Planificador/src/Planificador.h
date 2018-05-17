#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

typedef struct {
	int socket;
	int ID;
	double estimacion_ant;
	double duracion_raf_ant;
	char* recursoBloqueante;

} proceso_esi_t;

void* consola(void* no_use);
void inicializar(char* path);
void finalizar();
int procesar_mensaje(int socket);
void agregar_espacio(char* buffer);
proceso_esi_t* nuevo_processo_esi(int socket);
void planificar();
void definirAlgoritmo(char* algoritmoString);
void levantoConfig(char* path);
void estimar_proxima_rafaga(proceso_esi_t* esi);

#define FIFO 600
#define SJFCD 601
#define SJFSD 602
#define HRRN 603

#endif /* PLANIFICADOR_H_ */
