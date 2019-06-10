#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"

// para simular que jugador esta enviando el movimiento (esto habria que ajustarlo a como lo maneja el server, al recibir el mov 
// identificar al cliente y aca se tratan distinto cada uno)
int jugador = 1; 

Board* board_init() // inicializar el tablero con las fichas de cada jugador
{
    Board* board = malloc(sizeof(Board));
    board -> cantidad_blancas = 12; // partimos con 12 fichas de cada color
    board -> cantidad_negras = 12;
    board -> con_fichas = malloc(sizeof(Cell*) * 8);
    for (int i = 0; i < 8; i++)    //  Se rellena el tablero original (solo n y b) y el que contiene las fichas (con x y o)
    {
        board -> con_fichas[i] = malloc(sizeof(Cell*) * 8);
        for (int j = 0; j < 8; j++)
        {
            if (i%2 == 0) // fila par comienza con negro
            {
                if (j%2 == 0)
                {
                    board -> con_fichas[i][j].item = 'n';
                }
                else
                {
                    if (i < 3)
                    {
                        board -> con_fichas[i][j].item = 'x';
                    }
                    else if (i > 4)
                    {
                        board -> con_fichas[i][j].item = 'o';
                    }
                    else
                    {
                        board -> con_fichas[i][j].item = 'b';
                    }
                }              
            }
            else // fila impar comienza cn blanco
            {
                if (j%2 == 0)
                {
                    if (i < 3)
                    {
                        board -> con_fichas[i][j].item = 'x';
                    }
                    else if (i > 4)
                    {
                        board -> con_fichas[i][j].item = 'o';
                    }
                    else
                    {
                        board -> con_fichas[i][j].item = 'b';
                    }
                }
                else
                {
                    board -> con_fichas[i][j].item = 'n';
                }
            }        
        }
    }
    return board;
}

int letter_to_number(char s) // funcion que transforma la letra del tablero dada por el cliente al entero de la posicion correspondiente
{
    if (s == 'A')
    {
        return 0;
    }
    else if (s == 'B')
    {
        return 1;
    }
    else if (s == 'C')
    {
        return 2;
    }
    else if (s == 'D')
    {
        return 3;
    }
    else if (s == 'E')
    {
        return 4;
    }
    else if (s == 'F')
    {
        return 5;
    }
    else if (s == 'G')
    {
        return 6;
    }
    else if (s == 'H')
    {
        return 7;
    } 
    
    return 100;
}

bool board_move(Board* board, int from_1, int from_2, int to_1, int to_2) // funcion que recibe el origen y destino del movimiento dados por el cliente y valida y realiza el movimiento
{
    int from_row = from_1 - 1; // al numero le resta 1 y lo deja como fila
    int from_col = from_2 - 1; // la letra la transforma y la deja como columna
    int to_row = to_1 - 1;
    int to_col = to_2 - 1;

    // movimientos fuera del tablero son invalidos
    if (from_row < 0 || from_row > 7 || from_col < 0 || from_col > 7 || to_row < 0 ||  to_row > 7 || to_col < 0 || to_col > 7)
    {
        return false;
    }
    // un jugador solo puede mover una ficha que es de el (jugador 1 tiene las blancas)
    if (jugador == 1 && board -> con_fichas[from_row][from_col].item != 'o' && board -> con_fichas[from_row][from_col].item != 'O')
    {
        return false;
    }
    else if (jugador == 2 && board -> con_fichas[from_row][from_col].item != 'x' && board -> con_fichas[from_row][from_col].item != 'X')
    {
        return false;
    }
    // solo se puede mover a una celda blanca que no tenga fichas
    if (board -> con_fichas[to_row][to_col].item != 'b')
    {
        return false;
    }
    // las fichas que no son reina solo se pueden mover hacia adelante
    if (jugador == 1 && board -> con_fichas[from_row][from_col].item == 'o')
    {
        if (from_row <= to_row) return false;
    }
    else if (jugador == 2 && board -> con_fichas[from_row][from_col].item == 'x')
    {
        if (from_row >= to_row) return false;
    }
    // si el movimiento es de 1 espacio, no es para comer, asi que se realiza el movimiento (ya se valido que estuviera vacio el destino)
    if (abs(from_col - to_col) == 1 && abs(from_row - to_row) == 1)
    {
        board -> con_fichas[to_row][to_col].item = board -> con_fichas[from_row][from_col].item; // en el deeestino dejamos la ficha a mover
        board -> con_fichas[from_row][from_col].item = 'b'; // en el origen dejamos la celda blanca
        return true;
    }
    // si el movimiento es de 2 espacios, es para comer a la ficha que esta entremedio
    if (abs(from_col - to_col) == 2 && abs(from_row - to_row) == 2)
    {
        // ya se valido que se moviera una ficha tuya, hay que validar que ahora haya una entremedio y que sea contrincante
        if (jugador == 1)
        {
            if (to_col > from_col) // si es en diagonal a la derecha
            {
                if (board -> con_fichas[from_row-1][from_col+1].item == 'x' || board -> con_fichas[from_row-1][from_col+1].item == 'X') // se la come
                {
                    board -> con_fichas[to_row][to_col].item = board -> con_fichas[from_row][from_col].item; // en el destino dejamos la ficha a mover
                    board -> con_fichas[from_row][from_col].item = 'b'; // en el origen dejamos la celda blanca
                    board -> con_fichas[from_row-1][from_col+1].item = 'b'; // sacamos la ficha comida
                    board -> cantidad_negras--; // restamos una negra del tablero
                    return true;
                }
                else // si hay una blanca o nada es movimiento invalido
                {
                    return false;
                }
            }
            else // si es en diagonal a la izquierda
            {
                if (board -> con_fichas[from_row-1][from_col-1].item == 'x' || board -> con_fichas[from_row-1][from_col-1].item == 'X') // se la come
                {
                    board -> con_fichas[to_row][to_col].item = board -> con_fichas[from_row][from_col].item; // en el destino dejamos la ficha a mover
                    board -> con_fichas[from_row][from_col].item = 'b'; // en el origen dejamos la celda blanca
                    board -> con_fichas[from_row-1][from_col-1].item = 'b'; // sacamos la ficha comida
                    board -> cantidad_negras--; // restamos una negra del tablero
                    return true;
                }
                else // si hay una blanca o nada es movimiento invalido
                {
                    return false;
                }
            }
        }
        else // jugador 2
        {
            if (to_col > from_col) // si es en diagonal a la derecha
            {
                if (board -> con_fichas[from_row+1][from_col+1].item == 'o' || board -> con_fichas[from_row-1][from_col+1].item == 'O') // se la come
                {
                    board -> con_fichas[to_row][to_col].item = board -> con_fichas[from_row][from_col].item; // en el destino dejamos la ficha a mover
                    board -> con_fichas[from_row][from_col].item = 'b'; // en el origen dejamos la celda blanca
                    board -> con_fichas[from_row+1][from_col+1].item = 'b'; // sacamos la ficha comida
                    board -> cantidad_blancas--; // restamos una blanca del tablero
                    return true;
                }
                else // si hay una negra o nada es movimiento invalido
                {
                    return false;
                }
            }
            else // si es en diagonal a la izquierda
            {
                if (board -> con_fichas[from_row+1][from_col-1].item == 'o' || board -> con_fichas[from_row-1][from_col-1].item == 'O') // se la come
                {
                    board -> con_fichas[to_row][to_col].item = board -> con_fichas[from_row][from_col].item; // en el destino dejamos la ficha a mover
                    board -> con_fichas[from_row][from_col].item = 'b'; // en el origen dejamos la celda blanca
                    board -> con_fichas[from_row+1][from_col-1].item = 'b'; // sacamos la ficha comida
                    board -> cantidad_blancas--; // restamos una negra del tablero
                    return true;
                }
                else // si hay una blanca o nada es movimiento invalido
                {
                    return false;
                }
            }
        }
    }
    else // si el movimiento es de mas de dos espacios lo consideramos invalido (no hay comidas doble)
    {
        return false;
    }
    
    return false;
}


/*int main() // esto era solo para probar, en este caso se mueve una blaca 2 veces, despues una negra que se la come y despues otra blanca que se come a esa negra
{
    Board* board = board_init();
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf("%c ", board -> con_fichas[i][j].item);
        }
        printf("\n");
    }
    printf("\n");
    if (board_move(board, 6,5,5,4))
    {
        printf("Se movio\n");
    }
    else
    {
        printf("Na que na que\n");
    }
    printf("\n");
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf("%c ", board -> con_fichas[i][j].item);
        }
        printf("\n");
    }
    printf("\n");
    if (board_move(board, 5,4, 4,3))
    {
        printf("Se movio\n");
    }
    else
    {
        printf("Na que na que\n");
    }
    printf("\n");
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf("%c ", board -> con_fichas[i][j].item);
        }
        printf("\n");
    }
    printf("\n");
    jugador = 2;
    if (board_move(board, 3,2, 5,4))
    {
        printf("Se movio\n");
    }
    else
    {
        printf("Na que na que\n");
    }
    printf("\n");
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf("%c ", board -> con_fichas[i][j].item);
        }
        printf("\n");
    }
    jugador = 1;
    if (board_move(board, 6,3, 4,5))
    {
        printf("Se movio\n");
    }
    else
    {
        printf("Na que na que\n");
    }
    printf("\n");
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf("%c ", board -> con_fichas[i][j].item);
        }
        printf("\n");
    }
} */