/*
 * sockets.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */

#include "sockets.h"

/* Crea un socket servidor, lo setea para escuchar a varios al mismo tiempo y lo pone a escuchar */

int crear_server(char* puerto){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &serverInfo);

	int listenningSocket;
	if((listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0) {
		perror("Error al crear socket");
		exit(EXIT_FAILURE);
	};

	int enable = 1;
	if(setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
		perror("Error al setear socket");
		exit(EXIT_FAILURE);
	};

	if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)== -1){
		perror("Error al bindear socket");
		exit(EXIT_FAILURE);
	};

	freeaddrinfo(serverInfo);

	if (listen(listenningSocket, BACKLOG) == -1) {
		perror("Error al poner a escuchar al socket");
		exit(EXIT_FAILURE);
	}

	return listenningSocket;
}

/* Crea un socket cliente y lo conecta al server */

int conectar_a_server(char* ip, char* puerto){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	int serverSocket;
	if((serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0) {
		perror("Error al crear socket");
		exit(EXIT_FAILURE);
	};


	if(connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
		perror("Error al conectar al server");
		exit(EXIT_FAILURE);
	};

	freeaddrinfo(serverInfo);

	return serverSocket;
}

/* Acepta una nueva conexión y devuelve el nuevo socket conectado */

int aceptar_conexion(int socketServidor) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(socketServidor, (struct sockaddr *) &addr, &addrlen);
	if (socketCliente < 0) {

		perror("Error al aceptar cliente");
		return -1;
	}

	return socketCliente;
}

/* Devuelve la IP de un socket */

char* get_ip_socket(int fd){
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(fd, (struct sockaddr *)&addr, &addr_size);
	if(res == -1){
		return NULL;
	}
	char ipNodo[20];
	strcpy(ipNodo, inet_ntoa(addr.sin_addr));
	return strdup(ipNodo);
}

/* Cierra un socket */

void cerrar_socket(int socket){
	close(socket);
}

/* Función para manejar múltiples conexiones. Recibe un puerto y una función "procesar_mensaje" que recibe como argumento un socket */

int stop_multiplexar = 0;

void multiplexar(char * puerto, int (*procesar_mensaje)(int)) {
	fd_set master;
	fd_set read_fds;
	int fdmax;
	int listener;
	int i;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	listener = crear_server(puerto);
	FD_SET(listener, &master);
	fdmax = listener;
	while (!stop_multiplexar) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					int newfd = aceptar_conexion(listener);
					FD_SET(newfd, &master);
					if (newfd > fdmax) {
						fdmax = newfd;
					}
					//printf("Nueva conexion con fd: %d\n",newfd);
				} else {

					int resultado = procesar_mensaje(i);
					if(resultado < 0) {
						cerrar_socket(i);
					}
					if(resultado <= 0) {
						FD_CLR(i, &master);
					}
				}
			}
		}
	}
}

int enviar_string(int fd ,char* mensaje){
	int total =0;
	int tamanio = string_length(mensaje);
	int pendiente = tamanio;
	char* msj = string_duplicate(mensaje);

	int tamanioEnviado = enviar_int(fd, tamanio);
	if (tamanioEnviado < 0) {
		perror("Error al enviar tamaño");
		free(msj);
		return -1;
	}

	while (total < pendiente) {

		int enviado = send(fd, msj, tamanio, MSG_NOSIGNAL);
		if (enviado < 0) {
			free(msj);
			return -1;
		}
		total += enviado;
		pendiente -= enviado;
	}
	free(msj);
	return total;
}

char* recibir_string(int fd) {
	int tamanio = recibir_int(fd);
	char* buffer;

	if (tamanio < 0) {
		perror("Error al recibir tamaño");
		return NULL;
	}
	if(tamanio == 0 ){
		return NULL;
	}

	buffer = malloc(tamanio+1);

	if (recv(fd, buffer,tamanio, MSG_WAITALL) < 0) {
		perror("Error al recibir string");
		free(buffer);
		return NULL;
	}

	buffer[tamanio] = '\0';

	return buffer;
}

int enviar_int(int fd, int32_t numero){
	return (send(fd, &numero, sizeof(int32_t),MSG_NOSIGNAL));
}

int32_t recibir_int(int fd){
	int32_t numero = 0;
	if(recv(fd, &numero, sizeof(int32_t),MSG_WAITALL) < 0){
		perror("Error al recibir numero");
		return -1;
	}
	return numero;
}

int enviar(int socket, int codigo_operacion, int tamanio, void * data) { //Para enviar un handshake: tamanio = 0 y data = NULL

	void * buffer;
	int res;

	if(tamanio != 0){

	int tamanio_paquete = 2 * sizeof(int) + tamanio;
	buffer = malloc(tamanio_paquete);

	memcpy(buffer, &codigo_operacion, sizeof(int));
	memcpy(buffer + sizeof(int), &tamanio, sizeof(int));
	memcpy(buffer + 2 * sizeof(int), data, tamanio);

	res = send(socket, buffer, tamanio_paquete, MSG_NOSIGNAL);

	}else{

		buffer = malloc(2 * sizeof(int));
		memcpy(buffer, &codigo_operacion, sizeof(int));
		memcpy(buffer + sizeof(int), &tamanio, sizeof(int));
		res = send(socket, buffer, 2 * sizeof(int), MSG_NOSIGNAL);
	}

	free(buffer);

	return res;

}

t_paquete* recibir(int socket) {

	t_paquete * paquete = malloc(sizeof(t_paquete));

	int res = recv(socket, &paquete->codigo_operacion, sizeof(int),
			MSG_WAITALL);

	if (res < 0) {
		paquete->codigo_operacion = -1; //Si el cod. de operacion es -1 indica que hubo un error al recibir el paquete
		paquete->tamanio = -1;
		paquete->data = NULL;
		return paquete;

	}

	recv(socket, &paquete->tamanio, sizeof(int), MSG_WAITALL);

	if (paquete->tamanio == 0) { // Si el tamanio es cero, no hay payload que recibir. Por ejemplo al recibir un handshake
		paquete->data = NULL;
	} else {
		void * info = malloc(paquete->tamanio);

		recv(socket, info, paquete->tamanio, MSG_WAITALL);

		paquete->data = info;
	}

	return paquete;
}

void destruir_paquete(t_paquete* paq){
	free(paq->data);
	free(paq);
}
