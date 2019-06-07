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




