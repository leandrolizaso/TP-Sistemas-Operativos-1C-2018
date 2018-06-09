#include "pruebaCristian.h"

int main(int argc, char** argv){
	inicializar();
	atenderConexiones();
	liberarRecursos();
	return EXIT_SUCCESS;
}

void inicializar(){
	crearLog();
	levantarConfig();
	conectarCoordinador();
	crearTablaIndices();
	crearMemoria();
}

void levantarConfig(){
	t_config* config_aux = config_create("inst.cfg");

	// Tener en cuenta config_has_property(config_aux,"key"); para validar si esta todo...

	config.algoritmo = config_get_string_value(config_aux,"distribution_replacement");
	config.intervalo= config_get_double_value(config_aux,"interval");
	config.ip_coordinador = config_get_string_value(config_aux,"ip_coordinador");
	config.nombre = config_get_string_value(config_aux,"name_instancia");
	config.point_mount = config_get_string_value(config_aux,"point_mount");
	config.puerto_coordinador = config_get_int_value(config_aux,"port_coordinador");

	config_destroy(config_aux);

}

void crearLog() {
	logger = log_create("esi.log", "ESI", true, LOG_LEVEL_TRACE);
}

void conectarCoordinador() {

	socket_coordinador = conectar_a_server(config.ip_coordinador,config.puerto_coordinador);

	enviar(socket_coordinador, HANDSHAKE_INSTANCIA, strlen_null(config.nombre), config.nombre);
	t_paquete* paquete;
	paquete = recibir(socket_coordinador);
	if (paquete->codigo_operacion != HANDSHAKE_COORDINADOR) {
		log_error(logger, "Falló el handshake con el Coordinador");
		destruir_paquete(paquete);
		morir();
	} else {
		cantidad_entradas = (int)paquete->data;
		tamanio_entrada = (int)(paquete->data+sizeof(int));
		log_info(logger,"Handshake exitoso con el Coordinador");
		destruir_paquete(paquete);
	}
}

void atenderConexiones(){
	t_paquete* paquete;
	paquete = recibir(socket_coordinador);
	char* error;
	int imRunning = 1;
	while(imRunning){

		switch(paquete->codigo_operacion){
		case SAVE_CLAVE:
			t_clavevalor claveValor = deserializar_clavevalor(paquete->data);
			if (tengoLaClave(claveValor->valor)) {
				guardarPisandoClaveValor(claveValor);
			} else {
				guardarClaveValor(claveValor);
			}
			notificar_coordinador(0);
		break;

		//DUMP_CLAVE    7777: nana nana nana nana
		case 7777:
			char* clave = malloc(sizeof(char)*paquete->tamanio);
			strcpy(clave,paquete->data);
			t_espacio_memoria* memory = tengoLaClave(claveValor->valor);

			// mmap para guardar el valor <<--- ia bere khe ago

			t_indice* indice = list_get(tablaIndices, memory->id);
			indice->idOcupante = -1;

		break;

		default:
			error = string_from_format("El codigo de operación %d no es válido", paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			destruir_paquete(paquete);
		}


	}

}


void notificar_coordinador(int respuesta){
	enviar(socket_coordinador,RESPUESTA_INTANCIA,sizeof(int),respuesta);
}

// FORMAS DE GUARDAR

//verificar que tengas entradas atomicas libres contiguas
// 		si:guardar
//		no: lo anterior pero considero a las atomicas ocupadas como libres
//			 si: guardo
//			 no: compacto <-- por ahora no. Entonces ahora es mandar un error por falta de espacio

void guardarPisandoClaveValor(claveValor){
	// tener en cuenta si el nuevo valor ocupa +o- Entradas
}

void guardarClaveValor(claveValor){
}


// Creacion y Destruccion

void crearTablaIndices(){
	tablaIndices = list_create();
	int i = 0;
	while(i<cantidad_entradas){
		t_indice* indice;
		indice->espacio = NULL;
		indice->idOcupante = -1;
		list_add(tablaIndices,indice);
	}
}

void crearMemoria(){
	memoria = list_create();
}

void destruirMemoria(){
	list_destroy(memoria);
}

void desruirTablaIndices(){
	list_destroy(tablaIndices);
}



// PARA LISTAS

t_espacio_memoria* tengoLaClave(char* clave){
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
