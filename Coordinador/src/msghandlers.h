#ifndef SRC_MSGHANDLERS_H_
#define SRC_MSGHANDLERS_H_

void do_handhsake(int socket);
int do_planificador_config();
void do_esi_request(int socket, t_mensaje_esi mensaje_esi);

#endif /* SRC_MSGHANDLERS_H_ */
