#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_
#include <commons/string.h>

typedef struct {
	int socket;
	int ID;
	double estimacion_ant;
	double duracion_raf_ant;
	double ejecuto_ant;
	char* recurso_bloqueante;
	_Bool viene_de_blocked;
	_Bool a_blocked;  //lo uso para lograr la atomicidad de las operaciones si el esi ejecutando es bloqueado por consola

} proceso_esi_t;

typedef struct {
	char* valor; //Es el nombre del recurso
	int ID_esi;
} t_clave;



void* consola(void* no_use);
void inicializar(char* path);
void finalizar();
int procesar_mensaje(int socket);
void agregar_espacio(char* buffer);
proceso_esi_t* nuevo_processo_esi(int socket);
void planificar();
void definirAlgoritmo(char* algoritmoString);
void levantoConfig(char* path);
void estimar_proxima_rafaga(void* esi);
void imprimir(t_list* esis_a_imprimir);
void bloquear(proceso_esi_t* esi, char* string);
_Bool esi_esperando(char* recurso);
void desbloquear(char* recurso);
void bloquear_key(char* clave);
bool esta_clave(char* clave);
void init_semaphores();
bool menor_tiempo(void*,void*);
void aumentar_rafaga(proceso_esi_t* esi);
_Bool hizo_get(proceso_esi_t* esi, char* recurso);
void error_de_esi(char* mensaje);
void liberar_esi(proceso_esi_t* esi);
void destructor_key(void* pointer);
void destructor_esi(void* pointer);
_Bool is_blocked(proceso_esi_t* esi);


#define FIFO 600
#define SJFCD 601
#define SJFSD 602
#define HRRN 603

#endif /* PLANIFICADOR_H_ */
