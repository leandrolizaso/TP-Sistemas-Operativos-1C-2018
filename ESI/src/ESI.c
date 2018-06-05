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
void ejecutarESI(char* script){

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	t_paquete* paquete;

	ultima_linea = NULL;
	ejecutoUltima = false;
	correr = true;
	explotoParsi = false;

	fp = fopen(script, "r");

	validarAperturaScript(fp);

	if((read = getline(&line, &len, fp)) == -1){
		log_error(logger, "Script vacio.");
		finalizar();
		exit(EXIT_FAILURE);
	}

	t_esi_operacion operacion;

	paquete = recibir(socket_planificador);

	while (codigoBueno(paquete->codigo_operacion) && correr) {

		t_mensaje_esi mensaje;

		switch (paquete->codigo_operacion) {
		case VOLVE:
			ejecutoUltima = true;
			break;

		case EJECUTAR_LINEA:
			if (ejecutoUltima) {
				ejecutoUltima = false;
			} else {

				if((read = getline(&line, &len, fp)) == -1){
					log_info(logger, "Script finalizado.");
					finalizar();
					exit(EXIT_FAILURE);
				}

				ultima_linea = realloc(ultima_linea,len);
				strcpy(ultima_linea,line);
			}

			operacion = parse(line);

			if (operacion.valido) {
				mensaje = extraer_mensaje_esi(operacion, paquete);
				ejecutarMensaje(mensaje, paquete, line);
				destruir_operacion(operacion);
			} else {

				// DEJO ACA DEJO ACA DEJO ACA LALALALAALLA
				char* error;
				error = string_from_format("La línea %s no es válida", line);
				log_error(logger, error);
				free(error);
				finalizar();
				exit(EXIT_FAILURE);
			}
		break;

		default:
		break;
		}
		if(!ejecutoUltima){
			if((read = getline(&line, &len, fp)) == -1){
				log_info(logger, "Se termino el script.");
				finalizar();
				exit(EXIT_FAILURE);
			}
		}
		destruir_paquete(paquete);
		paquete = recibir(socket_planificador);
	}

}

void validarAperturaScript(FILE* fp){
	if (fp == NULL) {
		log_error(logger, "No se pudo abrir el script.");
		finalizar();
		exit(EXIT_FAILURE);
	}
}


bool codigoBueno(int codigo_operacion){
	return codigo_operacion == EJECUTAR_LINEA || codigo_operacion == VOLVE;
}

t_mensaje_esi copiarMensajeEsi(t_mensaje_esi original){
	t_mensaje_esi copia;
	copia.keyword = original.keyword;
	copia.id_esi = original.id_esi;
	copia.clave_valor.clave = string_duplicate(original.clave_valor.clave);
	copia.clave_valor.valor = string_duplicate(original.clave_valor.valor);

	return copia;
}

void ejecutarMensaje(t_mensaje_esi mensaje_esi,t_paquete* paquete,char* line){

	char* msg;
	char error;

	enviar_operacion(mensaje_esi);
	actualizarUltimoMensaje(mensaje_esi);
	liberarClaveValor(mensaje_esi.clave_valor);

	msg = string_from_format("Línea %s fue enviada al Coordinador por el ESI%d",line, ID);
	log_info(logger, msg);
	free(msg);

	destruir_paquete(paquete);
	paquete = recibir(socket_coordinador);

	switch (paquete->codigo_operacion) {

		case EXITO_OPERACION:
			verificarEnvioPlanificador(enviar(socket_planificador, EXITO_OPERACION, 0, NULL), paquete);
			msg = string_from_format("Línea %s ejecutada exitosamente",line);
			log_mensaje(msg);
			free(msg);
			break;
		case ERROR_OPERACION:
			msg = string_from_format("ESI%d abortado. %s",ID,paquete->data);
			log_error(logger, msg);
			free(msg);
			//verificarEnvioPlanificador(enviar(socket_planificador, ERROR_OPERACION, paquete->tamanio,paquete->data), paquete);
			msg = string_from_format("Línea %s falló en su ejecución",line);
			log_mensaje(msg);
			free(msg);
			verificarEnvioPlanificador(enviar(socket_planificador, ESI_FINALIZADO, 0,NULL), paquete);
			destruir_paquete(paquete);
			finalizar();
			exit(EXIT_FAILURE);
			break;
		default:
			error = string_from_format("El codigo de operación %d no es válido",paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			finalizar();
			exit(EXIT_FAILURE);
		}

}

void log_mensaje(char* mensaje) {
	log_info(logger, mensaje);
}


void ejecutar(char* script) {

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	t_paquete* paquete;
	char* error;
	char* msg;
	ultimo_mensaje.clave_valor.clave = NULL;

	fp = fopen(script, "r");
	if (fp == NULL) {
		log_error(logger, "No se pudo abrir el script");
		finalizar();
		exit(EXIT_FAILURE);
	}

	paquete = recibir(socket_planificador);

	while ((read = getline(&line, &len, fp)) != -1
			&& (paquete->codigo_operacion == EJECUTAR_LINEA || paquete->codigo_operacion == VOLVE)) {

		t_mensaje_esi mensaje_esi;

		if(paquete->codigo_operacion == VOLVE){
			mensaje_esi.keyword = ultimo_mensaje.keyword;
			mensaje_esi.id_esi = ultimo_mensaje.id_esi;
			mensaje_esi.clave_valor.clave = string_duplicate(ultimo_mensaje.clave_valor.clave);
			mensaje_esi.clave_valor.valor = string_duplicate(ultimo_mensaje.clave_valor.valor);

			esperoEjecutarDePlanif:

			destruir_paquete(paquete);
			paquete = recibir(socket_planificador);

			switch(paquete->codigo_operacion){
				case VOLVE:
					goto esperoEjecutarDePlanif;
					break; // ni llega aca me parece xd
				case EJECUTAR_LINEA:
					goto envioUltimo;
					break; // ni llega aca me parece * 2 xd
				default:
					goto verificarMuerte;
					break; // ni llega aca me parece * 3 xd
			}
		}
		destruir_paquete(paquete);

		t_esi_operacion operacion = parse(line);

		if (operacion.valido) {

			mensaje_esi.clave_valor = extraerClaveValor(operacion,paquete);;
			mensaje_esi.id_esi = ID;
			mensaje_esi.keyword = operacion.keyword;

			envioUltimo:
			enviar_operacion(mensaje_esi);
			actualizarUltimoMensaje(mensaje_esi);
			liberarClaveValor(mensaje_esi.clave_valor);

			msg = string_from_format("Línea %s fue enviada al Coordinador por el ESI%d", line,ID);
			log_info(logger, msg);
			free(msg);

			destruir_operacion(operacion);

			paquete = recibir(socket_coordinador);

			switch (paquete->codigo_operacion) {

			void loggear(char* mensaje){
				msg = string_from_format(mensaje,line);
				log_info(logger, msg);
				free(msg);
			}

			case EXITO_OPERACION:
				verificarEnvioPlanificador(enviar(socket_planificador, EXITO_OPERACION, 0, NULL),paquete);
				loggear("Línea %s ejecutada exitosamente");
				break;
			case ERROR_OPERACION:
				log_error(logger,paquete->data);
				verificarEnvioPlanificador(enviar(socket_planificador, ERROR_OPERACION, paquete->tamanio, paquete->data),paquete);
				loggear("Línea %s falló en su ejecución");
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

	verificarMuerte:
	switch(paquete->codigo_operacion ){
	case FINALIZAR:
		msg = string_from_format("ESI%d: Me mataron por consola.",ID);
		log_info(logger,msg);
		destruir_paquete(paquete);
		free(msg);
		finalizar();
		break;
	case ABORTAR:
		msg = string_from_format("ESI%d: Me abortaron.",ID);
		log_info(logger,msg);
		destruir_paquete(paquete);
		free(msg);
		finalizar();
		break;
	default:
		esiFinalizado(read,paquete);
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

void esiFinalizado(ssize_t read, t_paquete* paquete){
	if(read == -1){
		int envio = enviar(socket_planificador, ESI_FINALIZADO,0, NULL);
		verificarEnvioPlanificador(envio,paquete);
	}
}

void verificarEnvioPlanificador(int envio,t_paquete* paquete){
	if (envio < 0) {
		perror("Error de comunicación con el Planificador");
		log_error(logger, "Error de comunicación con el Planificador");
		destruir_paquete(paquete);
		finalizar();
		exit(EXIT_FAILURE);
	}
}

void verificarEnvioCoordinador(int envio){
	if (envio < 0) {
		perror("Error de comunicación con el Coordinador");
		log_error(logger,"Error de comunicación con el Coordinador");
		finalizar();
		exit(EXIT_FAILURE);
	}
}

void enviar_operacion(t_mensaje_esi mensaje_esi){
	void* buffer = serializar_mensaje_esi(mensaje_esi);
	verificarEnvioCoordinador(enviar(socket_coordinador, OPERACION,sizeof_mensaje_esi(mensaje_esi), buffer));
	free(buffer);
}

t_mensaje_esi extraer_mensaje_esi(t_esi_operacion operacion,t_paquete* paquete){
	t_mensaje_esi mensaje;
	mensaje.clave_valor = extraerClaveValor(operacion,paquete);
	mensaje.id_esi = ID;
	mensaje.keyword = operacion.keyword;
	return mensaje;
}

t_clavevalor extraerClaveValor(t_esi_operacion operacion,t_paquete* paquete){
	t_clavevalor clavevalor;
	char* error;
	switch (operacion.keyword) {
	case GET:
		clavevalor.clave = string_duplicate(operacion.argumentos.GET.clave);
		clavevalor.valor = NULL;
		return clavevalor;
		break;
	case SET:
		clavevalor.clave = string_duplicate(operacion.argumentos.SET.clave);
		clavevalor.valor = string_duplicate(operacion.argumentos.SET.valor);
		return clavevalor;
		break;
	case STORE:
		clavevalor.clave = string_duplicate(operacion.argumentos.STORE.clave);
		clavevalor.valor = NULL;
		return clavevalor;
		break;
	default:
		error = string_from_format("La keyword %d del esi %d no es válida",operacion.keyword, ID);
		log_error(logger, error);
		free(error);
		destruir_paquete(paquete);
		finalizar();
		exit(EXIT_FAILURE);
	}
}

void actualizarUltimoMensaje(t_mensaje_esi mensajeEnviado){
	liberarUltimaClaveValor();
	ultimo_mensaje = copiarMensajeEsi(mensajeEnviado);
}

void liberarUltimaClaveValor(){
	if(ultimo_mensaje.clave_valor.clave != NULL)
		liberarClaveValor(ultimo_mensaje.clave_valor);
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
