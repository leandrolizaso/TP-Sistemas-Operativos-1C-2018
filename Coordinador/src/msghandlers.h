#ifndef SRC_MSGHANDLERS_H_
#define SRC_MSGHANDLERS_H_

void do_dummy_string(int socket, t_paquete* paquete);
void do_handhsake(t_dictionary* conexiones, int socket, t_paquete* operacion);
void do_esi_request(t_dictionary* conexiones, int socket, t_paquete* paquete);

#endif /* SRC_MSGHANDLERS_H_ */
