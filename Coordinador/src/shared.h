#ifndef SRC_SHARED_H_
#define SRC_SHARED_H_

#include <commons/config.h>

typedef struct {
	int fd;
	char* nombre;
	int libre;
} t_instancia;

typedef struct {
	char* clave;
	int entradas;
	t_instancia* instancia;
} t_clave;

void inicializar_sincronizacion(void);
void finalizar_sincronizacion(void);

void registrar_instancia(int socket, char* nombre_instancia);
void destruir_instancia(t_instancia* instancia);
void registrar_clave(char* clave);
void destruir_clave(t_clave* clave);

bool no_existe_clave(char* clave);
t_clave* obtener_clave(char* clave);

int cant_instancias(void);
t_instancia* obtener_instancia(int indice);
void ordenar_instancias_segun(bool (*criterio)(void *, void *));

int config_entry_cant(void);
int config_entry_size(void);
int config_delay(void);
char* config_dist_algo(void);

#endif /* SRC_SHARED_H_ */
