#ifndef SRC_SHARED_H_
#define SRC_SHARED_H_

void registrar_instancia(t_instancia* instancia);
char* obtener_instancia_por_clave(char* clave);
void guardsar_clave_instancia(char* instancia, char* clave, int entradas);

#endif /* SRC_SHARED_H_ */
