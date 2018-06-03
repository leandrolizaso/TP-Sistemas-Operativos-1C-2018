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

	if (argc < 3) {
		perror("Faltan argumentos para una correcta ejecución");
		exit(EXIT_FAILURE);
	}

	inicializar(argv[1]);

	ejecutar(argv[2]);

	finalizar();

	return EXIT_SUCCESS;
}

void inicializar(char* path) {

	crearLog();
	levantarConfig(path);
	conectarPlanificador();
	conectarCoordinador();

}

void ejecutar(char* script) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	t_paquete* paquete;
	char* error;
	char* msg;

	fp = fopen(script, "r");
	if (fp == NULL) {
		log_error(logger, "No se pudo abrir el script");
		finalizar();
		exit(EXIT_FAILURE);
	}

	paquete = recibir(socket_planificador);

	while ((read = getline(&line, &len, fp)) != -1 && paquete->codigo_operacion == EJECUTAR_LINEA) {

		destruir_paquete(paquete);

		t_esi_operacion sentencia = parse(line);

		if (sentencia.valido) {

			t_clavevalor extraerClaveValor(t_esi_operacion sentencia){
				t_clavevalor clavevalor;
				switch (sentencia.keyword) {
				case GET:
					clavevalor.clave = string_duplicate(sentencia.argumentos.GET.clave);
					clavevalor.valor = NULL;
					return clavevalor;
					break;
				case SET:
					clavevalor.clave = string_duplicate(sentencia.argumentos.SET.clave);
					clavevalor.valor = string_duplicate(sentencia.argumentos.SET.valor);
					return clavevalor;
					break;
				case STORE:
					clavevalor.clave = string_duplicate(sentencia.argumentos.STORE.clave);
					clavevalor.valor = NULL;
					return clavevalor;
					break;
				default:
					error = string_from_format("La keyword %d del esi %d no es válida",sentencia.keyword, ID);
					log_error(logger, error);
					free(error);
					destruir_paquete(paquete);
					finalizar();
					exit(EXIT_FAILURE);
				}
			}

			t_clavevalor claveValor = extraerClaveValor(sentencia);

			t_mensaje_esi mensaje_esi;

			mensaje_esi.clave_valor = claveValor;
			mensaje_esi.id_esi = ID;
			mensaje_esi.keyword = sentencia.keyword;

			enviar_operacion(mensaje_esi);
			liberarClaveValor(claveValor);

			msg = string_from_format("Línea %s fue enviada al Coordinador", line);
			log_info(logger, msg);
			free(msg);

			destruir_operacion(sentencia);

			paquete = recibir(socket_coordinador);

			switch (paquete->codigo_operacion) {

			case EXITO_OPERACION:
				if (enviar(socket_planificador, EXITO_OPERACION, 0, NULL) < 0) {
					perror("Error de comunicación con el Planificador");
					log_error(logger,"Error de comunicación con el Planificador");
					destruir_paquete(paquete);
					finalizar();
					exit(EXIT_FAILURE);
				}
				;
				msg = string_from_format("Línea %s ejecutada exitosamente",line);
				log_info(logger, msg);
				free(msg);
				break;
			case ERROR_OPERACION:
				log_error(logger,paquete->data);
				if (enviar(socket_planificador, ERROR_OPERACION, 0, NULL) < 0) {
					perror("Error de comunicación con el Planificador");
					log_error(logger,"Error de comunicación con el Planificador");
					destruir_paquete(paquete);
					finalizar();
					exit(EXIT_FAILURE);
				};
				msg = string_from_format("Línea %s falló en su ejecución",line);
				log_info(logger, msg);
				free(msg);
				break;

			case VOLVE:
				break;
			default:
				error = string_from_format("El codigo de operación %d no es válido",paquete->codigo_operacion);
				log_error(logger, error);
				free(error);
				destruir_paquete(paquete);
				finalizar();
				exit(EXIT_FAILURE);
			}

			destruir_paquete(paquete);

		} else {
			error = string_from_format("La línea %s no es válida", line);
			log_error(logger, error);
			free(error);
			finalizar();
			exit(EXIT_FAILURE);
		}

		paquete = recibir(socket_planificador);
	}

	switch(paquete->codigo_operacion){

		case FINALIZAR:
			destruir_paquete(paquete);
			msg= string_from_format("El ESI %d fue finalizado por consola.",ID);
			log_info(logger,msg);
			free(msg);
			finalizar();
			break;
		case ABORTAR:
			log_info(logger,paquete->data);
			destruir_paquete(paquete);
			free(msg);
			finalizar();
			break;
		default:
			error = string_from_format("El codigo de operación %d no es válido",paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			destruir_paquete(paquete);
			finalizar();
			exit(EXIT_FAILURE);
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

void enviar_operacion(t_mensaje_esi mensaje_esi){
	int envio = enviar(socket_coordinador, OPERACION,
			sizeof_mensaje_esi(mensaje_esi),serializar_mensaje_esi(mensaje_esi));

	verificarEnvioCoordinador(envio);
}

void verificarEnvioCoordinador(int envio){
	if (envio < 0) {
		perror("Error de comunicación con el Coordinador");
		log_error(logger,"Error de comunicación con el Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}
}

void liberarClaveValor(t_clavevalor claveValor){
	free(claveValor.clave);
	if(claveValor.valor != NULL)
		free(claveValor.valor);
}



// Encapsulamiento
void crearLog() {
	logger = log_create("esi.log", "ESI", false, LOG_LEVEL_TRACE);
}
void levantarConfig(char* path) {

	config_aux = config_create(path);

	if (config_has_property(config_aux, "IP_COORDINADOR")) {
		config.ip_coordinador = config_get_string_value(config_aux,"IP_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config_aux, "PUERTO_COORDINADOR")) {
		config.puerto_coordinador = config_get_string_value(config_aux,"PUERTO_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config_aux, "IP_PLANIFICADOR")) {
		config.ip_planificador = config_get_string_value(config_aux,"IP_PLANIFICADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Planificador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	if (config_has_property(config_aux, "PUERTO_PLANIFICADOR")) {
		config.puerto_planificador = config_get_string_value(config_aux,"PUERTO_PLANIFICADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Planificador");
		finalizar();
		exit(EXIT_FAILURE);
	}

	log_info(logger, "Se cargó exitosamente la configuración");
}
void conectarPlanificador() {
	socket_planificador = conectar_a_server(config.ip_planificador,config.puerto_planificador);
	enviar(socket_planificador, HANDSHAKE_ESI, 0, NULL);
	t_paquete* paquete;
	paquete = recibir(socket_planificador);

	if (paquete->codigo_operacion != HANDSHAKE_PLANIFICADOR) {
		log_error(logger, "Falló el handshake con el Planificador");
		destruir_paquete(paquete);
		finalizar();
		exit(EXIT_FAILURE);
	} else {
		ID = (int) paquete->data;
		log_info(logger, "Handshake exitoso con el Planificador");
		destruir_paquete(paquete);
	}
}
void conectarCoordinador() {
	socket_coordinador = conectar_a_server(config.ip_coordinador,config.puerto_coordinador);
	enviar(socket_coordinador, HANDSHAKE_ESI, sizeof(int), (void*) ID);
	t_paquete* paquete;
	paquete = recibir(socket_coordinador);
	if (paquete->codigo_operacion != HANDSHAKE_COORDINADOR) {
		log_error(logger, "Falló el handshake con el Coordinador");
		destruir_paquete(paquete);
		finalizar();
		exit(EXIT_FAILURE);
	} else {
		log_info(logger, "Handshake exitoso con el Coordinador");
		destruir_paquete(paquete);
	}
}
