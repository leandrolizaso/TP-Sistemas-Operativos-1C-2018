#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "pruebaCristian.h"

#include <commons/string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc,char** argv){
	inicializar(argv[1]);
	atenderConexiones();
	liberarRecursos();
	return EXIT_SUCCESS;
}

void inicializar(char* path){
	levantarConfig(path);
	crearLog();
	conectarCoordinador();
	indiceMemoria = calloc(cantidad_entradas, sizeof(int));
	tabla = list_create();  // 
	memoria = calloc(cantidad_entradas*tamanio_entradas,sizeof(char));
	signal(SIGALRM, dump(lista));
}

void atenderConexiones(){
	log_trace(logger,"atenderConexiones()");
	char* error;
	id = 1;
	int imRunning = 1;
	int indice = 0;
	time = 0;
	
	alarm(config.intervalo);
	
	t_paquete* paquete;
	paquete = recibir(socket_coordinador);

	while(imRunning){
		log_trace(logger,"atenderConexiones() #imRunning");

		switch (paquete->codigo_operacion) {
		case SAVE_CLAVE: {
			log_trace(logger, "SAVE_CLAVE");

			t_clavevalor claveValor = deserializar_clavevalor(paquete->data);
			if (tengoLaClave(claveValor.clave))
				guardarPisandoClaveValor(claveValor, &indice);
			else {
				guardar(claveValor, &indice);
			}
			destruir_paquete(paquete);
			break;
		}
		case DUMP_CLAVE: {
			log_trace(logger, "DUMP_CLAVE");
			t_espacio_memoria* espacio = conseguirEspacioMemoria(paquete->data);
			if (espacio == NULL) {
				//notificar_coordinador(3); // <-- 3 = ERROR: se quiere hacer STORE de una clave que no se posee.
				// CLAVE_NO_TOMADA <-- abortar ESI
				// no deberia pasar esto dado que el coord sabe cuando reemplazo una clave
			} else {
				escribirEnArchivo(espacio);
			}
			destruir_paquete(paquete);
			notificarCoordinador(0);
			break;
		}
		case COMPACTA:{
			compactar(&indice);
			break;
		}
		default: {
			error = string_from_format("El codigo de operación %d no es válido",paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			imRunning = 0;
			destruir_paquete(paquete);
			break;
		}
		}
	puts("\n Post Operacion quede asi:");
	list_iterate(tabla,&mostrar);
	time++;
	paquete = recibir(socket_coordinador);
	}
	destruir_paquete(paquete);
}

void liberarRecursos(){
	free(indiceMemoria);
	free(memoria);
	config_destroy(config_aux);
	log_destroy(logger);
	list_destroy_and_destroy_elements(tabla,&destructorEspacioMemoria);
}

void mostrar(t_espacio_memoria* espacio){
	char* valor = extraerValor(espacio);
	printf("\nClave: %s    Valor: %s\n",espacio->clave,valor);
	free(valor);
}

void guardar(t_clavevalor claveValor,int *indice){

	int entradas = entradasQueOcupa(claveValor.valor);

	if(tengoEntradas(entradas)){
		if(tengoLibres(entradas,indice)){
			registrarNuevoEspacio(claveValor,indice,entradas); // TODO: idem abajo
			notificarCoordinador(0);
		}else{
			if (tengoAtomicas(entradas, indice)) {

				switch (algoritmo) {

				case CIRC: {
					registrarNuevoEspacio(claveValor, indice, entradas);
					notificarCoordinador(0); // TODO: se debe tener en cuantas y que claves se reemplazaron,notif coord
					break;
				}
				case LRU: {

					break;
				}
				case BSU: {

					break;
				}

				}

			} else {
				// liberar(entradas - entradasLibres); TODO: ademas eliminar de la tabla
				enviar(socket_coordinador, NEED_COMPACTAR, 0, NULL);
			}
		}
	}else{
//		notificarCoordinador(1); // ERROR: "no hay espacio"
	}
}

void guardarPisandoClaveValor(t_clavevalor claveValor,int *indice){
	t_espacio_memoria* espacio = conseguirEspacioMemoria(claveValor.clave);//no verif por NULL dado que ya se hizo antes
	int entradasAnteriores = entradasQueOcupa(espacio->valor);
	int entradasNuevas = entradasQueOcupa(claveValor.valor);

	if(entradasNuevas > entradasAnteriores){
		//notificarCoordinador(4); Abortar ESI por querer hacer SET con un valor que ocupa mas entradas que el anterior
	} else {
		liberarSobrantes(espacio->id, entradasNuevas);
		reemplazarValor(espacio, claveValor.valor);
		espacio->ultima_referencia = time;
		notificarCoordinador(0);
	}
}

void notificarCoordinador(int indice){
	int* respuesta = malloc(sizeof(int*));
	*respuesta = indice;
	log_trace(logger,"notificador_coordinador(%d)",*respuesta);
	enviar(socket_coordinador,RESPUESTA_INTANCIA,sizeof(int),respuesta);
	free(respuesta);
}

// PARA LISTAS

void destructorEspacioMemoria(void* elem){
	t_espacio_memoria* espacio = (t_espacio_memoria*) elem;
	free(espacio->clave);
	free(espacio);
}

bool tengoLaClave(char* clave){
//	if(list_is_empty(memoria))
//		return false;                 <-- no necesario aparentemente
	bool contieneClave(void* unaParteDeMemoria){
		return string_equals_ignore_case(((t_espacio_memoria*)unaParteDeMemoria)->clave,clave);
	}

	return list_any_satisfy(tabla,&contieneClave);
}

t_espacio_memoria* conseguirEspacioMemoria(char* clave){
	bool contieneClave(void* unaParteDeMemoria){
		return string_equals_ignore_case(((t_espacio_memoria*)unaParteDeMemoria)->clave,clave);
	}
	return list_find(tabla,&contieneClave);
}

// inicializar

void levantarConfig(char* path) {

	config_aux = config_create(path);

	if (config_has_property(config_aux, "ip_coordinador")) {
		config.ip_coordinador = config_get_string_value(config_aux,"ip_coordinador");
	} else {
		perror( "No se encuentra la ip del Coordinador");
	}

	if (config_has_property(config_aux, "port_coordinador")) {
		config.puerto_coordinador = config_get_string_value(config_aux,"port_coordinador");
	} else {
		perror( "No se encuentra port_coordinador");
	}

	if (config_has_property(config_aux, "distribution_replacement")) {
		config.algoritmo = config_get_string_value(config_aux,"distribution_replacement");
	} else {
		perror( "No se encuentra distribution_replacement");
	}

	if (config_has_property(config_aux, "point_mount")) {
		config.point_mount = config_get_string_value(config_aux,"point_mount");
	} else {
		perror( "No se encuentra point_mount");
	}

	if (config_has_property(config_aux, "name_instancia")) {
		config.nombre= config_get_string_value(config_aux,"name_instancia");
	} else {
		perror( "No se encuentra name_instancia");
	}

	if (config_has_property(config_aux, "interval")) {
		config.intervalo= config_get_int_value(config_aux,"interval");
	} else {
		perror( "No se encuentra interval");
	}

	definirAlgoritmo();
}

void definirAlgoritmo(){
	if (string_equals_ignore_case(config.algoritmo, "CIRC"))
		algoritmo = CIRC;
		else if (string_equals_ignore_case(config.algoritmo, "LRU"))
			algoritmo = LRU;
			else if (string_equals_ignore_case(config.algoritmo, "BSU"))
				algoritmo = BSU;
	else {
		char* error = string_new();
		string_append(&error,"No esta contemplado el algoritmo ");
		string_append(&error,config.algoritmo);
		perror(error);
		free(error);
		exit(EXIT_FAILURE);
	}
}

void crearLog() {
	char* aux= string_new();
	string_append(&aux,config.nombre);
	string_append(&aux,".log");
	logger = log_create(aux, config.nombre, true, LOG_LEVEL_TRACE);
	free(aux);
}

void conectarCoordinador() {
	log_trace(logger,"conectarCoordionador()");
	socket_coordinador = conectar_a_server(config.ip_coordinador,config.puerto_coordinador);

	enviar(socket_coordinador, HANDSHAKE_INSTANCIA, strlen_null(config.nombre), config.nombre);
	t_paquete* paquete;
	paquete = recibir(socket_coordinador);
	if (paquete->codigo_operacion != HANDSHAKE_COORDINADOR) {
		log_error(logger, "Falló el handshake con el Coordinador");
		destruir_paquete(paquete);
	} else {
		int* ent = paquete->data;
		int* tam = paquete->data+sizeof(int);
		cantidad_entradas = *ent;
		tamanio_entradas= *tam;
		log_info(logger,"Handshake exitoso con el Coordinador");
		destruir_paquete(paquete);
	}
}

// Compactar

int* compactar(int* indice){
	int *nuevoIndiceMemoria = calloc(cantidad_entradas,sizeof(int));
	int indiceNuevo = 0;
	agregarNoAtomicos(nuevoIndiceMemoria,&indiceNuevo);
	agregarAtomicos(nuevoIndiceMemoria,&indiceNuevo);
	*indice = indiceNuevo;
	free(indiceMemoria);
	actualizarMemoria();
	return nuevoIndiceMemoria;
}


void actualizarMemoria(){
	int i = 0;
	while(indiceMemoria[i] != 0 && i< cantidad_entradas){
		int id = indiceMemoria[i];
		t_espacio_memoria* espacio = conseguirEspacioMemoriaID(id);
		memcpy(memoria + i*tamanio_entradas*sizeof(char),espacio->valor,strlen_null(espacio->valor));
		if(esAtomica(i))
			incrementarIndice(&i);
		else
			avanzarIndice(&i,cantidadEntradasOcupadas(i));
	}

}

t_espacio_memoria* conseguirEspacioMemoriaID(int id){
	bool contieneID(void* unaParteDeMemoria){
		return ((t_espacio_memoria*)unaParteDeMemoria)->id = id;
	}
	return list_find(tabla,&contieneID);
}

void agregarNoAtomicos(int* nuevoIndiceMemoria,int* indiceNuevo){
	int i = 0;
	int indice = 0;

	while(i< cantidad_entradas-1){

		if(!esAtomica(i) && indiceMemoria[i] != 0){
			int cantidad = cantidadEntradasOcupadas(i);
			asignar(nuevoIndiceMemoria,&indice,indiceMemoria[i],cantidad);
			i = i + cantidad -1 ;
		}
		i++;
	}
	*indiceNuevo = indice;
}

void asignar(int* unIndiceMemoria,int* indice,int valor,int cantidad){
	int acum = 0;
	while(acum<cantidad){
		unIndiceMemoria[*indice] = valor;
		incrementarIndice(indice);
		acum++;
	}
}

void agregarAtomicos(int* nuevoIndiceMemoria,int*indiceNuevo){

	int i = 0;

	while(i< cantidad_entradas){

		if(esAtomica(i) && indiceMemoria[i] != 0){
			nuevoIndiceMemoria[*indiceNuevo]= indiceMemoria[i];
			incrementarIndice(indiceNuevo);
		}else{
			int cantidad = cantidadEntradasOcupadas(i);
			i = i + cantidad - 1;
		}
		i++;
	}
}

// Auxiliares t_espacio_memoria

void escribirEnArchivo(t_espacio_memoria* espacio){ //No deberia recibir t_list*???
	char* punto_montaje = malloc(strlen(config.point_mount)+strlen(espacio->clave)+2);
	strcpy(punto_montaje, config.point_mount);
	string_append(&punto_montaje, "/");
	string_append(&punto_montaje, espacio->clave);
	struct stat sb;
	if (!(stat(config.point_mount, &sb) == 0 && S_ISDIR(sb.st_mode))) {
		mkdir(config.point_mount, S_IRWXU);
	}
	int tamanio = sizeof(char)*(espacio->tamanio + 1);
	char* valor = extraerValor(espacio);
	int fd = open(punto_montaje, O_RDWR | O_CREAT, S_IRWXU);
	ftruncate(fd, tamanio);
	char* memoria_mapeada = mmap(NULL, tamanio,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memcpy(memoria_mapeada, valor, tamanio);
	msync((void*) memoria_mapeada, tamanio,MS_SYNC);
	close(fd);
	free(punto_montaje);
	free(valor);
}

char* extraerValor(t_espacio_memoria* espacio){
	int tamanio = espacio->tamanio + 1;
	char* valor = calloc(tamanio,sizeof(char));
	memcpy(valor,espacio->valor,espacio->tamanio);
	return valor;
}

void reemplazarValorLimpiandoIndice(t_espacio_memoria* espacio,char* valor, int* indice,int entradasNuevas){
	liberarSobrantes(espacio->id,0);
	registrarEnIndiceMemoria(espacio->id,indice,entradasNuevas);
	reemplazarValor(espacio,valor);
}//ya no se usa dado que ahora los SET ocupan igual o menos cant de entradas

void reemplazarValor(t_espacio_memoria* espacio,char* valor){
	int pos = posicion(espacio->id);
	int tamanio = strlen(valor);
	espacio->valor = memoria + sizeof(char) * pos * tamanio_entradas;
	memcpy(memoria + sizeof(char) * pos * tamanio_entradas, valor,tamanio);
	espacio->tamanio = tamanio;
}

t_espacio_memoria* nuevoEspacioMemoria(t_clavevalor claveValor){
	t_espacio_memoria* nuevoEspacio = malloc(sizeof(t_espacio_memoria));
	nuevoEspacio->clave = string_duplicate(claveValor.clave);
	int pos = posicion(id);
	int tamanio = strlen(claveValor.valor);
	nuevoEspacio->valor = memoria + sizeof(char)*pos*tamanio_entradas;
	memcpy(memoria + sizeof(char)*pos*tamanio_entradas,claveValor.valor,tamanio);
	nuevoEspacio->tamanio = tamanio;
	nuevoEspacio->id = id;
	nuevoEspacio->ultima_referencia = time;
	id++;
	return nuevoEspacio;
}

void registrarNuevoEspacio(t_clavevalor claveValor,int* indice,int entradas){
	registrarEnIndiceMemoria(id,indice,entradas);
	t_espacio_memoria* nuevoEspacio = nuevoEspacioMemoria(claveValor);
	list_add(tabla,nuevoEspacio);
}

// Auxiliares indiceMemoria

void registrarEnIndiceMemoria(int id,int* indice,int entradas){
	for(int i = 0; i<entradas ; i++){
		int idPos = indiceMemoria[*indice + i];
		if( idPos != 0){
			bool eliminarEspacio(void* elem) {
				t_espacio_memoria* espacio = (t_espacio_memoria*) elem;
				return espacio->id == idPos;
			}
			list_remove_by_condition(tabla,&eliminarEspacio);
		}
		indiceMemoria[*indice + i] = id;
	}
	avanzarIndice(indice,entradas);
}

void liberarSobrantes(int id,int cantidadNecesaria){
	int pos = posicion(id);
	for(int i = pos + cantidadNecesaria ; indiceMemoria[i] == id ; i++){
		indiceMemoria[i] = 0;
	}
}

int posicion(int id){
	int i;
	for(i = 0;indiceMemoria[i] != id; i++);
	return i;
}

void incrementarIndice(int *indice){
	if(*indice < cantidad_entradas-1)
		*indice= *indice +1;
	else
		*indice = 0;
}

void avanzarIndice(int *indice,int veces){
	for(int i = 0; i < veces; i++){
		incrementarIndice(indice);
	}
}

int entradasQueOcupa(char* valor){
	int largo = strlen_null(valor)-1;
	int cantidad = largo/tamanio_entradas;
	if(largo % tamanio_entradas)
		return cantidad+1;
	else
		return cantidad;
}

bool tengoLibres(int entradas,int *indice){
	int indiceAux = *indice;
	int libres = 0;
	bool encontre = false;
	int vueltas = 0;
	while( vueltas <= 2 && !encontre){

		if(*indice == indiceAux)
			vueltas++;

		if(libres == entradas){
			encontre = true;
			if(indiceAux == 0)
				indiceAux = cantidad_entradas;
			*indice = indiceAux - entradas;
		}
		else{
			if (vueltas <= 2) {
				if (indiceMemoria[indiceAux] == 0 )
					libres++;
				else
					libres = 0;
			}
			incrementarIndice(&indiceAux);
		}

		if(indiceAux == 0 && libres!=entradas )
			libres = 0;

	}
	return encontre;
}

bool tengoAtomicas(int entradas,int *indice){
	int indiceAux = *indice;
	int libres = 0;
	bool encontre = false;
	int vueltas = 0;
	while( vueltas <= 2 && !encontre){

		if(*indice == indiceAux)
			vueltas++;

		if(libres == entradas){
			encontre = true;
			if(indiceAux == 0)
				indiceAux = cantidad_entradas;
			*indice = indiceAux - entradas;
		}
		else{
			if (vueltas <= 2) {
				if (indiceMemoria[indiceAux] == 0 || esAtomica(indiceAux))
					libres++;
				else{
					libres = 0;
					if(!esAtomica(indiceAux)){
						int cantidad = cantidadEntradasOcupadas(indiceAux);
						avanzarIndice(&indiceAux,cantidad-1);
					}
				}
			}
			incrementarIndice(&indiceAux);
		}

		if(indiceAux == 0 && libres!=entradas )
			libres = 0;
	}
	return encontre;
}

bool esAtomica(int indice){

	if(indice== cantidad_entradas - 1)
		return true;
	return indiceMemoria[indice] != indiceMemoria[indice + 1];
}

bool tengoEntradas(int cantidad){
	int libres = 0;
	bool encontre = false;
	for(int i = 0 ; i < cantidad_entradas && !encontre ; i++){

		if (indiceMemoria[i] == 0 || esAtomica(i))
			libres++;
		else if (!esAtomica(i)) {
			int cantidad = cantidadEntradasOcupadas(i);
			avanzarIndice(&i, cantidad - 1);
		}
		if(libres == cantidad)
			encontre = true;
	}
	return encontre;
}

int cantidadEntradasOcupadas(int indiceAux){
	int acum = 0;
	int i = indiceAux;
	while(indiceMemoria[i] == indiceMemoria[i + 1] && i<cantidad_entradas-1){
		i++;
		acum++;
	}
	return acum;
}
void dump (t_list* tabla){
	int ocupadas;
	int index= 0;
	int pos= 0;
	posicion(pos);
	//int size_tabla = list_size(tabla) + 1;

	ocupadas = cantidad_entradas;

	//void list_iterate(tabla, void(*closure)(void*));

	while (index < ocupadas + 1) {
			
		
			escribirEnArchivo(tabla);
			pos++;
			posicion(pos);
			index++;
	};
	alarm(config.intervalo);
	log_info(logger, "Se guardaron los datos");
  }
