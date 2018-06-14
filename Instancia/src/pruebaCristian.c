#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "pruebaCristian.h"

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
	memoria = list_create();
}

void atenderConexiones(){
	log_trace(logger,"atenderConexiones()");
	char* error;
	id = 1;
	int imRunning = 1;
	int indice = 0;

	t_paquete* paquete;
	paquete = recibir(socket_coordinador);

	while(imRunning){
		log_trace(logger,"atenderConexiones() #imRunning");

		switch (paquete->codigo_operacion) {
		case SAVE_CLAVE: {
			log_trace(logger, "SAVE_CLAVE");

			switch(algoritmo){

			case CIRC: {
				t_clavevalor claveValor = deserializar_clavevalor(
						paquete->data);
				if (tengoLaClave(claveValor.clave)) {
					guardarPisandoClaveValor(claveValor, &indice);
				} else {
					guardarClaveValor(claveValor, &indice);
				}
				notificarCoordinador(0);
				// en cada guardar deberia tener un notificar y depende del error/exito notificar
				// ahora notifico cero, para que todinho salga bien
				destruir_paquete(paquete);
				break;
			}
			case LRU: {

				break;
			}

			case BSU: {

				break;
			}

			}
			break;
		}
		case DUMP_CLAVE: {
			log_trace(logger, "DUMP_CLAVE");
			t_espacio_memoria* espacio = conseguirEspacioMemoria(paquete->data);
			if (espacio == NULL) {
				//notificar_coordinador(3); // <-- 3 = ERROR: se quiere hacer STORE de una clave que no se posee.
				// CLAVE_NO_TOMADA <-- abortar ESI
			} else {
				// mmap para guardar el valor con un texto plano con el nombre de la clave <<--- ia bere khe ago
				// solo eso?
			}
			destruir_paquete(paquete);
			notificarCoordinador(0);					//para que de bien
			break;
		}
		default: {
			error = string_from_format("El codigo de operación %d no es válido",
					paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			imRunning = 0;
			destruir_paquete(paquete);
			break;
		}
		}

	paquete = recibir(socket_coordinador);
	}
	destruir_paquete(paquete);
}

void liberarRecursos(){
	free(indiceMemoria);
	config_destroy(config_aux);
	log_destroy(logger);
	list_destroy_and_destroy_elements(memoria,&destructorEspacioMemoria);
}

void guardarPisandoClaveValor(t_clavevalor claveValor,int *indice){
	t_espacio_memoria* espacio = conseguirEspacioMemoria(claveValor.clave);//no verif por NULL dado que ya se hizo antes
	int entradasAnteriores = entradasQueOcupa(espacio->valor);
	int entradasNuevas = entradasQueOcupa(claveValor.valor);

	if(entradasNuevas > entradasAnteriores){
		if(tengoLibres(entradasNuevas,indice)){
			reemplazarValorLimpiandoIndice(espacio,claveValor.valor, indice,entradasNuevas);
			//notificarCoordinador(0) <-- comentado porque atenderConexiones sigue dando que tuti ok
		}else{
			if(tengoAtomicas(entradasNuevas,indice)){
				reemplazarValorLimpiandoIndice(espacio,claveValor.valor, indice,entradasNuevas);
				//notificarCoordinador(0) <-- comentado porque atenderConexiones sigue dando que tuti ok
			}else{
				//compactar: indiceMemoria = compactar(indice);
				// enviar al coord NECESITO_COMPACTAR, aca deberia cortar este flujo y
				// esperar un COMPACTA, compactar y luego enviar COMPACTACION_OK  y recibir un SEGUI o algo asi del coord
				// para que el segui este bien deberia guardar la ultima claveValor  que quise setiar para hacerlo de nuevo
					//  si puedo : notificarCoordinador(0)
					// si no puedo:	notificarCoordinador(1); // ERROR: "no hay espacio"
			}
		}
	}else{
		if(entradasNuevas < entradasAnteriores){
			liberarSobrantes(espacio->id,entradasNuevas);
			reemplazarValor(espacio,claveValor.valor);
		}else{
			reemplazarValor(espacio,claveValor.valor);
		}
	}
}

void guardarClaveValor(t_clavevalor claveValor,int *indice){
	int entradas = entradasQueOcupa(claveValor.valor);
	if(tengoLibres(entradas,indice)){
		registrarNuevoEspacio(claveValor,indice,entradas);
	}else{
		if(tengoAtomicas(entradas,indice)){
			registrarNuevoEspacio(claveValor,indice,entradas);
			//notificarCoordinador(0) <-- comentado porque atenderConexiones sigue dando que Tuti ok
		}else{
			//compactar: indiceMemoria = compactar(indice);
			// enviar al coord NECESITO_COMPACTAR, aca deberia cortar este flujo y
			// esperar un COMPACTA, compactar y luego enviar COMPACTACION_OK  y recibir un SEGUI o algo asi del coord
			// para que el segui este bien deberia guardar la ultima claveValor  que quise setiar para hacerlo de nuevo
				//  si puedo : notificarCoordinador(0)
				// si no puedo:	notificarCoordinador(1); // ERROR: "no hay espacio"
		}
	}
}

void notificarCoordinador(int respuesta){
	log_trace(logger,"notificador_coordinador(%d)",respuesta);
	enviar(socket_coordinador,RESPUESTA_INTANCIA,sizeof(int),&respuesta);
}

// PARA LISTAS

void destructorEspacioMemoria(void* elem){
	t_espacio_memoria* espacio = (t_espacio_memoria*) elem;
	free(espacio->clave);
	free(espacio->valor);
	free(espacio);
}

bool tengoLaClave(char* clave){
//	if(list_is_empty(memoria))
//		return false;                 <-- no necesario aparentemente
	bool contieneClave(void* unaParteDeMemoria){
		return string_equals_ignore_case(((t_espacio_memoria*)unaParteDeMemoria)->clave,clave);
	}

	return list_any_satisfy(memoria,&contieneClave);
}

t_espacio_memoria* conseguirEspacioMemoria(char* clave){
	bool contieneClave(void* unaParteDeMemoria){
		return string_equals_ignore_case(((t_espacio_memoria*)unaParteDeMemoria)->clave,clave);
	}
	return list_find(memoria,&contieneClave);
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
	return nuevoIndiceMemoria;
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

// AUXILIARES

void reemplazarValorLimpiandoIndice(t_espacio_memoria* espacio,char* valor, int* indice,int entradasNuevas){
	liberarSobrantes(espacio->id,0);
	registrarEnIndiceMemoria(espacio->id,indice,entradasNuevas);
	reemplazarValor(espacio,valor);
}

void registrarEnIndiceMemoria(int id,int* indice,int entradas){
	for(int i = 0; i<entradas ; i++){
		indiceMemoria[*indice + i] = id;
	}
	avanzarIndice(indice,entradas);
}

void reemplazarValor(t_espacio_memoria* espacio,char* valor){
	free(espacio->valor); // por string_duplicate al crear el t_espacio_memoria
	espacio->valor = string_duplicate(valor);
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

int cantidadEntradasOcupadas(int indiceAux){
	int acum = 0;
	int i = indiceAux;
	while(indiceMemoria[i] == indiceMemoria[i + 1] && i<cantidad_entradas-1){
		i++;
		acum++;
	}
	return acum;
}

t_espacio_memoria* nuevoEspacioMemoria(t_clavevalor claveValor){
	t_espacio_memoria* nuevoEspacio = malloc(sizeof(t_espacio_memoria));
	nuevoEspacio->clave = string_duplicate(claveValor.clave);
	nuevoEspacio->valor = string_duplicate(claveValor.valor);
	nuevoEspacio->id = id;
	id++;
	return nuevoEspacio;
}

void registrarNuevoEspacio(t_clavevalor claveValor,int* indice,int entradas){
	t_espacio_memoria* nuevoEspacio = nuevoEspacioMemoria(claveValor);
	list_add(memoria,nuevoEspacio);
	registrarEnIndiceMemoria(nuevoEspacio->id,indice,entradas);
}