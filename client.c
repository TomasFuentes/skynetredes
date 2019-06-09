#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "protocolo.h"

int socket_client;
int conectados = 0;
void error(const char *msg)
{
    printf("Either the server shut down or the other player disconnected.\nGame over.\n");
    exit(0);
}
void write_server_msg(int sockfd, unsigned char * msg){
  int n = write(sockfd, msg, strlen(msg));
  if (n < 0){
      error("ERROR writing int to server socket");
    }
    printf("Se envió un mensaje al servidor: id = %x, payload_len = %x\n", msg[0], msg[1]);
}

void sendSignal(int socket, char* package){
  // Obtenemos el largo del payload para saber qué tamaño tiene el paquete y cuántos bytes debe enviar mi socket
  int payloadSize = package[1];
  send(socket, package, 2 + payloadSize, 0);
}



int initializeClient(char* ip, int port){
	int clientSocket;
  //char buffer[1024];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  /*---- Creación del Socket. Se pasan 3 argumentos ----*/
	/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP en este caso) */
  clientSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  /*---- Configuración de la estructura del servidor ----*/
	/* Address family = Internet */
  serverAddr.sin_family = AF_INET;
	/* Set port number */
  serverAddr.sin_port = htons(port);
  /* Setear IP address como localhost */
  serverAddr.sin_addr.s_addr = inet_addr(ip);
  /* Setear todos los bits del padding en 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*---- Conectar el socket al server ----*/
  addr_size = sizeof serverAddr;
  int ret = connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
  if (ret == 0) printf("Connected to server!\n");
  else printf("Error connecting to server. Exit code: %d (probablemente el puerto está tomado. Solución: esperar unos segundos, cambiar el puerto o configurar el socket)\n", ret);
  // RECUERDEN MANEJAR ERRORES EN CADA FUNCIÓN EXTERNA!!

	return clientSocket;
}
int color_ficha; // 1: ficha blanca, 2 fichas negras
void receiveSignalClient(int socket){
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
    if (payloadSize != 0) {
        recv(socket, content, payloadSize, 0);
    }
    printf("The Content is: %s\n", content);
    // Aqui se las ingenian para ver como retornan todo. Puden retornar el paquete y separarlo afuera, o retornar una struct.
    Message mensaje = read_message(ID,payloadSize,content);
    if (mensaje.id == 0x02){
        printf("SE INICIA CONEXIÓN CON EL SERVIDOR");
    }
    else if (mensaje.id == 0x03){
        printf("ENTREGAR NICKNAME");
        char nickname[255];
        scanf("%s", nickname);
        printf("%s\n", nickname);
        sendMessage(socket_client,generar_mensaje(0x04,nickname));
    }
    else if (mensaje.id == 0x05){
      printf("OPONENTE ENCONTRADO");
      printf("%s\n", content);
    }
    else if (mensaje.id == 0x06){
      printf("COMENZANDO JUEGO");
      //OPERACIONES DE INICIO DE JUEGO (RECIBIR TABLERO y DESPLEGARLO y mostrar scores)
      //crear funcion desplegar tablero.
    }
    else if (mensaje.id == 0x07){
      printf("RECIBIENDO PUNTAJE");
      printf("puntaje jugador 1: %d",content[0]);
      printf("puntaje jugador 2: %d",content[1]);

      //implementar función mostrar puntaje
    }
    else if (mensaje.id == 0x08){
      printf("Who is First. Recibiendo si se inicia jugada ");
      color_ficha = content[0];
      printf("Mi color de ficha es %d\n", color_ficha);
      //ENDGAME
      
    }
    else if (mensaje.id == 0x09){
      printf("TAblERO \n");
      imprimir_tablero(content);
      int i_actual;
      int j_actual; 
      int i_a_poner; 
      int j_a_poner;
      char hola[255];
      printf("Posicion i actual: \n");
      scanf("%d", &i_actual);
      printf("Posicion j actual: \n");
      scanf("%d", &j_actual);
      printf("Posicion i a ponr: \n");
      scanf("%d", &i_a_poner);
      printf("Posicion a a poner: \n");
      scanf("%d", &j_a_poner);
      char posiciones_a_mandar[4]; 
      posiciones_a_mandar[0] = i_actual;
      posiciones_a_mandar[1] = j_actual;
      posiciones_a_mandar[2] = i_a_poner;
      posiciones_a_mandar[3] = j_a_poner;
      sendMessage(socket_client,generar_mensaje(0x0a,posiciones_a_mandar));


      //ENDGAME
      
    }
    else if (mensaje.id == 0x0b){
        printf("Jugada inválida, intente nuevamente..\n");
    }
    free(content);
}




int main(int argc, char *argv[])
{
    char *IP;
    int port;
    int with_log = 0;
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

    /* Connect to the server. */
    socket_client = initializeClient(IP, port);
    /* Enviar mensaje start connection. */
    sendMessage(socket_client,generar_mensaje(0x01,"Payload: 0"));




    /* Make sure host and port are specified. */
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    unsigned char msg[1024];
    printf("Poker \n------------\n");
    char nickname[256]; // nombre del jugador
    char * opp_nickname; // nombre del contrincante
    int balance; // saldo del jugador
    int hand[10]; // cartas en la mano del jugador
    int first; // quien va primero, puede ser 1 o 2
    char * bet_mapping[5] = {"FOLD", "$0", "$100", "$200", "$500"};
    int * bets; // contiene las apuestas disponibles, mandadas por el servidor
    int message_len; // contiene el largo del arreglo bets
    int * opp_hand; // cartas del contrincante
    int winner; // contiene 1 si gané, 2 si perdí

    Message message;


    while(1) {
        printf("waiting\n");
        // RECIBE SEÑAL DE SERVIDOR
        receiveSignalClient(socket_client);
    }

    /* Close server socket and exit. */
    //close(sockfd);
    return 0;
}
