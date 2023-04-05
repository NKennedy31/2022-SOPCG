#include <comunicacion.h>
#include <utils/test_serializacion.h>
int fd_kernel_dispatch =-1;
int fd_kernel_interrupt = -1;
int fd_memoria = -1;
int fd_dispatch = -1;
int fd_interrupt = -1;
int fd_kernel = -1;

char *ip_cpu;
pthread_t crear_server_dispatch;
pthread_t crear_server_interrupt;
pthread_t ciclo_instrucciones_thread;
int cliente_socket; //Siempre es Kernel
pcb *pcb_actual;
pthread_t conexion_memoria;
sem_t semHayPCBCargada;
sem_t semNoEstoyEjecutando;
sem_t semEsperandoRespuestaDeMarco;
sem_t sem_interrupt;
bool inicializacion_fd_completada = false;
//INTERRUPCION
int interrupcion=0;




void cortar_conexiones() {
    liberar_conexion(&fd_memoria);
    log_info(logger_cpu, "CONEXIONES LIBERADAS");
}

void cerrar_servers() {
    close(fd_dispatch);
    close(fd_interrupt);
    log_info(logger_cpu, "SERVIDORES CERRADOS");
}


bool generar_conexiones() {
    generarConexionesConMemoria();
    levantarConexiones();
    return true;
}


int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if(recv(socket_cliente, &cod_op, sizeof(op_code), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}

bool generarConexionesConMemoria() {
    char *ip;

    ip = strdup(cfg_cpu->IP_MEMORIA);
    log_info(logger_cpu, "Lei la ip %s", ip);

    char *puerto;
    puerto = strdup(cfg_cpu->PUERTO_MEMORIA);

    log_info(logger_cpu, "Lei el puerto %s", puerto);

    fd_memoria = crear_conexion(
            logger_cpu,
            "SERVER MEMORIA",
            ip,
            puerto
    );
    int *estado = malloc(sizeof(int));
    *estado = fd_memoria != 0;

    enviarOrden(HANDSHAKE_CPU, fd_memoria, logger_cpu);
    recibir_operacion(fd_memoria);

    uint32_t* array = recibir_int_array(fd_memoria);

    tamanioDePagina = array[1];
    log_info(logger_cpu,"Cargo el tamaño de pagina : <%d>", tamanioDePagina);
    entradasPorTabla = array[2];
    log_info(logger_cpu,"Cargo las entradas por tabla: <%d>", entradasPorTabla);
    //TODO ASIGNAR LO QUE HAGA FALTA EN EL HANDSHAKE

    free(ip);
    free(puerto);

    return (void *) estado;

}




bool levantarConexiones(){
    fd_interrupt= iniciar_servidor(logger_cpu,"SERVER CPU INTERRUPT", ip_cpu, cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
    if (fd_interrupt == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor INTERRUPT, cerrando CPU");
        return EXIT_FAILURE;
    }

    fd_dispatch = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", ip_cpu, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);

    if (fd_dispatch == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor DISPATCH, cerrando CPU");
        return EXIT_FAILURE;
    }
    pthread_t atenderNuevoPcbHilo;
    pthread_t atenderInterruptHilo;

    fd_kernel_dispatch = esperar_cliente(logger_cpu,"SERVER CPU DISPATCH",fd_dispatch);
    pthread_create(&atenderNuevoPcbHilo,NULL,atenderPcb,NULL);
    fd_kernel_interrupt = esperar_cliente(logger_cpu, "SERVER CPU INTERRUPT", fd_interrupt);
    pthread_create(&atenderInterruptHilo,NULL,atenderInterrupcion,NULL);
    fd_kernel = fd_kernel_dispatch;
    pthread_join(atenderNuevoPcbHilo,NULL);
    pthread_join(atenderInterruptHilo,NULL);


}












