#ifndef SRC_MSGHANDLERS_H_
#define SRC_MSGHANDLERS_H_

void do_handhsake(int socket);
void do_esi_request(t_dictionary* conexiones, int socket, t_paquete* paquete);

#endif /* SRC_MSGHANDLERS_H_ */
