#ifndef SRC_COORDINADOR_H_
#define SRC_COORDINADOR_H_

#define CFG_PORT  "listen_port"
#define CFG_ALGO  "distribution_algorithm"
#define CFG_ENTRYCANT  "entry_cant"
#define CFG_ENTRYSIZE  "entry_size"
#define CFG_DELAY  "delay"

#define CONTINUE_COMMUNICATION  1
#define END_CONNECTION -1

t_config* leer_config(int argc, char* argv[]);
int config_incorrecta(t_config* config);
int recibir_mensaje(int socket);
void loggear(char* level_char, char* template, ...);

#endif /* SRC_COORDINADOR_H_ */
