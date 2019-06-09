## Proyecto Redes

### Integrantes:

- Felipe Berrios
- Tomas Fuentes
- Nicolás Garrido
- Vicente Vial


### Comentarios:


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

Por ahora se reciben posiciones como (i,j) númericas, falta cambiar i a letra.("A","B",..)




Para compilar:

Abrir server: 

gcc -o server client.c

./server -i "0.0.0.0" -p 4000

Abrir dos terminales con:

gcc -o cliente1 client.c

./cliente1 -i "0.0.0.0" -p 4000

