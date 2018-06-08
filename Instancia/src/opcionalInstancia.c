#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "opcionalInstancia.h"

#define CFG_IP  "ip_coordinador"
#define CFG_PORT  "port_coordinador"
#define CFG_ALGO  "distribution_replacement"
#define CFG_POINT  "point_mount"
#define CFG_NAME_INST  "name_instancia"
#define CFG_INTERVAL  "interval"

int tamanio_entrada;
int cantidad_entradas;
void* memoria ;

int algoritmo;

int socket_coordinador;
int indiceTabla;

t_config_instancia config;

int main(int argc, char* argv[])  {

	t_log* logger;

	puts("Iniciando instancia");

	logger = log_create("instancia.log","INSTANCIA",false,LOG_LEVEL_TRACE);
	t_config* config = leer_config(argc, argv);
	if (config_incorrecta(config)) {
		config_destroy(config);
		return EXIT_FAILURE;
	}

	inicializar(config,logger);
	ejecutar(config,logger);
	finalizar(config,logger);

	return EXIT_SUCCESS;
}

void ejecutar(t_config* config, t_log* logger){

	memoria = crearMemoria();
	inicializarTabla();

	char* error;
	t_paquete* paquete;
	paquete=recibir(socket_coordinador);

	switch (paquete->codigo_operacion) {

	case SAVE_CLAVE:{
		t_clavevalor claveValor = deserializar_clavevalor(paquete->data);
		set(claveValor);
		break;
		}
	case 5:
		break;

	default:
		error = string_from_format("El codigo de operación %d no es válido",paquete->codigo_operacion);
		log_error(logger, error);
		free(error);
	}

}

void set(t_clavevalor claveValor){

	switch(algoritmo){
	case 0:
		break;
	}

}

void inicializarTabla(){}

void* crearMemoria(){
	// manejo de lista
	return malloc(tamanio_entrada*cantidad_entradas);
}


bool hayEntradasDisponible(int largo){
	return true;
}

void reemplazarAtomico(char* valor){}

int entradasQueOcupa(int largoValor){

	int cantEntradas = largoValor/tamanio_entrada;
	if( cantEntradas <= 1)
		return 1;
	else{
		return cantEntradas +1;
	}
}



// anterior


t_config* leer_config(int argc, char* argv[]) {
	int opcion;
	t_config* config = NULL;
	opterr = 1; //ver getopt()

	while ((opcion = getopt(argc, argv, "c:")) != -1) {
		switch (opcion) {
		case 'c':
			printf("Levantando config... %s\n", optarg);
			config = config_create(optarg);
			break;
		case ':':
			fprintf(stderr, "El parametro '-%c' requiere un argumento.\n",
					optopt);
			break;
		case '?':
		default:
			fprintf(stderr, "El parametro '-%c' es invalido. Ignorado.\n",
					optopt);
			break;
		}
	}

	return config;
};
int config_incorrecta(t_config* config) {
	if (config == NULL) {
		// PARA CORRER DESDE ECLIPSE
		// AGREGAR EN "Run Configurations.. > Arguments"
		// -c ${workspace_loc:/Instancia/src/inst.cfg}
		puts("El parametro -c <config_file> es obligatorio.\n");
		return EXIT_FAILURE;
	}

	int failures = 0;
		void validar(char* key) { //TODO validar tipos?
			if (!config_has_property(config, key)) {
				printf("Se requiere configurar \"%s\"\n", key);
				failures++;
			}
		}

	validar(CFG_IP);
	validar(CFG_PORT);
	validar(CFG_ALGO);
	validar(CFG_POINT);
	validar(CFG_NAME_INST);
	validar(CFG_INTERVAL);


	if (failures > 0) {
		printf("Por favor revisar el archivo \"%s\"\n", config->path);
		return EXIT_FAILURE;
	}else{
		puts("Validacion correcta.\n");
	}
	return EXIT_SUCCESS;
};
void finalizar( t_config* config,t_log* logger){
	log_info(logger, "Fin ejecución");
	config_destroy(config);
	log_destroy(logger);
};
void inicializar(t_config* config,t_log* logger){

	if(!config_has_property(config, CFG_IP)){
	log_error(logger, "No se encuentra la ip del Coordinador");
				finalizar( config,logger);
				exit(EXIT_FAILURE);
	}

	if(!config_has_property(config, CFG_PORT)){
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar(config,logger);
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Se cargó exitosamente la configuración");

	// Me conecto al Coordinador
	socket_coordinador = conectar_a_server(config_get_string_value(config, CFG_IP), config_get_string_value(config, CFG_PORT));
	log_info(logger, "Conexión exitosa al Coordinador");

	//ENVIAR MENSAJE
	enviar(socket_coordinador, HANDSHAKE_INSTANCIA, 0, NULL);

	t_paquete* paquete = recibir(socket_coordinador);
	if(paquete->codigo_operacion == HANDSHAKE_COORDINADOR){
		log_info(logger, "Mensaje recibido de coordinador");
	}else{
		log_info(logger, "Error al recibir mensaje de corrdinador");
	}
	destruir_paquete(paquete);
};

