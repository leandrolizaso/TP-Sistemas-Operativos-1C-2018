#include "Instancia.h"
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pelao/sockets.h>
#include <pelao/protocolo.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

/*#define CFG_IP  "ip_coordinador"
#define CFG_PORT  "port_coordinador"
#define CFG_ALGO  "distribution_replacement"
#define CFG_POINT  "point_mount"
#define CFG_NAME_INST  "name_instancia"
#define CFG_INTERVAL  "interval"
#define CFG_TAMANIO   100*/

typedef int sig_atomic_t;
sig_atomic_t (CFG_TAMANIO);


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

	t_paquete* paquete;
	paquete = recibir(socket_coordinador);
	if(paquete->codigo_operacion == HANDSHAKE_COORDINADOR){
		log_info(logger, "Mensaje recibido de coordinador");
	}else{
		log_info(logger, "Error al recibir mensaje de corrdinador");
	}

	destruir_paquete(paquete);
};

struct entrada{
	int t_clave;
	char* clave;
	int t_valor;
	char* valor;
	int numero_entrada;
};

listas* listaAlmacenamiento = list_create();

void push (t_list* listaAlmacenamiento, t_entrada* valores){
	t_list* aux = malloc(sizeof(t_list));
	aux->head->data = valores;
	aux->elements_count = listaAlmacenamiento->head;
	listaAlmacenamiento = aux;

	return listaAlmacenamiento;
};
void pop (t_list* listaAlmacenamiento){
     t_entrada* x;
     t_list* aux = listaAlmacenamiento;
     x = aux->info;
	aux = aux->siguiente;

	return x;
};
void crearEntrada (t_clavevalor* claveValor){
	t_entrada* bloque;
	bloque = malloc(sizeof(t_entrada));
	bloque->clave = malloc(strlen(clave) + 1);
	bloque->valor = malloc (strlen(valor) + 1);
	bloque->clave = claveValor->clave;
	bloque->valor = claveValor->valor;
	bloque->t_clave = sizeof(claveValor->clave);
	bloque->t_valor = sizeof(claveValor->valor);

	if (bloque == NULL){
		log_error(logger, "Error al asignar memoria");
	} else {
		push (t_list* listaAlmacenamiento, bloque);
		bloque->numero_entrada = (bloque->numero_entrada) + 1;
		log_info (logger, "Se creo entrada")
	}
	destruir_entrada(bloque);
	return listaAlmacenamiento;
};

int destruir_entrada (t_entrada* bloque){
	free (bloque->clave);
	free (bloque->valor);
	free (bloque->numero_entrada);
	free (bloque);

	log_info(logger, "La memoria qdó libre");

	return 0;
};
void verificarOperacion (int codOperacion, t_clavevalor* claveValor){
	switch (codOperacion){
	case SET_CLAVE:
		crearEntrada (claveValor);
		log_info(logger, "Creación de entrada exitosa");
		break;
	case GET_CLAVE:
		buscarEntrada(claveValor->clave);
		break;
	case STORE_CLAVE:
		almacenarClaves();
		break;
	case SAVE_CLAVE:
		guardarClaves();
		break;
	default:
		log_error(logger, "Error al recibir operacion");
			};
};
void dump (int intervalo){
	int tiempo = intervalo;
	alarm (tiempo);
    while(tiempo > -2){
	signal(SIGALRM, guardarClaves);
	tiempo = intervalo;
	alarm(tiempo);
	log_info(t_log* logger, "Se guardaron datos");
 	 };
  };

int archivo;
char* memo;
char* filePath = CFG_POINT;
char* punto_montaje= malloc(strlen(filePath) + 1);

strcpy(punto_montaje, filePath);

void buscarEntrada(struct t_entrada* listaAlmacenamiento, t_clavevalor* info){
     int cod;

     t_clavevalor* getClave = malloc(strlen(info->clave)+ 1);
     strcpy(getClave, info->clave);
     t_entrada* claveGuardada = malloc(strlen(info->clave)+ 1);
	strcpy(claveGuardada, bloque->clave);
     cod = strcmp (getClave->clave, claveGuardada->clave);

     if (cod == 0){
          log_info(logger, "se encontro clave");
          return claveGuardada->valor;
     } else {
          while (cod != 0){

               claveGuardada = pop(struct Nodo* pila);
               cod = strcmp (getClave->valor, claveGuardada->valor);
          };
          log_info(logger, "se encontro clave");
          return claveGuardada->valor;
     };
};

void guardarClaves(listaAlmacenamiento ){
	t_log* logger;
	archivo = open(punto_montaje, O_CREAT, S_IRWXU);
	if (archivo == NULL){
            log_error(logger, "Error de apertura del archivo.");
        }else {
		size_t len = strlen(listaAlmacenamiento->head->) + 1;

		lseek(archivo, len-1, SEEK_SET);
		write(archivo, "", 1);

		memo = mmap(NULL, len , PROT_WRITE | PROT_READ, MAP_SHARED, archivo, 0);
		memcpy(memo, lista->valor, len);
		msync(memo, len, MS_SYNC);
		munmap(memo, len);
		log_info(logger, "se guardaron las claves");
		free(filePath);
		free(punto_montaje);
       		 };
        fclose(archivo);
};
void almacenarClaves (t_entrada* lista){
	t_log* log_almacen;
 	guardarClaves(lista);
	destruir_entrada (lista);
	log_info(log_almacen, "se almacenaron las entradas");
};
void esValorAtomico(t_entrada* lista){
	if (lista->t_vañor <= sig_atomic_t){
		return true;
	}else{return false;};
};
void algoritmoCircular(t_entrada* bloque, t_list listaAlmacenamiento){
     int i=0;
     while (i != CFG_CAPACIDAD){

     bool remplazar = esValorAtomico(listaAlmacenamiento);
      if(remplazar == true){
          list_replace(listaAlmacenamiento, i, bloque);
          i++;} else{
               i++;
          };
};
     return listaAlmacenamiento;
};
