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
	crearMemoria();
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
//
//	char* aux= string_new();
//	string_append(&aux,config.nombre);
//	string_append(&aux,".log");
	logger = log_create("instancia.log", config.nombre, true, LOG_LEVEL_TRACE);
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
		cantidad_entradas = (int)paquete->data;
		tamanio_entradas= (int)(paquete->data+sizeof(int));
		log_info(logger,"Handshake exitoso con el Coordinador");
		destruir_paquete(paquete);
	}
}

void atenderConexiones(){
	log_trace(logger,"atenderConexiones()");
	t_paquete* paquete;
	paquete = recibir(socket_coordinador);
	char* error;
	int imRunning = 1;

	while(imRunning){
		log_trace(logger,"atenderConexiones() #imRunning");
		switch(paquete->codigo_operacion){
		case SAVE_CLAVE:{
			log_trace(logger,"SAVE_CLAVE");
			t_clavevalor claveValor = deserializar_clavevalor(paquete->data);
			if (tengoLaClave(claveValor.clave)) {
				guardarPisandoClaveValor(claveValor);
			} else {
				guardarClaveValor(claveValor);
			}
			notificar_coordinador(0);
			// en cada guardar deberia tener un notificar y depende del error/exito notificar
			// ahora notifico cero, para que todinho salga bien
		break;}
		case DUMP_CLAVE:{
			log_trace(logger,"DUMP_CLAVE");
			char* clave = malloc(paquete->tamanio*sizeof(char));
			clave = strdup(paquete->data);
			t_espacio_memoria* memory = conseguirEspacioMemoria(clave);
			if(memory == NULL){
				//notificar_coordinador(3); // <-- 3 = ERROR: se quiere hacer STORE de una clave que no se posee.
			}else{
				// mmap para guardar el valor <<--- ia bere khe ago
				//t_indice* indice = list_get(tablaIndices, memory->id);
				//indice->idOcupante = -1;
			}
			free(clave);
			notificar_coordinador(0);
			//para que de bien
		break;}
		default:{
			error = string_from_format("El codigo de operación %d no es válido", paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			imRunning = 0;
			}
		}

	destruir_paquete(paquete);
	paquete = recibir(socket_coordinador);
	}

}


void notificar_coordinador(int respuesta){
	log_trace(logger,"notificador_coordinador(%d)",respuesta);
	enviar(socket_coordinador,RESPUESTA_INTANCIA,sizeof(int),&respuesta);
}

// FORMAS DE GUARDAR

//verificar que tengas entradas atomicas libres contiguas
// 		si:guardar
//		no: lo anterior pero considero a las atomicas ocupadas como libres
//			 si: guardo
//			 no: compacto <-- por ahora no. Entonces ahora es mandar un error por falta de espacio

void guardarPisandoClaveValor(t_clavevalor claveValor){
	// tener en cuenta si el nuevo valor ocupa +o- Entradas
}

void guardarClaveValor(t_clavevalor claveValor){
}

// Creacion y Destruccion

void finalizar(){
	liberarRecursos();
	exit(EXIT_SUCCESS);
}

void liberarRecursos(){
	config_destroy(config_aux);
	log_destroy(logger);
	destruirMemoria();
}

void crearTablaIndices(){
	log_trace(logger,"crearTablaIndices()");
}

void crearMemoria(){
	memoria = list_create();
}

void destruirMemoria(){
	list_destroy(memoria);
}

void destruirTablaIndices(){
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


// AUXILIARES
bool esAtomico(t_indice* indice){
	return entradasQueOcupa(indice->espacio->valor)<= 1;
}

int entradasQueOcupa(char* valor){
	float cantidadEntradas = (strlen_null(valor) -1)/tamanio_entradas;
	if(cantidadEntradas < 1)
		return 1;
	else
		return (int)cantidadEntradas +1;
}
