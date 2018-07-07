#ifndef SRC_MSGHANDLERS_H_
#define SRC_MSGHANDLERS_H_

void do_handhsake(int socket, t_paquete* paquete);
int do_planificador_config(int socket, char* puerto);
void do_instance_config(int socket, char* nombre);
void do_esi_request(int socket, t_mensaje_esi mensaje_esi);
int instancia_guardar(int keyword, t_clavevalor clave_valor);

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

void destruir_instancia(t_instancia* instancia);

#endif /* SRC_MSGHANDLERS_H_ */
