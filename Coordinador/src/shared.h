#ifndef SRC_SHARED_H_
#define SRC_SHARED_H_

typedef struct {
	int fd;
	char* nombre;
	int ocupado;
} t_instancia;

typedef struct {
	char* clave;
	int entradas;
	t_instancia* instancia;
} t_clave;

void registrar_instancia(int socket, char* nombre_instancia);
void destruir_instancia(t_instancia* instancia);

#endif /* SRC_SHARED_H_ */
