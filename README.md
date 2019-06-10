## Proyecto Redes

### Integrantes:

- Felipe Berrios
- Tomas Fuentes
- Nicolás Garrido
- Vicente Vial


### Comentarios:
Se desarrolla completamente:
- Inicio y termino de conexion
- Nickname de jugadores
- Implementación de protocolos
- Implemntación de logs

No se desarrolla completamente la Logistica de movimientos, en los cuales se valida la jugada correctamente, a excepción de un salto múltiple. La condición de término del juego es únicamente el criterio de quedarse sin fichas y no se entrega puntaje, ya que no se tenía información sobre este, sin embargo, entrega la cantidad de fichas restantes de cada jugador.



##### protocolo.h: 
En este archivo se encuentra como generar los mensajes de acurdo al protocolo. La funcion 
```unsigned char* generar_mensaje(int id, char *texto)``` recibe un id y un string. Esta funcion retorna un arreglo de unsigned char(bytes) que cumpla con todas las caracteristicas del protocolo
Además se hizó una funcion ```Message read_message(unsigned char* msg)``` el cual retorna un struct llamado Mensaje. Esta función se utiliza para leer el contenido de un mensaje recibido.

##### server.c y client.c
Estos archivos representan al servidor y al cliente. Ambos tienen las siguientes funciones de envio y recibo de paquetes:

Conversión en char tablero, por cada byte, se cambia el int por las siguientes letras:

1 -> "b"

2 -> "n"

3 -> "o"

4 -> "O"

5 -> "z"

6 -> "X" 

Tanto el servidor como cliente tienen la capacidad de lectura o envio de las instrucciones de protocolo que corresponden para cada uno. (Están implementadas las 20)

La utilización de los paquetes se puede revisar en el archivo log.txt, en el que se mostrará el id del paquete enviado o recibido por el servidor. 
La manera de utilizarlos es al ir ejecutando las opciones mostradas en pantalla, con las cuales el programa automáticamente irá enviando los paquetes respectivos entrel el servidor y los clientes

Para compilar:

Abrir server: 

gcc -o server client.c

./server -i "0.0.0.0" -p 4000

Abrir dos terminales con:

gcc -o cliente1 client.c

./cliente1 -i "0.0.0.0" -p 4000

Aumentar en 1 4000 si ya se ocupo puerto




##### log.txt

Se genera un archivo log.txt en el que cada vez que se envia o recibe un paquete por parte del servidor, en él se escribe el timestamp, el id del mensaje correspondiente a una instrucción de protocolo y el contenido enviado o recibido por parte del servidor.
