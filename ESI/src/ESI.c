/*
 ============================================================================
 Name        : ESI.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "ESI.h"

int main(int argc, char* argv[]) {

	puts(" ********** Proceso ESI **********");

	if(argc < 3){
		perror("Faltan argumentos para una correcta ejecución");
		exit(EXIT_FAILURE);
	}

	inicializar(argv[1]);

	ejecutar(argv[2]);

	finalizar();

	return EXIT_SUCCESS;
}

void inicializar(char* path) {

	// Creo log

	logger = log_create("esi.log", "ESI", false, LOG_LEVEL_TRACE);

	// Cargo la configuración

	config_aux = config_create(path);

	if (config_has_property(config_aux, "IP_COORDINADOR")) {
		config.ip_coordinador = config_get_string_value(config_aux, "IP_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config_aux, "PUERTO_COORDINADOR")) {
		config.puerto_coordinador = config_get_string_value(config_aux, "PUERTO_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config_aux, "IP_PLANIFICADOR")) {
		config.ip_planificador = config_get_string_value(config_aux, "IP_PLANIFICADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Planificador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config_aux, "PUERTO_PLANIFICADOR")) {
		config.puerto_planificador = config_get_string_value(config_aux, "PUERTO_PLANIFICADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Planificador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Se cargó exitosamente la configuración");

	// Me conecto al Coordinador y Planificador

	socket_coordinador = conectar_a_server(config.ip_coordinador, config.puerto_coordinador);

	enviar(socket_coordinador, HANDSHAKE_ESI, 0, NULL);

	t_paquete* rs;

	rs = recibir(socket_coordinador);

	if (rs->codigo_operacion != HANDSHAKE_COORDINADOR) {
		log_error(logger, "Falló el handshake con el Coordinador");
		destruir_paquete(rs);
		finalizar();
		exit(EXIT_FAILURE);
	} else {
		log_info(logger, "Handshake exitoso con el Coordinador");
		destruir_paquete(rs);
	}

	socket_planificador = conectar_a_server(config.ip_planificador, config.puerto_planificador);

	enviar(socket_planificador, HANDSHAKE_ESI, 0, NULL);

	rs = recibir(socket_planificador);

	if (rs->codigo_operacion != HANDSHAKE_PLANIFICADOR) {
		log_error(logger, "Falló el handshake con el Planificador");
		destruir_paquete(rs);
		finalizar();
		exit(EXIT_FAILURE);
	} else {
		log_info(logger, "Handshake exitoso con el Planificador");
		destruir_paquete(rs);
	}

}

void ejecutar(char* script){

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    t_paquete* paquete;

    fp = fopen(script, "r");
    if (fp == NULL){
    	log_error(logger, "No se pudo abrir el script");
        finalizar();
        exit(EXIT_FAILURE);
    }

    paquete = recibir(socket_planificador);

    while ((read = getline(&line, &len, fp)) != -1 && paquete->codigo_operacion == EJECUTAR_LINEA) {

    	destruir_paquete(paquete);

        t_esi_operacion sentencia = parse(line);

        if(sentencia.valido){
            switch(sentencia.keyword){
                case GET:
                    if(enviar_get(sentencia) < 0){
                    	perror("Error de comunicacion con el Coordinador");
                    	log_error("Error de comunicacion con el Coordinador");
                        finalizar();
                        exit(EXIT_FAILURE);
                    };
                    break;
                case SET:
                    if(enviar_set(sentencia) < 0){
                    	perror("Error de comunicacion con el Coordinador");
                    	log_error("Error de comunicacion con el Coordinador");
                        finalizar();
                        exit(EXIT_FAILURE);
                    };
                    break;
                case STORE:
                    if(enviar_store(sentencia) < 0){
                    	perror("Error de comunicacion con el Coordinador");
                    	log_error("Error de comunicacion con el Coordinador");
                        finalizar();
                        exit(EXIT_FAILURE);
                    };
                    break;
                default:
                	char* error = string_from_format("La linea %s no se pudo interpretar", line);
                    log_error(logger, error);
                    free(error);
                    finalizar();
                    exit(EXIT_FAILURE);
            }

            destruir_operacion(sentencia);

            paquete = recibir(socket_coordinador);

            switch(paquete->codigo_operacion){
                case EXITO_OPERACION:
                    if(enviar(socket_planificador, EXITO_OPERACION, 0, NULL) < 0){
                    	perror("Error de comunicacion con el Planificador");
                    	log_error("Error de comunicacion con el Planificador");
                    	destruir_paquete(paquete);
                        finalizar();
                        exit(EXIT_FAILURE);
                    };
                    char* msg = string_from_format("Linea %s ejecutada exitosamente", line);
                    log_info(logger, msg);
                    free(msg);
                    break;
                case ERROR_OPERACION:
                	if(enviar(socket_planificador, ERROR_OPERACION, 0, NULL) < 0){
                		perror("Error de comunicacion con el Planificador");
                    	log_error("Error de comunicacion con el Planificador");
                    	destruir_paquete(paquete);
                        finalizar();
                        exit(EXIT_FAILURE);
                	};
                    char* msg = string_from_format("Linea %s fallo en su ejecucion", line);
                    log_info(logger, msg);
                    free(msg);
                	break;
                default:
                	char* error = string_from_format("El codigo de operacion %d no es valido", paquete->codigo_operacion);
                    log_error(logger, error);
                    free(error);
                    destruir_paquete(paquete);
                    finalizar();
                    exit(EXIT_FAILURE);
            }

            destruir_paquete(paquete);

        } else {
        	char* error = string_from_format("La linea %s no es válida", line);
            log_error(logger, error);
            free(error);
            finalizar();
            exit(EXIT_FAILURE);
        }

        paquete = recibir(socket_planificador);
    }

    fclose(fp);

    if (line)
        free(line);
}

void finalizar() {

	log_info(logger, "Fin ejecución");
	config_destroy(config_aux);
	log_destroy(logger);
}

int enviar_get(t_esi_operacion sentencia){

	char* clave = string_duplicate(sentencia.argumentos.GET.clave);
	int len = string_length(clave);

	int rs = enviar(socket_coordinador, OPERACION_GET, len, (void*)clave);

	free(clave);

	return rs;
}

int enviar_set(t_esi_operacion sentencia){

	char* clave = string_duplicate(sentencia.argumentos.SET.clave);
	char* valor = string_duplicate(sentencia.argumentos.SET.valor);
	int len_clave = string_length(clave);
	int len_valor = string_length(valor);
	int len_buffer = 2 * sizeof(int) + len_clave + len_valor;
	void* buffer = malloc(len_buffer);

	memcpy(buffer, &len_clave, sizeof(int));
	memcpy(buffer + sizeof(int), clave, len_clave);
	memcpy(buffer + sizeof(int) + len_clave, &len_valor, sizeof(int));
	memcpy(buffer + 2 * sizeof(int) + len_clave, valor, len_valor);

	int rs = enviar(socket_coordinador, OPERACION_SET, len_buffer, buffer);

	free(clave);
	free(valor);
	free(buffer);

	return rs;
}

int enviar_store(t_esi_operacion sentencia){

	char* clave = string_duplicate(sentencia.argumentos.STORE.clave);
	int len = string_length(clave);

	int rs = enviar(socket_coordinador, OPERACION_STORE, len, (void*)clave);

	free(clave);

	return rs;
}
