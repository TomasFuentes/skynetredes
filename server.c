#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "protocolo.h"

int cantidad_jugadores = 0;

/* Función que inicializa el servidor en el port
con ip */
int initializeServer(char* ip, int port){
	int welcomeSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	/*---- Creación del Socket. Se pasan 3 argumentos ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP en este caso) */
	welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

  // Pueden buscar como configurar el socket para que libere el puerto una vez que se termine el programa
  
	/*---- Configuración de la estructura del servidor ----*/
	/* Address family = Internet */
	serverAddr.sin_family = AF_INET;
	/* Set port number */
	serverAddr.sin_port = htons(port);
	/* Setear IP address como localhost */
	serverAddr.sin_addr.s_addr = inet_addr(ip);
	/* Setear todos los bits del padding en 0 */
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*---- Bindear la struct al socket ----*/
	bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	/*---- Listen del socket, con un máximo de 5 conexiones (solo como ejemplo) ----*/
	//if(listen(welcomeSocket,5)==0)
//		printf("Waiting a client to connect...\n");
//	else
//		printf("Error\n");

	//addr_size = sizeof serverStorage;
  // Servidor queda bloqueado aquí hasta que alguien se conecte.
	//newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
	//printf("Client has connected to me!\n");
	return welcomeSocket;
}

void sendSignal(int socket, char* package){
  // Obtenemos el largo del payload para saber qué tamaño tiene el paquete y cuántos bytes debe enviar mi socket
  int payloadSize = package[1];
  send(socket, package, 2 + payloadSize, 0);
}

char Nickname1 [255] = "-1";
char Nickname2 [255] = "-1";

void receiveSignal(int socket){
    printf("Waiting message... \n");
    // Esperamos a que llegue el primer byte, que corresponde al ID del paquete
    char ID;
    recv(socket, &ID, 1, 0);
    printf("\n####### Paquete recibido ####\n");
    printf("The ID is: %d\n", ID); // lo imprimimos como número porque esa es la interpretación que acordamos
        // Recibimos el payload size en el siguiente byte
    char payloadSize;
    recv(socket, &payloadSize, 1, 0);
    printf("The PayloadSize is: %d\n", payloadSize);
    // Recibimos el resto del paquete, según el payloadSize. Lo guardamos en un puntero de caracteres, porque no es necesario modificarlo
    char *content = malloc(payloadSize);
    if (payloadSize != 0){
      recv(socket, content, payloadSize, 0);
      printf("The Content is: %s\n", content);
    }
    else{
      strcpy(content,"sin payload");
    }

    // Aqui se las ingenian para ver como retornan todo. Puden retornar el paquete y separarlo afuera, o retornar una struct.
    Message mensaje = read_message(ID,payloadSize,content);
    if (mensaje.id == 0x01){
      printf("SE INICIA CONEXIÓN CON CLIENTE");
      sendSignal(socket, generar_mensaje(0x02,"Se inicio conexion"));
      sendSignal(socket, generar_mensaje(0x03,"Preguntar Nickname"));
      // RESPONDEMOS CON CONEXIÓN ESTABLISHED 0x02
      // PREGUNTAMOS NICKNAME 0x03
    }
    else if (mensaje.id == 0x04){
      printf("RECIBIR NICKNAME DE CLIENTE %s", content);
      if (strcmp(Nickname1,"-1") == 0){
        strcpy(Nickname1,content);
      }
      else{
        strcpy(Nickname2,content);
      }
      // revisar si es que hay dos oponentes
      // si hay dos oponentes iniciamos partida.
    }
    else if (mensaje.id == 0x0a){
      printf("RECIBIENDO MOVIDA");
      printf("%s", mensaje.char_array);
      // Revisar validez jugada
      // Responder si la jugada NO es válida.
      // Si la jugada es válida, se actualiza el tablero y se indica si ganó o no. Y se envían los puntaje a AMBOS Jugadores
    }
    else if (mensaje.id == 0x10){
      printf("Si la respuesta es sí debe agregarlo denuevo como jugador");
      printf("Respuesta es %d ",mensaje.int_content);
      //SEND MOVE
    }
    else if (mensaje.id == 0x13){
      printf("Si la respuesta es sí debe agregarlo denuevo como jugador");
      printf("Respuesta es %d ",mensaje.int_content);
      //ENDGAME
    }
    free(content);
}



/* Sets up the client sockets and client connections. */
void get_clients(int lis_sockfd, int * cli_sockfd)
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int num_conn = 0;
    while(num_conn < 2)
    {
        printf("Esperando cliente\n" );
        listen(lis_sockfd, 2);
        memset(&cli_addr, 0, sizeof(cli_addr));
        clilen = sizeof(cli_addr);
      // Accept the connection from the client. 
        cli_sockfd[num_conn] = accept(lis_sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // <enviar mensaje id 2: connection established a player>
        if (cli_sockfd[num_conn] < 0)
            /* Horrible things have happened. */
            printf("ERROR accepting a connection from a client.");
        /* Send the client it's ID. */
        //write(cli_sockfd[num_conn], &num_conn, sizeof(int));
        /* Increment the player count. */
        /* Recibe señal de Star conecction */
        receiveSignal(cli_sockfd[num_conn]);
        /* Recibe señal de Return Nickname */
        receiveSignal(cli_sockfd[num_conn]);
        //write_client_msg(cli_sockfd[num_conn], generar_mensaje(2,""));
        //write_client_msg(cli_sockfd[num_conn], generar_mensaje(3,""));
        // Recibe mensaje de cliente Start Connection
        //recv_msg(cli_sockfd[num_conn], msg);
        if (num_conn == 0) {
            /* Send "HLD" to first client to let the user know the server is waiting on a second client. */
        }

        num_conn++;
    }
}



int main(int argc, char *argv[])
{
    /* Make sure a port was specified. */
    char *IP;
    int port;
    int with_log = 0;
    int socket_server;
    if (argc < 5) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    if (strcmp(argv[1], "-i") == 0){
      IP = argv[2];
    }
    else if (strcmp(argv[3], "-i") == 0){
      IP = argv[4];
    }

    if (strcmp(argv[1], "-p") == 0){
      port = atoi(argv[2]);
    }
    else if (strcmp(argv[3], "-p") == 0){
      port = atoi(argv[4]);
    }
    if (argc == 6){
        if (strcmp(argv[5], "-l") == 0){
            with_log = 1;
        }
    }
    socket_server = initializeServer(IP, port);
    while (1) {
            int *cli_sockfd = (int*)malloc(2*sizeof(int)); /* Client sockets */
            memset(cli_sockfd, 0, 2*sizeof(int));
            /* Get two clients connected. */
            get_clients(socket_server, cli_sockfd);
            // Avisar a los dos clientes que se encontro rival.
            sendSignal(cli_sockfd[0],generar_mensaje(0x05,Nickname2));
            sendSignal(cli_sockfd[1],generar_mensaje(0x05,Nickname1));
            // NoTIFIAR INICIO JUEGO A CLIENTES.
            sendSignal(cli_sockfd[0],generar_mensaje(0x06,"Inicio Juego"));
            sendSignal(cli_sockfd[1],generar_mensaje(0x06,"Inicio Juego"));
        }
}
