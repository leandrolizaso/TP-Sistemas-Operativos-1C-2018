#ifndef SRC_MSGHANDLERS_H_
#define SRC_MSGHANDLERS_H_

void do_handhsake(int socket, t_paquete* paquete);
int do_planificador_config(int socket, char* puerto);
void do_instance_config(int socket, char* nombre);
void do_esi_request(int socket, t_mensaje_esi mensaje_esi);
int instancia_guardar(t_clavevalor clave_valor);

typedef struct {
	int fd;
	int conectada;
	char* nombre;
} t_meta_instancia;

void destruir_meta_instancia(t_meta_instancia meta) {
	free(meta.nombre);
}

#endif /* SRC_MSGHANDLERS_H_ */
