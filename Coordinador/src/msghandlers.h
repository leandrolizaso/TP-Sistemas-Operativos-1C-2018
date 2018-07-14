#ifndef SRC_MSGHANDLERS_H_
#define SRC_MSGHANDLERS_H_

typedef struct{
	int socket;
	t_paquete* paquete;
} t_params;

void* do_handhsake(void* args);
void* do_esi_request(void* args);
void* do_status_request(void* args);
char* instancia_guardar(int keyword, t_clavevalor clave_valor);

#endif /* SRC_MSGHANDLERS_H_ */
