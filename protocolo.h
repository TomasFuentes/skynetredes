#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <inttypes.h>

typedef struct
{
  int id;
  int len;
  int int_content;
  int int_array[10];
  char char_array[256];
} Message;

void sendMessage(int socket, char* package){
  // Obtenemos el largo del payload para saber qué tamaño tiene el paquete y cuántos bytes debe enviar mi socket
  int payloadSize = package[1];
  send(socket, package, 2 + payloadSize, 0);
}

static bool getBit(unsigned char byte, int position) {// position can be 0-7
    return (byte >> position) & 0x1;
}

static int ChartoInt(unsigned char *buffer) {
    int valor = 1;
    int retorno = 0;
    for( int j = 0; j < 8; j++ ){
        bool aux = getBit(*buffer, j);
        if (aux){
            printf("1\n");
            retorno = retorno + valor;
        }
        else{printf("0\n");}
        valor = valor *2;
    }
    return retorno;
}

unsigned char* generar_mensaje(int id, char *texto){
  unsigned char type_id;
  unsigned char payload;
  unsigned char *content;
  unsigned char *ret;
  int len;

  if (id == 1){ //Start Connection: no requiere texto

    type_id = 0x01;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
  }

  else if (id == 2){ //Connection Established
    type_id = 0x02;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
  }

  else if (id == 3){ //Ask Nickname:
    type_id = 0x03;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
  }

  else if (id == 4){ //Return Nickname;
    type_id = 0x04;
    len = strlen(texto);
    payload = (unsigned char)len;
    content = (unsigned char *) texto;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * (len + 2));
    ret[0] = type_id;
    ret[1] = payload;
    for (int i = 2; i<len+2; i++){
      ret[i] = content[i-2];
    }
  }

  else if (id == 5){ //Opponent Found: texto corresponde al nickname;
    type_id = 0x05;
    len = strlen(texto);
    payload = (unsigned char)len;
    content = (unsigned char *) texto;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * (len + 2));
    ret[0] = type_id;
    ret[1] = payload;
    for (int i = 2; i<len+2; i++){
      ret[i] = content[i-2];
    }
  }
  else if (id == 6){ //Start Game;
    type_id = 0x06;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
  }

  else if (id ==7){ //Scores: texto es el pot del cliente
    type_id = 0x07;
    payload = 0x02;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 4);
    ret[0] = type_id;
    ret[1]= payload;
    content = (unsigned char *) texto;
    ret[2] = content[0];
    ret[3]= content[1];
  }
  else if (id == 8){// Whos First: se recibe en texto un 1 o un 2.
    type_id = 0x08;
    payload = 0x01;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 3);
    ret[0] = type_id;
    ret[1] = payload;
    content = (unsigned char *) texto;
    ret[2] = content[0];
  }

  else if(id == 9){ //Board State (64 bytes representando el tablero)
    type_id = 0x09;
    payload = 0x40;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 66);
    ret[0] = type_id;
    ret[1] = payload;
    content = (unsigned char *) texto;
    for (int i = 2; i<66; i++){
      ret[i] = content[i-2];
    }
  }

  else if (id == 10){ //Send Move: payload 4 bytes, 2 primeros casilla inicial otros 2 casilla final
    type_id = 0x0a;
    payload = 0x04;
    content = (unsigned char *) texto;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 6);
    ret[0] = type_id;
    ret[1] = payload;
    ret[2] = content[0];
    ret[3] = content[1];
    ret[4] = content[2];
    ret[5] = content[3];
  }
  else if (id == 11){// Error Move.
    type_id = 0x0b;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
  }

  else if (id == 12){ //get cards to change: texto = ""
    type_id = 0x0c;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
}

  else if (id == 13){ //End Game
    type_id = 0x0d;
    payload = 0x00;
    ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
    ret[0] = type_id;
    ret[1] = payload;
  }

  else if (id == 14){ //Win Lose -- Consideración que se envía un INT.
  type_id = 0x0e;
  payload = 0x01;
  ret = (unsigned char *)malloc(sizeof(unsigned char) * 3);
  ret[0] = type_id;
  ret[1] = payload;
  int a = atoi(texto);
  ret[2] = a;
}

else if (id == 15){//Ask New Game
  type_id = 0x0f;
  payload = 0x00;
  ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
  ret[0] = type_id;
  ret[1] = payload;
}

else if (id == 16){//Answer New Game:  OJO!
type_id = 0x10;
payload = 0x01;
ret = (unsigned char *)malloc(sizeof(unsigned char) * 3);
//content = (bool *) texto; // no sabemos si se puede hace reso
int a = atoi(texto);
ret[0] = type_id;
ret[1] = payload;
ret[2] = a;
}

else if (id == 17){// Disconnected
type_id = 0x11;
payload = 0x00;
ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
ret[0] = type_id;
ret[1] = payload;
}

else if (id == 18){//Error Bad Package
type_id = 0x12;
payload = 0x00;
ret = (unsigned char *)malloc(sizeof(unsigned char) * 2);
ret[0] = type_id;
ret[1] = payload;
}

else if (id == 19){//Send Message
  type_id = 0x13;
  len = strlen(texto);
  payload = (unsigned char)len;
  ret = (unsigned char *)malloc(sizeof(unsigned char) * (len + 2));
  ret[0] = type_id;
  ret[1] = payload;
  for (int i = 2; i<len+2; i++){
  ret[i] = content[i-2];
  }
}
else if (id == 20){// Spread Message.
  type_id = 0x14;
  len = strlen(texto);
  payload = (unsigned char)len;
  ret = (unsigned char *)malloc(sizeof(unsigned char) * (len + 2));
  ret[0] = type_id;
  ret[1] = payload;
  for (int i = 2; i<len+2; i++){
  ret[i] = content[i-2];
  }
}
return ret;
}


Message message_init(int id, char* texto){

  Message message;

  unsigned char * msg = generar_mensaje(id, texto);
  message.id = msg[0];
  message.len = msg[1];

  if(id == 4 || id == 5 || id == 9|| id == 10|| id == 19|| id == 20){
    for (int i=0; i<message.len; i++){
        message.char_array[i] = msg[i+2];
    }
  }
  else if(message.id == 7){
    message.int_array[0] = msg[2];
    message.int_array[1] = msg[3];
  }
  else if (message.id == 8 || message.id ==14|| message.id == 16){
    message.int_content = msg[2];
  }
  return message;
}

Message read_message(unsigned char id, unsigned char payload, unsigned char* msg){
  Message message;
  message.id = id;
  message.len = payload;

  if(message.id == 4 || message.id == 5 || message.id == 9|| message.id == 10|| message.id == 19|| message.id == 20){
    for (int i=0; i<message.len; i++){
        message.char_array[i] = msg[i];
    }
  }
  else if(message.id == 7){
    message.int_array[0] = msg[0];
    message.int_array[1] = msg[1];
  }
  else if (message.id == 8 || message.id ==14|| message.id == 16){
    message.int_content = msg[1];
  }
  return message;
}

// convierte el array a string y lo  guarda en  buff
void array_to_string(int* array, char * buff, int array_len){
    int n = 0;
    for (int i = 0; i < array_len-1; i++) {
        n += sprintf (&buff[n], "%d", array[i]);
        n += sprintf (&buff[n], "%s", ",");
    }
    n += sprintf (&buff[n], "%d", array[array_len-1]);
    // printf ("s = %s\n", s);
}

void imprimir_tablero(char* tablero){
      for(int i = 0; i<8; i++){
        for (int j = 0; j<8; j++){
          if (tablero[i*8 + j] == 1){ printf(" b | ");}
          if (tablero[i*8 + j] == 2){ printf(" n | ");}
          if (tablero[i*8 + j] == 3){ printf(" o | ");}
          if (tablero[i*8 + j] == 4){ printf(" O | ");}
          if (tablero[i*8 + j] == 5){ printf(" x | ");}
          if (tablero[i*8 + j] == 6){ printf(" X | ");}
          }
        printf("\n");
      }
  }

//void main() {
  //generar_mensaje(1, "");
  //generar_mensaje(2, "");
  //generar_mensaje(3, "");
  //generar_mensaje(4, "Javier");
  //generar_mensaje(5, "Tomas");
  //generar_mensaje(6, "1000");
  //generar_mensaje(7, "");
  //generar_mensaje(8, "803");
  //generar_mensaje(8, "1000");
  //generar_mensaje(8, "100000000");
  //generar_mensaje(9, "10");
  //generar_mensaje(10, "10,1,13,2,12,3,1,4,4,4");
  //generar_mensaje(11, "1");
  //generar_mensaje(12, "");
  //generar_mensaje(13, "10,1,13,4");
  //generar_mensaje(13, "10,1,13,4,8,1");
  //generar_mensaje(14, "1,2,3");
  //generar_mensaje(15, "1");
  //generar_mensaje(16, "");
  //generar_mensaje(17, "");
  //generar_mensaje(18, "");
  //generar_mensaje(19, "10,1,13,2,12,3,1,4,4,4");
  //generar_mensaje(20, "2");
  //generar_mensaje(21, "2000");
  //generar_mensaje(21, "11");
  //generar_mensaje(22, "");
  //generar_mensaje(23, "");
//}
