#ifndef SRC_SHARED_H_
#define SRC_SHARED_H_

void registrar_instancia(char* nombre);
char* obtener_clave_instancia(char* clave);
void guardar_clave_instancia(char* instancia, char* clave, int entradas);

#endif /* SRC_SHARED_H_ */
