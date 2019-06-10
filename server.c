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
#include "board.c"
int cantidad_jugadores = 0;
FILE* fichero;
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
//    printf("Waiting a client to connect...\n");
//  else
//    printf("Error\n");

  //addr_size = sizeof serverStorage;
  // Servidor queda bloqueado aquí hasta que alguien se conecte.
  //newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
  //printf("Client has connected to me!\n");
  return welcomeSocket;
}


int ai;
int aj;
int pi;
int pj;


void sendSignal(int socket, char* package){
  // Obtenemos el largo del payload para saber qué tamaño tiene el paquete y cuántos bytes debe enviar mi socket
  int payloadSize = package[1];
  send(socket, package, 2 + payloadSize, 0);
  fprintf(fichero,"%s %d  ","TIMESTAMP:",(int)time(NULL));
  fprintf(fichero,"ID: %d  ",package[0]);
  fputs("CONTENIDO: ",fichero);
  fputs(package,fichero);
  fputc('\n', fichero);
}

char Nickname1 [255] = "-1";
char Nickname2 [255] = "-1";
int jugada_valida = 1; // 0: si jugada no es válida, 1: jugada válida
int ganador = 1; // 0: si NO GANO, 1: SI  GANO
int empate = 0; // 0: No hay empate, 1: sihay empate
int respuesta_juego = 0;
char mensaje_chat [255];
int desconectado_enmedio = 0;
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
      printf("Posicion Antigua: (%i,%i) \n", content[0],content[1]);
      printf("Posicion Nueva: (%i,%i)", content[2],content[3]);
      ai = content[0];
      aj = content[1];
      pi = content[2];
      pj = content[3];
    }
    else if (mensaje.id == 0x0b){
      printf("desconectado");
    }
    else if (mensaje.id == 0x10){
      printf("Si la respuesta es sí debe agregarlo denuevo como jugador");
      printf("Respuesta es %d ",content[0]);
      respuesta_juego = respuesta_juego + content[0];
      printf(" Respuesa juego %d\n", respuesta_juego);
    }
    else if (mensaje.id == 0x11){
      desconectado_enmedio = 1;
    }
    else if (mensaje.id == 0x13){
      printf("Mensaje recibido desde el cliente");
      printf("Enviando mensaje al recepetor");
      jugada_valida = -1;
      strcpy(mensaje_chat,content);
      //sendSignal(socket, generar_mensaje(0x14,content));
    }

    else {
      printf("Error bad package");
      sendSignal(socket, generar_mensaje(0x12,"ID desconocido"));
    }
    fprintf(fichero,"%s %d  ","TIMESTAMP:",(int)time(NULL));
    fprintf(fichero,"ID: %d  ",mensaje.id);
    fputs("CONTENIDO: ",fichero);
    fputs(content,fichero);
    fputc('\n', fichero);
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
        receiveSignal(cli_sockfd[num_conn]);
        receiveSignal(cli_sockfd[num_conn]);
        if (num_conn == 0) {
            /* Send "HLD" to first client to let the user know the server is waiting on a second client. */
        }

        num_conn++;
    }
}



int main(int argc, char *argv[])
{
    // Se crea tablero
    Board* board = board_init();
    /* Make sure a port was specified. */
    fichero = fopen("log.txt","wt");
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
    int puntaje_jugador1 = 10;
    int puntaje_jugador2 = 56;
    char Puntajes[2];
    Puntajes[0] = puntaje_jugador1;
    Puntajes[1] = puntaje_jugador2;
    int id_jugador1 = 1;
    int id_jugador2 = 2;
    char whos_first_1[1];
    char whos_first_2[1];
    whos_first_1[0] = id_jugador1;
    whos_first_2[0] = id_jugador2;
    int jugador_actual = 0; // Parte jugando el 1..   (falta aleatorio)
    jugador = 1;
    // crear tablero
    char tablero[64];
    //
    //imprimir_tablero(tablero);
    int *cli_sockfd = (int*)malloc(2*sizeof(int)); /* Client sockets */
    // Conexion
    int jugar_denuevo = 0;
    while (1) {
            memset(cli_sockfd, 0, 2*sizeof(int));
            /* Get two clients connected. */
            get_clients(socket_server, cli_sockfd);
            break;
          }
    while (1) {
      //Avisar a los dos clientes que se encontro rival.
      sendSignal(cli_sockfd[0],generar_mensaje(0x05,Nickname2));
      sendSignal(cli_sockfd[1],generar_mensaje(0x05,Nickname1));
      // NoTIFIAR INICIO JUEGO A CLIENTES.
      sendSignal(cli_sockfd[0],generar_mensaje(0x06,"Inicio Juego"));
      sendSignal(cli_sockfd[1],generar_mensaje(0x06,"Inicio Juego"));
      // se mandan puntajes iniciales correspondientes a cada usuario
      sendSignal(cli_sockfd[0],generar_mensaje(0x07,Puntajes));
      sendSignal(cli_sockfd[1],generar_mensaje(0x07,Puntajes));      
      // se mandan WHOS FIRST correspondientes a cada usuario
      sendSignal(cli_sockfd[0],generar_mensaje(0x08,whos_first_1));
      sendSignal(cli_sockfd[1],generar_mensaje(0x08,whos_first_2));
  // Juego
      while(1){
        // se manda tablero a usuario que corresponde jugar
        // se convierte tablero a  numero.
        for (int i = 0; i < 8; i++){
          for (int j = 0; j < 8; j++){
            if (board -> con_fichas[i][j].item =='b'){
                    (tablero[i*8 + j] = 1);}
            if (board -> con_fichas[i][j].item =='n'){
                    (tablero[i*8 + j] = 2);}
            if (board -> con_fichas[i][j].item =='o'){
                    (tablero[i*8 + j] = 3);}
            if (board -> con_fichas[i][j].item =='O'){
                    (tablero[i*8 + j] = 4);}
            if (board -> con_fichas[i][j].item =='x'){
                    (tablero[i*8 + j] = 5);}
            if (board -> con_fichas[i][j].item =='X'){
                    (tablero[i*8 + j] = 6);}
            }   
          }
        sendSignal(cli_sockfd[jugador_actual],generar_mensaje(0x09,tablero));
        // Servidor recibe jugada de cliente
        receiveSignal(cli_sockfd[jugador_actual]);
        if (jugada_valida == -1){
          int otro_jugador;
          if (jugador_actual == 0){
            otro_jugador = 1;
          }
          else{
            otro_jugador = 0;
          }
          sendSignal(cli_sockfd[otro_jugador], generar_mensaje(0x14,mensaje_chat));
          receiveSignal(cli_sockfd[jugador_actual]);
        }
        if (desconectado_enmedio ==1){
          sendSignal(cli_sockfd[0],generar_mensaje(0x11,"DESCONECTA"));
          sendSignal(cli_sockfd[1],generar_mensaje(0x11,"DESCONECTA")); 
          break;     
        }
        // En funcion de ReceiveSignal se determino si jugada es valida o no, si gano o no, y se actualizo el puntaje
        //si jugada es invalida, jugada_valida sera igual a 0. Luego se realizan acciones según esta variable
         // Revisar validez jugada
        if (board_move(board,ai,aj,pi,pj))
        {
          jugada_valida = 1;
        }
        else
        {
          printf("Jugada INVálida \n");
          jugada_valida = 0;
        }
        // se revisa si gano
        if (jugada_valida == 1){
          // se actualiza puntaje
          printf("Jugada Válida \n");
          // revisa si gano, cambia variable ganador.., Si no gana: 
          ganador = 0;
        }
        if (jugada_valida == 0){
          sendSignal(cli_sockfd[jugador_actual],generar_mensaje(0x0b,"Jugada inválida"));
          //se vuelve al principio del loop..
        }
        // jugada válida:
        else
        {
          sendSignal(cli_sockfd[jugador_actual],generar_mensaje(0x0c,"Jugada válida"));
          // se revisa si gano, si no gana, se cambia turno de jugador
          //ganador = 0; // Para probar casos.. después se elimina
          //empate = 0; // Para probar casos.. después se elimina
          if (ganador == 0){
            if (jugador_actual == 0){
              jugador_actual = 1;
              jugador = 2;}
            else{
              jugador_actual = 0;
              jugador = 1;}
            // se mandan puntajes a jugadores:
            sendSignal(cli_sockfd[0],generar_mensaje(0x07,Puntajes));
            sendSignal(cli_sockfd[1],generar_mensaje(0x07,Puntajes));      
            // se vuelve a inicio para que comience jugada el jugador correspondiente
            }

          if (ganador == 1)
          { 
            // END GAME
            sendSignal(cli_sockfd[0],generar_mensaje(0x0d,"Termino Partida"));
            sendSignal(cli_sockfd[1],generar_mensaje(0x0d,"Termino Partida"));   
            // BoardState
            sendSignal(cli_sockfd[0],generar_mensaje(0x09,tablero));
            sendSignal(cli_sockfd[1],generar_mensaje(0x09,tablero));
            // Enviar señal de ganador a jugadores, empate = 0, no hay empate
            // GAME WINNER/LOSER
            char ganador[1];
            if (empate == 0){   
                ganador[0] = 0;  
              }
            else{
              ganador[0] = jugador_actual + 1; // para corresponder numero de enunciado..
            }
            sendSignal(cli_sockfd[0],generar_mensaje(0x0e,ganador));
            sendSignal(cli_sockfd[1],generar_mensaje(0x0e,ganador)); 
            // SCORES
            sendSignal(cli_sockfd[0],generar_mensaje(0x07,Puntajes));
            sendSignal(cli_sockfd[1],generar_mensaje(0x07,Puntajes));  
            if (ganador == 0){
              // SE DESCONECTA SIN PREGUNTAR
              sendSignal(cli_sockfd[0],generar_mensaje(0x11,"DESCONECTA"));
              sendSignal(cli_sockfd[1],generar_mensaje(0x11,"DESCONECTA"));            
            } 
            // ASK NEW GAME
            else{
                sendSignal(cli_sockfd[0],generar_mensaje(0x0f,"ASK GAME"));
                sendSignal(cli_sockfd[1],generar_mensaje(0x0f,"ASK GAME"));  
                receiveSignal(cli_sockfd[0]);
                receiveSignal(cli_sockfd[1]);  
                if (respuesta_juego == 2){ // suma de las respuestas
                  printf("JUGAR DE NUEVO\n");
                  jugar_denuevo = 1;
                  respuesta_juego = 0;   
                }  
                else{
                  sendSignal(cli_sockfd[0],generar_mensaje(0x11,"DESCONECTA"));
                  sendSignal(cli_sockfd[1],generar_mensaje(0x11,"DESCONECTA"));                    
                }   
              }
              break; 
          }
        }
      }
      if (jugar_denuevo == 0){
        printf("NO\n");
        break;
      }
    }
  fclose(fichero);
}