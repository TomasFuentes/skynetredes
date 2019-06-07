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
    recv(socket, content, payloadSize, 0);
    printf("The Content is: %s\n", content);
    // Aqui se las ingenian para ver como retornan todo. Puden retornar el paquete y separarlo afuera, o retornar una struct.
    Message mensaje = read_message(ID,payloadSize,content);
    if (mensaje.id == 0x02){
        printf("SE INICIA CONEXIÓN CON EL SERVIDOR");
    }
    else if (mensaje.id == 0x03){
        printf("ENTREGAR NICKNAME");
        char* nickname;
        scanf("%s", nickname);
        sendMessage(socket_client,generar_mensaje(0x04,nickname));
    }
    else if (mensaje.id == 0x05){
      printf("OPONENTE ENCONTRADO");
    }
    else if (mensaje.id == 0x06){
      printf("COMENZANDO JUEGO");
      //OPERACIONES DE INICIO DE JUEGO (RECIBIR TABLERO y DESPLEGARLO y mostrar scores)
      //crear funcion desplegar tablero.
    }
    else if (mensaje.id == 0x07){
      printf("RECIBIENDO PUNTAJE");
      //implementar función mostrar puntaje
    }
    else if (mensaje.id == 0x08){
      printf("Who is First. Recibiendo si se inicia jugada ");
      //ENDGAME
      
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
    /* Make sure host and port are specified. */
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    /* The client ID is the first thing we receive after connecting. */
    //    int id = recv_int(sockfd);


    unsigned char msg[1024];

    printf("Poker \n------------\n");

    // /* Wait for the game to start. */
    // do {
    //     recv_msg(sockfd, msg);
    //     if (!strcmp(msg, "HLD"))
    //         printf("Waiting for a second player...\n");
    // } while ( strcmp(msg, "SRT") );

    /* The game has begun. */
    // printf("Comienza el juego !!\n");
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
       // recv_msg(sockfd, msg);
        message = read_message(msg);
        printf("%d\n", message.id);

        if(message.id == 2){ // connection established
            printf("Connected to server\n");
        }
        //else if(message.id == 3){ // ask nickname
            //printf("Enter your nickname: ");
            //scanf("%s", nickname);
            //printf("\n");
            //write_server_msg(sockfd, generar_mensaje(4, nickname));
        // }
        else if(message.id == 5){ // opponent found
            opp_nickname = "opponent";
            printf("Opponent found: %s\n", opp_nickname);
        }
        if(message.id == 6){ // initial pot
            printf("Initial pot $%d\n", balance);
        }
        else if(message.id == 7){ // game start
            printf("Starting game\n");
        }
        else if(message.id == 8){ // start round
            printf("Starting game\n");
            printf("Current pot: $%d\n", balance);
        }
        else if(message.id == 9){ // initial bet
            printf("Initial bet: $10\n");
        }
        else if(message.id == 10){ // 5 cards
            printf("Here are your cards:\n");
            // guardarlas en hand
            for(int i=0; i<10; i+=2){
                printf("\t");
                print_card(hand[i], hand[i+1]);
            }
        }
        else if(message.id == 11){ // who's first
            if (first == 1){
                printf("You go first\n");
            }
            else if (first == 2){
                printf("Opponent goes first\n");
            }
        }
        else if(message.id == 12){ // get cards to change
            printf("You can now change your cards\n");
            int change[5];
            int cards_to_change[10];

            for(int i=0; i<5; i++){
                char answer;
                printf("Do you want to change ");
                print_card(hand[2*i], hand[2*i+1]);
                printf("? (y/n)\n");
                scanf(" %c", &answer);
                change[i] = answer;
                if (answer == 'y'){
                    change[i] = 1;
                }
                else{
                    change[i] = 0;
                }
            }
            for (int i=0; i<5; i++){
                if (change[i] == 1){
                    cards_to_change[2*i] = hand[2*i];
                    cards_to_change[2*i+1] = hand[2*i+1];
                }
                else{
                    cards_to_change[2*i] = 0;
                    cards_to_change[2*i+1] = 0;
                }
            }
            // <mandar mensaje id 13: return cards to change>
            char change_string [256];
            array_to_string(cards_to_change, change_string, 10);
            //write_server_msg(sockfd, generar_mensaje(13, change_string));
        }
        else if(message.id == 14){ // get bet
            printf("Available bets: \n");
            for(int i=0; i<message_len; i++){
                printf("%d, %s\n", bets[i]-1, bet_mapping[bets[i]-1]);
            }
            int answer;
            printf("Choose one:");
            scanf(" %d", &answer);
            char str[5];
            sprintf(str, "%d", answer);
            // <mandar mensaje id 15: return bet>
            //write_server_msg(sockfd, generar_mensaje(15, str));
        }
        else if(message.id == 16){ // error bet
            printf("Bet error\n");
        }
        else if(message.id == 17){ // ok bet
            printf("Ok bet\n");
        }
        else if(message.id == 18){ // end round
            printf("End of round\n");
        }
        else if(message.id == 19){ // show opponent cards
            // opp_hand =
            printf("Opponent's cards: \n");
            for (int i=0; i< 5; i++){
                print_card(opp_hand[2*i], opp_hand[2*i+1]);
            }
        }
        else if(message.id == 20){ // winner/loser
            if (winner == 1){
                printf("You won! :D\n");
            }
            else{
                printf("You lost :(\n");
            }
        }
        else if(message.id == 21){ // update pot
            // actualizar balance
            printf("Your new balance is %d\n", balance );
        }
        else if(message.id == 22){ // game end
            printf("Game over\n");
        }
        else if(message.id == 23){ // image
            printf("Received image\n");
        }
        else if(message.id == 24){ // error not implemented
            printf("ERROR: not implemented\n");
        }


    }

    /* Close server socket and exit. */
    //close(sockfd);
    return 0;
}
