#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// estructura que representa un mazo de cartas
// cada pinta se guarda como un int array de 13 elementos
// 1 representa que la carta está, 0 que no está
typedef struct
{
	int id;
	char * nickname;
	int balance;
	int hand[10];

} Player;

// retorna un jugador con saldo inicial $1000
Player player_init(int id){

	Player player;
	
	player.id = id;
	player.nickname = "player";
	player.balance = 1000;

	return player;
}

// elimina las cartas especificadas en buffer de la mano
void remove_from_hand(Player* player, int* buff, int ncards){
	for (int i=0; i<ncards; i++){ // para cada carta en buff
		if(buff[2*i] == 0){ // ingnorar los ceros
			continue;
		}
		for (int j=0; j<5; j++){ // revisar todas las cartas de la mano
			if(player->hand[2*j] == buff[2*i] && player->hand[2*j+1] == buff[2*i+1]){ // si son iguales
				player->hand[2*j] = 0;
				player->hand[2*j+1] = 0;
			}
		}
	}
}

// void remove_hand(Player* player, int* buff){
// 	// int buff[10];
// 	for (int i=0; i<10; i++){
// 		buff[i] = player->hand[i];
// 		player->hand[i] = 0;
// 	}
// }

int player_main(){ // MAIN SOLO PARA PROBAR LAS FUNCIONES DE ESTE ARCHIVO
	
	Player player = player_init(0);
	// Deck deck = deck_init();

	player.nickname = "Pedro";

	player.balance -= 100;

	// deal_cards(&deck, player.hand, 5);

	printf("%d\n", player.balance);
	for(int i=0; i<5; i++){
		// print_card(player.hand[2*i], player.hand[2*i+1]);
	}

	// int score = get_score(player.hand);

}
