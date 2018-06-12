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
	indiceMemoria = calloc(cantidad_entradas, sizeof(int));;
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
		switch(paquete->codigo_operacion){
		case SAVE_CLAVE:{
			log_trace(logger,"SAVE_CLAVE");
			t_clavevalor claveValor = deserializar_clavevalor(paquete->data);
			if (tengoLaClave(claveValor.clave)) {
				guardarPisandoClaveValor(claveValor,&indice);
			} else {
				guardarClaveValor(claveValor,&indice);
			}
			notificarCoordinador(0);
			// en cada guardar deberia tener un notificar y depende del error/exito notificar
			// ahora notifico cero, para que todinho salga bien
			destruir_paquete(paquete);
		break;}
		case DUMP_CLAVE:{
			log_trace(logger,"DUMP_CLAVE");
			t_espacio_memoria* memory = conseguirEspacioMemoria(paquete->data);
			if(memory == NULL){
				//notificar_coordinador(3); // <-- 3 = ERROR: se quiere hacer STORE de una clave que no se posee.
			}else{
				// mmap para guardar el valor <<--- ia bere khe ago
				//t_indice* indice = list_get(tablaIndices, memory->id);
				//indice->idOcupante = -1;
				free(memory);
			}

			destruir_paquete(paquete);
			notificarCoordinador(0);//para que de bien
		break;}
		default:{
			error = string_from_format("El codigo de operación %d no es válido", paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			imRunning = 0;
			destruir_paquete(paquete);
		break;}
		}
	paquete = recibir(socket_coordinador);
	}
	destruir_paquete(paquete);
}

void guardarPisandoClaveValor(t_clavevalor claveValor,int *indice){
	t_espacio_memoria* espacio = conseguirEspacioMemoria(claveValor.clave);//no verif por NULL dado que ya se hizo antes
	int entradasAnteriores = entradasQueOcupa(espacio->valor);
	int entradasNuevas = entradasQueOcupa(claveValor.valor);

	if(entradasNuevas > entradasAnteriores){
		// ver si tengo esaCantidadDeLibres
			// si : poner como libres las que usaba antes y guardar en donde toque el nuevo valor
			// no : buscar atomicas para reemplazar
				// si: poner como libres las que usaba antes y guardar en donde toque el nuevo valor
				// no: compactar y tratar de hacer de nuevo.  <-- por ahora NO => ERROR falta de espacio
	}else{
		if(entradasNuevas < entradasAnteriores){
			// reemplazo ahi y libero las que sonbran.
		}else{
			// iguales, remplazo ahi GG
		}
	}
	// tener en cuenta si el nuevo valor ocupa +o- Entradas
}

void guardarClaveValor(t_clavevalor claveValor,int *indice){
	int entradas = entradasQueOcupa(claveValor.valor);
	if(tengoLibres(entradas,indice)){
		int largo = sizeof(char)*(strlen_null(claveValor.clave) + strlen_null(claveValor.valor));
		t_espacio_memoria* nuevoEspacio = malloc(largo + sizeof(int));
		strcpy(nuevoEspacio->clave,claveValor.clave);
		strcpy(nuevoEspacio->valor,claveValor.valor);
		nuevoEspacio->id = id;
		id++;
		list_add(memoria,nuevoEspacio);
		avanzarIndice(indice,entradas);
	}else{
		if(tengoAtomicas(entradas,indice)){
			// si tengo debo actualizar el indice al lugar que debo designar
			// creo t_espacio_memoria y agrego a la lista y GG
		}else{
			//compactar. puedo guardar? guardo:error "no hay espacio" <-- por ahora no.
			notificarCoordinador(1); // ERROR: "no hay espacio"
		}
	}
}

void notificarCoordinador(int respuesta){
	log_trace(logger,"notificador_coordinador(%d)",respuesta);
	enviar(socket_coordinador,RESPUESTA_INTANCIA,sizeof(int),&respuesta);
}

// PARA LISTAS

bool tengoLaClave(char* clave){
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

// inicializar y finalizar

void liberarRecursos(){
	free(indiceMemoria);
	config_destroy(config_aux);
	log_destroy(logger);
	list_destroy(memoria);
}

void finalizar(){
	liberarRecursos();
	exit(EXIT_SUCCESS);
}

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

// AUXILIARES

void incrementarIndice(int *indice){
	if(*indice < cantidad_entradas-1)
		*indice+=1;
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


