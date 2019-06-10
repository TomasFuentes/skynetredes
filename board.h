#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    char item;
} Cell;


typedef struct
{
    Cell** con_fichas;
    int cantidad_negras;
    int cantidad_blancas;
} Board;

Board* board_init();

bool board_move(Board* board, int from_1, int from_2, int to_1, int to_2);