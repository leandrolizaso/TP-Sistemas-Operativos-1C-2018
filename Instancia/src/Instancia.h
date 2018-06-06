#ifndef INSTANCIA_H_INCLUDED
#define INSTANCIA_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>
#include "Instancia.h"

#define CFG_IP  "ip_coordinador"
#define CFG_PORT  "port_coordinador"
#define CFG_ALGO  "distribution_replacement"
#define CFG_POINT  "point_mount"
#define CFG_NAME_INST  "name_instancia"
#define CFG_INTERVAL  "interval"

//Leer arcivo de configuracion
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

	int socket_coordinador;

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


typedef struct entrada{
	int t_clave;
	char* clave;
	int t_val;
	char* valor;
	int numero_entrada;
}entrada;
entrada* crearEntrada (char* clave, int t_clave, char* valor, int t_val, int numero_entrada){
	entrada* bloque;
	bloque = malloc(sizeof(entrada));
	bloque->clave = malloc(strlen(clave) + 1);
	//strcpy(bloque->clave, bloque);
	bloque->numero_entrada = numero_entrada;
	bloque->t_clave = t_clave;
	bloque->valor = malloc (strlen(valor) + 1);
	bloque->t_val = valor;

	return bloque;
};
int destruir_entrada (entrada* bloque){
	free (bloque->clave);
	free (bloque->valor);
	free (bloque);
	return 0;
};
#endif // INSTANCIA_H_INCLUDED
