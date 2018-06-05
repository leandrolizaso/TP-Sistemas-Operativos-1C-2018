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

void ejecutar(char* script){

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	t_paquete* paquete;

	char* ultima_linea = NULL;
	bool ejecutoUltima = false;

	fp = fopen(script, "r");

	validarAperturaScript(fp);

	if((read = getline(&line, &len, fp)) == -1){
		log_error(logger, "Script vacio.");
		morir();
	}

	t_esi_operacion operacion;

	paquete = recibir(socket_planificador);

	while (codigoBueno(paquete->codigo_operacion)) {

		t_mensaje_esi mensaje;

		switch (paquete->codigo_operacion) {
		case VOLVE:
			ejecutoUltima = true;
			destruir_paquete(paquete);
			break;

		case EJECUTAR_LINEA:
			if (ejecutoUltima) {
				ejecutoUltima = false;
			} else
			{
				if((read = getline(&line, &len, fp)) == -1){
					log_mensaje("Script finalizado.");
					verificarEnvioPlanificador(enviar(socket_planificador, ESI_FINALIZADO, 0,NULL), paquete);
					destruir_paquete(paquete);
					morir();
				}

				ultima_linea = realloc(ultima_linea,len);
				strcpy(ultima_linea,line);
			}

			operacion = parse(ultima_linea);

			if (operacion.valido) {
				mensaje = extraer_mensaje_esi(operacion, paquete);
				destruir_operacion(operacion);
				ejecutarMensaje(mensaje, paquete, line);
			} else {
				char* error;
				error = string_from_format("La línea: %s .No es válida", line);
				log_error(logger, error);// al loguear la linea uno ve el porque de invalidez :3
				free(error);
				destruir_operacion(operacion);
				destruir_paquete(paquete);
				morir();
			}
			break;

		}

		paquete = recibir(socket_planificador);
	}

	fclose(fp);

	if (line)
		free(line);

	if(paquete->codigo_operacion == FINALIZAR){
		char* msg = string_from_format("ESI%d fue finalizado por consola.",ID);
		log_mensaje(msg);
		free(msg);
	}else{
		char* error = string_from_format("Codigo de operacion %d inválido.",paquete->codigo_operacion);
		log_error(logger,error);
		free(error);
	}
}

void finalizar() {
	log_info(logger, "Fin ejecución");
	config_destroy(config_aux);
	log_destroy(logger);
}

void validarAperturaScript(FILE* fp){
	if (fp == NULL) {
		log_error(logger, "No se pudo abrir el script.");
		morir();
	}
}

bool codigoBueno(int codigo_operacion){
	return codigo_operacion == EJECUTAR_LINEA || codigo_operacion == VOLVE;
}

void ejecutarMensaje(t_mensaje_esi mensaje_esi,t_paquete* paquete,char* line){

	char* msg;
	char* error;

	enviar_operacion(mensaje_esi);

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
			destruir_paquete(paquete);
			break;
		case ERROR_OPERACION:
			msg = string_from_format("ESI%d abortado. %s",ID,paquete->data);
			log_mensaje(msg);
			free(msg);
			msg = string_from_format("Línea: %s .Falló en su ejecución",line);
			log_mensaje(msg);
			free(msg);
			verificarEnvioPlanificador(enviar(socket_planificador, ESI_FINALIZADO, 0,NULL), paquete);
			destruir_paquete(paquete);
			morir();
			break;
		default:
			error = string_from_format("El codigo de operación %d no es válido",paquete->codigo_operacion);
			log_error(logger, error);
			free(error);
			destruir_paquete(paquete);
			morir();
		}
}

void log_mensaje(char* mensaje) {
	log_info(logger, mensaje);
}

void morir(){
	finalizar();
	exit(EXIT_FAILURE);
}

void verificarEnvioPlanificador(int envio,t_paquete* paquete){
	if (envio < 0) {
		perror("Error de comunicación con el Planificador");
		log_error(logger, "Error de comunicación con el Planificador");
		destruir_paquete(paquete);
		morir();
	}
}

void verificarEnvioCoordinador(int envio){
	if (envio < 0) {
		perror("Error de comunicación con el Coordinador");
		log_error(logger,"Error de comunicación con el Coordinador");
		morir();
	}
}

void enviar_operacion(t_mensaje_esi mensaje_esi){
	int tamanio = sizeof_mensaje_esi(mensaje_esi);
	void* buffer = serializar_mensaje_esi(mensaje_esi);
	verificarEnvioCoordinador(enviar(socket_coordinador, OPERACION,tamanio, buffer));
	free(buffer);
}

t_mensaje_esi extraer_mensaje_esi(t_esi_operacion operacion, t_paquete* paquete){
	t_mensaje_esi mensaje;
	mensaje.clave_valor = extraerClaveValor(operacion, paquete);
	mensaje.id_esi = ID;
	mensaje.keyword = operacion.keyword;
	return mensaje;
}

t_clavevalor extraerClaveValor(t_esi_operacion operacion,t_paquete* paquete){
	t_clavevalor clavevalor;
	char* error;
	switch (operacion.keyword) {
	case GET:
		clavevalor.clave = operacion.argumentos.GET.clave;
		clavevalor.valor = NULL;
		return clavevalor;
		break;
	case SET:
		clavevalor.clave = operacion.argumentos.SET.clave;
		clavevalor.valor = operacion.argumentos.SET.valor;
		return clavevalor;
		break;
	case STORE:
		clavevalor.clave = operacion.argumentos.STORE.clave;
		clavevalor.valor = NULL;
		return clavevalor;
		break;
	default:
		error = string_from_format("La keyword %d del esi %d no es válida",operacion.keyword, ID);
		log_error(logger, error);
		free(error);
		destruir_paquete(paquete);
		morir();
	}
	return clavevalor;
}


t_mensaje_esi copiarMensajeEsi(t_mensaje_esi original){
	t_mensaje_esi copia;
	copia.keyword = original.keyword;
	copia.id_esi = original.id_esi;
	copia.clave_valor.clave = string_duplicate(original.clave_valor.clave);
	copia.clave_valor.valor = string_duplicate(original.clave_valor.valor);

	return copia;
} // ya no se usa

void liberarClaveValor(t_clavevalor claveValor){
	free(claveValor.clave);
	if(claveValor.valor != NULL)
		free(claveValor.valor);
}  // ya no se usa


void crearLog() {
	logger = log_create("esi.log", "ESI", false, LOG_LEVEL_TRACE);
}
void levantarConfig(char* path) {

	config_aux = config_create(path);

	if (config_has_property(config_aux, "IP_COORDINADOR")) {
		config.ip_coordinador = config_get_string_value(config_aux,"IP_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Coordinador");
		morir();
	}

	if (config_has_property(config_aux, "PUERTO_COORDINADOR")) {
		config.puerto_coordinador = config_get_string_value(config_aux,"PUERTO_COORDINADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Coordinador");
		morir();
	}

	if (config_has_property(config_aux, "IP_PLANIFICADOR")) {
		config.ip_planificador = config_get_string_value(config_aux,"IP_PLANIFICADOR");
	} else {
		log_error(logger, "No se encuentra la ip del Planificador");
		morir();
	}

	if (config_has_property(config_aux, "PUERTO_PLANIFICADOR")) {
		config.puerto_planificador = config_get_string_value(config_aux,"PUERTO_PLANIFICADOR");
	} else {
		log_error(logger, "No se encuentra el puerto del Planificador");
		morir();
	}

	log_mensaje("Se cargó exitosamente la configuración");

}
void conectarPlanificador() {
	socket_planificador = conectar_a_server(config.ip_planificador,config.puerto_planificador);
	enviar(socket_planificador, HANDSHAKE_ESI, 0, NULL);
	t_paquete* paquete;
	paquete = recibir(socket_planificador);

	if (paquete->codigo_operacion != HANDSHAKE_PLANIFICADOR) {
		log_error(logger, "Falló el handshake con el Planificador");
		destruir_paquete(paquete);
		morir();
	} else {
		ID = (int) paquete->data;
		log_mensaje("Handshake exitoso con el Planificador");
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
		morir();
	} else {
		log_mensaje("Handshake exitoso con el Coordinador");
		destruir_paquete(paquete);
	}
}
