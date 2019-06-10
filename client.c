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
int termino_juego = 0;
int desconeccion = 0;
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
        printf("###### SE INICIA CONEXIÓN CON EL SERVIDOR ######\n");
    }
    else if (mensaje.id == 0x03){
        printf("###### ENTREGAR NICKNAME ######\n");
        printf("Escribe tu nickname: ");
        char nickname[255];
        scanf("%s", nickname);
        printf("%s\n", nickname);
        sendMessage(socket_client,generar_mensaje(0x04,nickname));
    }
    else if (mensaje.id == 0x05){
      printf("####### OPONENTE ENCONTRADO #####\n");
      printf("%s\n", content);
    }
    else if (mensaje.id == 0x06){
      printf("####### COMENZANDO JUEGO #######\n");
      termino_juego = 0;

    }
    else if (mensaje.id == 0x07){
      printf("######### RECIBIENDO PUNTAJE #######\n");
      printf("puntaje jugador 1: %d \n",content[0]);
      printf("puntaje jugador 2: %d \n",content[1]);

      //implementar función mostrar puntaje
    }
    else if (mensaje.id == 0x08){
      printf("Who is First. Recibiendo si se inicia jugada\n ");
      color_ficha = content[0];
      printf("Mi color de ficha es %d\n", color_ficha);
      //ENDGAME
      
    }
    else if (mensaje.id == 0x09){
      printf("---------- TABLERO ----------- \n");
      int fin_turno =0;
      imprimir_tablero(content);
      printf("------------------------------ \n");
      int respuesta;
      printf("¿Que deseas hacer?\n");
      printf("(1) Mover Ficha\n");
      printf("(2) Enviar Mensaje\n");
      printf("(0) Desconectarse\n");
      scanf("%d", &respuesta);
      if (respuesta == 1){
        if (termino_juego == 0){
          int i_actual;
          int j_actual; 
          int i_a_poner; 
          int j_a_poner;
          printf("Posicion i actual: \n");
          scanf("%d", &i_actual);
          printf("Posicion j actual: \n");
          scanf("%d", &j_actual);
          printf("Posicion i a poner: \n");
          scanf("%d", &i_a_poner);
          printf("Posicion j a poner: \n");
          scanf("%d", &j_a_poner);
          char posiciones_a_mandar[4]; 
          posiciones_a_mandar[0] = i_actual;
          posiciones_a_mandar[1] = j_actual;
          posiciones_a_mandar[2] = i_a_poner;
          posiciones_a_mandar[3] = j_a_poner;
          sendMessage(socket_client,generar_mensaje(0x0a,posiciones_a_mandar));
          fin_turno = 1;
        }
      }
      else if (respuesta == 2){
        char mensaje_chat [255];
        printf("Escriba el mensaje a enviar\n");
        scanf("%s", mensaje_chat);
        sendMessage(socket_client,generar_mensaje(0x13,mensaje_chat));
      }
      else if (respuesta ==0){
        printf("Desconectandose\n");
        sendMessage(socket_client,generar_mensaje(0x11,"DESCONECTA"));
        fin_turno = 1;
      }
      if (fin_turno == 0){
        int respuesta2;
        printf("¿Que deseas hacer?\n");
        printf("(1) Mover Ficha\n");
        printf("(0) Desconectarse\n");
        scanf("%d", &respuesta2);
        if (respuesta2 == 1){
          if (termino_juego == 0){
            int i_actual;
            int j_actual; 
            int i_a_poner; 
            int j_a_poner;
            printf("Posicion i actual: \n");
            scanf("%d", &i_actual);
            printf("Posicion j actual: \n");
            scanf("%d", &j_actual);
            printf("Posicion i a poner: \n");
            scanf("%d", &i_a_poner);
            printf("Posicion j a poner: \n");
            scanf("%d", &j_a_poner);
            char posiciones_a_mandar[4]; 
            posiciones_a_mandar[0] = i_actual;
            posiciones_a_mandar[1] = j_actual;
            posiciones_a_mandar[2] = i_a_poner;
            posiciones_a_mandar[3] = j_a_poner;
            sendMessage(socket_client,generar_mensaje(0x0a,posiciones_a_mandar));
            fin_turno = 1;
          }
        }
        else if (respuesta2 ==0){
          printf("Desconectandose\n");
          sendMessage(socket_client,generar_mensaje(0x11,"DESCONECTA"));
          fin_turno = 1;
        }
      }
    }
    else if (mensaje.id == 0x0b){
        printf("Jugada inválida, intente nuevamente..\n");
    }
    else if (mensaje.id == 0x0c){
        printf("Jugada VÁLIDA\n");
    }
    else if (mensaje.id == 0x0d){
        termino_juego = 1;
        printf("Término Partida\n");
    }
    else if (mensaje.id == 0x0e){
        printf("Ganador: %d\n", content[0]);
    }
    else if (mensaje.id == 0x0f){
        printf("ASK NEW GAME: \n ");
        int respuesta;
        scanf("%d", &respuesta); // 1 Ó 0..
        char posiciones_a_mandar[1];
        posiciones_a_mandar[0] = respuesta;
        sendMessage(socket_client,generar_mensaje(0x10, posiciones_a_mandar));
    }
    else if (mensaje.id == 0x11){
        printf("####### DESCONECCIÓN #######\n");
        desconeccion = 1;
    }
    else if (mensaje.id == 0x14){
      printf("#################\n");
      printf("Mensaje recibido:\n");
      printf("%s\n", content);
      printf("#################\n");
    }
    else if (mensaje.id == 0x12){
        printf("####### Bad Package ########");
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
    printf("DAMAS. \n------------\n");
    char nickname[256]; // nombre del jugador
    int winner; // contiene 1 si gané, 2 si perdí

    Message message;


    while(1) {
        // RECIBE SEÑAL DE SERVIDOR
        receiveSignalClient(socket_client);
        if (desconeccion == 1){
            break;
        }
    }

    /* Close server socket and exit. */
    //close(sockfd);
    return 0;
}
