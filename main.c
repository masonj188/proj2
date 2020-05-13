/* DUNGEONS AND DEADLINES */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "aes.h"
#include <string.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

int playing = 0;

char* title =
"\n\n\n\n\n"
"______ _   _ _   _ _____  _____ _____ _   _  _____    ___   _   _______  ______ _____  ___ ______ _     _____ _   _  _____ _____ \n"
"|  _  \\ | | | \\ | |  __ \\|  ___|  _  | \\ | |/  ___|  / _ \\ | \\ | |  _  \\ |  _  \\  ___|/ _ \\|  _  \\ |   |_   _| \\ | ||  ___/  ___| \n"
"| | | | | | |  \\| | |  \\/| |__ | | | |  \\| |\\ `--.  / /_\\ \\|  \\| | | | | | | | | |__ / /_\\ \\ | | | |     | | |  \\| || |__ \\ `--.  \n"
"| | | | | | | . ` | | __ |  __|| | | | . ` | `--. \\ |  _  || . ` | | | | | | | |  __||  _  | | | | |     | | | . ` ||  __| `--. \\ \n"
"| |/ /| |_| | |\\  | |_\\ \\| |___\\ \\_/ / |\\  |/\\__/ / | | | || |\\  | |/ /  | |/ /| |___| | | | |/ /| |_____| |_| |\\  || |___/\\__/ / \n"
"|___/  \\____\\_| \\_/\\____/\\____/ \\___/\\_| \\_/\\____/  \\_| |_/\\_| \\_/___/   |___/ \\____/\\_| |_/___/ \\_____/\\___/\\_| \\_/\\____/\\____/  "
"\n\n\n\n\n";
                                                                                                                                 
char* GO = 
"\n\n"
" _____   ___  ___  ___ _____   _____  _   _ ___________ \n"
"|  __ \\ / _ \\ |  \\/  ||  ___| |  _  || | | |  ___| ___ \\\n"
"| |  \\// /_\\ \\| .  . || |__   | | | || | | | |__ | |_/ /\n"
"| | __ |  _  || |\\/| ||  __|  | | | || | | |  __||    / \n"
"| |_\\ \\| | | || |  | || |___  \\ \\_/ /\\ \\_/ / |___| |\\ \\ \n"
" \\____/\\_| |_/\\_|  |_/\\____/   \\___/  \\___/\\____/\\_| \\_|\n"
"\n\n\n\n\n";

typedef struct Output {
	char CharName[20];
	unsigned char *EncryptedBuffer;
	char IV[16];
} output;


/******* ITEM DETAILS ******* 
Gold - any amount up to int_max
Weapons - Initially empty array, contains up to 10 different weeapons, each weapon mapped to an int
Armor - Armor level, 1-5
Pots - Initially empty array of Potion type, contains the type (maps from int to type), and the quanitity of potions available

*****************************/

char* wepTypes[10] = {"Bronze Sword", "Iron Dagger", "Steel Scimitar", "Elven Bow", "Flaming Greatsword", "Mithril Short Sword", "Adamantine Claws", "Rune Halberd", "Dragon Greataxe", "Sword of Destiny"};
int wepCosts[10] = {0,100,200,300,400,500,750,1000,1500,2500};
char* armorTypes[6] = {"Bronze Armor", "Steel Armor", "Mithril Armor", "Adamantine Armor", "Rune Armor", "Dragon Armor"};
int armorCosts[6] = {0,200,400,750,1500,3000};
char* potTypes[5] = {"", "Health Potion", "Attack Potion", "Defense Potion", "Gold Potion"};
int potCosts[5] = {0,100,50,50,50};

typedef struct Status {
	int AttackPot;
	int DefensePot;
	int GoldPot;
} status;


typedef struct Potions {
	int type;
	int quantity;
} potion;

typedef struct Backpack {
	int gold;
	int weapon;
	int armor;
	int pots[5];
} backpack;

typedef struct Character {
	char name[20];
	char pass[20];
	int hp;
	backpack bag;
	status curStatus;
} character;


int checkNameExists(char* name) {
	int exists = 1;
	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(13339);
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n");
		return -1;
    } 
    send(sock, name, strlen(name), 0); 
    printf("Sent the name.\n");
	recv(sock, &exists, sizeof(int), 0);
	close(sock);

	return exists;

}

character receiveAndDecrypt(char* name, char* key) {
	unsigned char* receiveBuf = (unsigned char*)malloc(132);
	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
	character failCharacter = {"", "", 0, {0, 0, 0, {0, 0, 0, 0}}};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return failCharacter; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(13338); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return failCharacter; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n");
		return failCharacter;
    } 
    send(sock, name, strlen(name), 0);
	printf("Sent the name\n");
	recv(sock, receiveBuf, 132, 0); 
    printf("Received the Buffer.\n"); 
	close(sock);
	

	// Hash the key

	unsigned char md[32];
	SHA256_CTX sctx;
	SHA256_Init(&sctx);
	SHA256_Update(&sctx, key, strlen(key));
	SHA256_Final(md, &sctx);

	// Decrypt the player buffer

	unsigned char* decryptBuf = (unsigned char*)malloc(96);
	char IV[16];
	memcpy(IV, receiveBuf+116, 16);
	memcpy(decryptBuf, receiveBuf+20, 96);
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, md, IV);
	AES_CBC_decrypt_buffer(&ctx, decryptBuf, 96);


	// Put data from buffer into character

	character receivedPlayer;
	memcpy(&receivedPlayer, decryptBuf, sizeof(character));


	free(decryptBuf);
	free(receiveBuf);

	return receivedPlayer;
}

void encryptAndSend(character player, char* key) {

	unsigned char md[32];
	SHA256_CTX sctx;
	SHA256_Init(&sctx);
	SHA256_Update(&sctx, key, strlen(key));
	SHA256_Final(md, &sctx);

	output toSend = {"0", "0", "AAAAAAAAAAAAAAA"};
	memcpy(toSend.CharName, player.name, 20);
	RAND_load_file("/dev/random", 32);
	RAND_bytes(toSend.IV, 16); //16 random bytes
	unsigned char *buffer = (char*)malloc(96);
	//	toSend.EncryptedBuffer = (char*)malloc(sizeof(player));
	memcpy(buffer, (const void*)&player, sizeof(player));
	memset(buffer+sizeof(player), 48, 96-sizeof(player));
	// send

	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, md, toSend.IV);
	AES_CBC_encrypt_buffer(&ctx, buffer, 96);
	toSend.EncryptedBuffer = buffer;

	unsigned char* sendBuffer = (char*)malloc(132);

	//printf("Size of SendBuffer: %d\n", sizeof(sendBuffer));
	memcpy(sendBuffer, toSend.CharName, 20);
	memcpy(sendBuffer+20, toSend.EncryptedBuffer, 96);
	memcpy(sendBuffer+116, &toSend.IV, 16);




	int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(13337); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n");
		return;
    } 
    send(sock, sendBuffer, 132, 0); 
    printf("Sent the buffer.\n"); 
	close(sock);
    //valread = read( sock , buffer, 1024); 
    //printf("%s\n",buffer ); 


	free(buffer);
	free(sendBuffer);
}

void printDeathAndExit() {
	printf("You've been slain... \n\n");
	printf("%s", GO);

	exit(1);
}

void clearStatus(character *player) {
	player->curStatus.AttackPot = 0;
	player->curStatus.DefensePot = 0;
	player->curStatus.GoldPot = 0;
}

void usePot(character *player, int type) {
	switch(type) {
		case 1:
			if(player->hp != 100) {
				player->hp = 100;
				player->bag.pots[type]--;
				printf("Used a %s!\n", potTypes[type]);
			} else {
				printf("HP already at 100!\n");
			}
			break;
		case 2:
			if(player->curStatus.AttackPot != 1 && player->bag.pots[type] > 0) {
				player->curStatus.AttackPot = 1;
				player->bag.pots[type]--;
				printf("Used an %s!\n", potTypes[type]);
			} else {
				printf("Already used potion or not enough potions remaining!\n");
			}
			break;
		case 3:
			if(player->curStatus.DefensePot != 1 && player->bag.pots[type] > 0) {
				player->curStatus.DefensePot = 1;
				player->bag.pots[type]--;
				printf("Used a %s!\n", potTypes[type]);
			} else {
				printf("Already used potion or not enough potions remaining!\n");
			}
			break;
		case 4:
			if(player->curStatus.GoldPot != 1 && player->bag.pots[type] > 0) {
				player->curStatus.GoldPot = 1;
				player->bag.pots[type]--;
				printf("Used a %s!\n", potTypes[type]);
			} else {
				printf("Already used potion or not enough potions remaining!\n");
			}
			break;
		default:
			break;
	}
}

void printPotStatus(character player) {
	printf("Attack Potion Active: %d\n", player.curStatus.AttackPot);
	printf("Defense Potion Active: %d\n", player.curStatus.DefensePot);
	printf("Gold Potion Active: %d\n\n", player.curStatus.GoldPot);
}

void printWeapon(character player) {
	printf("Equipped Weapon: %s\nAttack Modifier: +%d\n\n", wepTypes[player.bag.weapon], player.bag.weapon);
}

void printArmor(character player) {
	printf("Equipped Armor: %s\nDefense Modifier: +%d\n\n", armorTypes[player.bag.armor], player.bag.armor);
}

void printPots(character player) {
	for(int i = 1; i < sizeof(player.bag.pots)/sizeof(player.bag.pots[0]); i++) {
		printf("%ss: %d\n", potTypes[i], player.bag.pots[i]);
	}
	printf("\n");
}

void printGold(character player) {
	printf("Gold: %d\n", player.bag.gold);
}

void printHP(character player) {
	printf("HP: %d\n", player.hp);
}


int getAP(character player) {
	int ap = 0;
	ap += player.bag.weapon;
	if(player.curStatus.AttackPot == 1) {
		ap += 2;
	}
	return ap;
}

int getDP(character player) {
	int dp = 0;
	dp += player.bag.armor;
	if(player.curStatus.DefensePot == 1) {
		dp += 2;
	}
	return dp;
}

void getCharacter(character player) {
	printf("Character Status:\n");
	printHP(player);
	printGold(player);
	printWeapon(player);
	printArmor(player);
	printPots(player);
	printPotStatus(player);	
}

void completeDungeon(character *player, int difficulty){
	srand(time(NULL));
	float p_success = (float)rand()/(float)RAND_MAX;
	int healthLost = 0;
	int goldGain = 0;
	switch (difficulty){
	case 0:
		if(p_success+(0.025*getAP(*player)) >= 0.3){
			goldGain = 10 + (player->curStatus.GoldPot*100);
			player->bag.gold += goldGain;
			healthLost = 5 - (2*getDP(*player));
			if(healthLost < 0){
				healthLost = 0;
			}

			if(healthLost > 0){
				player->hp -= healthLost;
			}

			printf("You successfully completed your adventure! You lost %d health and gained %d gold.\n\n",healthLost,goldGain);
		}
		else{
			healthLost = 10 - (2*getDP(*player));

			if(healthLost < 0){
				healthLost = 0;
			}

			if(healthLost > 0){
				player->hp -= healthLost;
			}

			printf("You failed your adventure! You lost %d health.\n\n",healthLost);
		}
		clearStatus(player);
		break;
	case 1:
		if(p_success+(0.04*getAP(*player)) >= 0.5){
			goldGain = 100 + (player->curStatus.GoldPot*100);
			player->bag.gold += goldGain;
			healthLost = 25 - (5*getDP(*player));

			if(healthLost < 0){
				healthLost = 0;
			}

			if(healthLost > 0){
				player->hp -= healthLost;
			}

			printf("You successfully completed your adventure! You lost %d health and gained %d gold.\n\n",healthLost,goldGain);
		}
		else{
			healthLost = 50 - (5*getDP(*player));

			if(healthLost < 0){	
				healthLost = 0;
			}

			if(healthLost > 0){
				player->hp -= healthLost;
			}

			printf("You failed your adventure! You lost %d health.\n\n",healthLost);
		}
		clearStatus(player);
		break;
	case 2:
		if(p_success+(0.06*getAP(*player)) >= 0.9){
			goldGain = 250 + (player->curStatus.GoldPot*100);
			player->bag.gold += goldGain;
			healthLost = 50 - (10*getDP(*player));

			if(healthLost < 0){
				healthLost = 0;
			}

			if(healthLost > 0){
				player->hp -= healthLost;
			}

			printf("You successfully completed your adventure! You lost %d health and gained %d gold.\n\n",healthLost,goldGain);
		}
		else{
			healthLost = 100 - (10*getDP(*player));

			if(healthLost < 0){
				healthLost = 0;
			}

			if(healthLost > 0){
				player->hp -= healthLost;
			}

			printf("You failed your adventure! You lost %d health.\n\n",healthLost);
			break;
		}
	default:
		break;
	}
}

int checkForGold(character player, int amount){
	if(player.bag.gold >= amount){
		return 1;
	}
	else{
		return 0;
	}
}

int checkForBetterWep(character player, int index){
	if(player.bag.weapon < index){
		return 1;
	}
	else{
		return 0;
	}
	
}

int checkForBetterArmor(character player, int index){
	if(player.bag.armor < index){
		return 1;
	}
	else{
		return 0;
	}
}

character login(){
	char name[20];
	char pass[20];
	character null = {"", "", 0, {0, 0, 0, {0, 0, 0, 0}}};
	printf("Enter Player name:\n");
	fgets(name, 20, stdin);
	strtok(name, "\n");
	//name[19] = '\0';
	//check player name exists

	printf("Enter password: ");
	fgets(pass, 20, stdin);
	strtok(pass, "\n");
	if(strlen(pass) > 19) {
		pass[19] = '\0';
	}
	character returnedChar = receiveAndDecrypt(name, pass);
	//check if correct password
	if(strcmp(returnedChar.name, name) == 0) {
		playing = 1;
	}
	else{
		printf("Failed login attempt\n\n");
	}
	return returnedChar;
}

character createNewCharacter(){
	char input[20];
	char pass[20];
	character new = {"", "", 100, {100, 0, 0, {0, 0, 0, 0}}};
	int taken = 1;
	printf("Enter Player name:\n");
	fgets(input, 20, stdin);
	strtok(input, "\n");
	//check if name already taken
	taken = checkNameExists(input);

	if(taken) {
		printf("Character name already taken!\n\n");
		return new;
	}
	memcpy(new.name, input, 20);
		
	printf("Enter a password: ");
	fgets(pass, 20, stdin);
	strtok(pass, "\n");

	if(strlen(pass) > 19) {
		pass[19] = '\0';
	}

	memcpy(new.pass, pass, 20);
	
	playing = 1;
	return new;
}

int main() {
	//login(); stub
	puts(title);
	puts(" ");
	character player;
	char input[8];

	while(!playing){
		printf("Welcome to Dungeons & Deadlines!\n");
		printf("1. Log in to an existing character\n");
		printf("2. Create a new character\n");

		fgets(input, 8, stdin);

		switch (atoi(input)) {
			case 1:
				player = login();

				break;
			case 2:
				player = createNewCharacter();
				break;
			default:
				printf("Invalid response!\n");
				break;
		}
	}

	while(playing){
		printf("What would you like to do next?\n");
		printf("1. Go on an adventure\n");
		printf("2. Visit the shop\n");
		printf("3. Use an item\n");
		printf("4. Check character status\n");
		printf("5. Go to sleep (save and log out)\n");
		fgets(input, 8, stdin);
		printf("\n");

		switch (atoi(input)) {
			case 1:
				printf("Choose your adventure difficulty:\n");
				printf("1. Easy - %.0lf%% chance of success\n", 70.0+(2.5*getAP(player)));
				printf("2. Medium - %.0lf%% chance of success\n", 50.0+(4*getAP(player)));
				printf("3. Hard - %.0lf%% chance of success\n", 10.0+(6*getAP(player)));
				printf("4. Return to previous menu\n");
				
				fgets(input, 8, stdin);
				printf("\n");

				switch (atoi(input)) {
					case 1:
						completeDungeon(&player,0);
						if(player.hp < 1) {
							printDeathAndExit();
						}
						break;
					case 2:
						completeDungeon(&player,1);
						if(player.hp < 1) {
							printDeathAndExit();
						}
						break;
					case 3:
						completeDungeon(&player,2);
						if(player.hp < 1) {
							printDeathAndExit();
						}
						break;
					case 4:
						//do nothing
						break;
					default:
						printf("Invalid input!\n");
						break;
				}
				break;

			case 2:
				printf("What would you like to buy?\n");
				printf("1. Weapons\n");
				printf("2. Armor\n");
				printf("3. Potions\n");
				printf("4. Return to previous menu\n");
				
				fgets(input, 8, stdin);
				printf("\n");

				switch (atoi(input)) {
					case 1:
						//show weapons
						for(int i = 1; i < sizeof(wepCosts)/sizeof(int); i++) {
							printf("%d. Buy %s - %d gold\n",i,wepTypes[i],wepCosts[i]);
						}
						printf("%d. Return to main menu\n",sizeof(wepCosts)/sizeof(int));

						fgets(input, 8, stdin);
						
						switch (atoi(input)) {
							case 1:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 2:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 3:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 4:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 5:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 6:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 7:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 8:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent or better weapon equipped.\n");
								}
								break;
							case 9:
								if(checkForGold(player,wepCosts[atoi(input)]) && checkForBetterWep(player,atoi(input))){
									player.bag.gold -= wepCosts[atoi(input)];
									player.bag.weapon = atoi(input);
									printf("You purchased a(n) %s\n",wepTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that weapon, or currently have an equivalent weapon equipped.\n");
								}
								break;
							case sizeof(wepCosts)/sizeof(int):
								break;
							default:
								printf("Invalid input!\n");
								break;
						}
						break;
					case 2:
						//show armor
						for(int i = 1; i < sizeof(armorCosts)/sizeof(int); i++) {
							printf("%d. Buy %s - %d gold\n",i,armorTypes[i],armorCosts[i]);
						}
						printf("%d. Return to main menu\n",sizeof(armorCosts)/sizeof(int));

						fgets(input, 8, stdin);

						switch (atoi(input)) {
							case 1:
								if(checkForGold(player,armorCosts[atoi(input)]) && checkForBetterArmor(player,atoi(input))){
									player.bag.gold -= armorCosts[atoi(input)];
									player.bag.armor = atoi(input);
									printf("You purchased %s\n",armorTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that armor, or currently have equivalent or better armor already equipped.\n");
								}
								break;
							case 2:
								if(checkForGold(player,armorCosts[atoi(input)]) && checkForBetterArmor(player,atoi(input))){
									player.bag.gold -= armorCosts[atoi(input)];
									player.bag.armor = atoi(input);
									printf("You purchased %s\n",armorTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that armor, or currently have equivalent or better armor already equipped.\n");
								}
								break;
							case 3:
								if(checkForGold(player,armorCosts[atoi(input)]) && checkForBetterArmor(player,atoi(input))){
									player.bag.gold -= armorCosts[atoi(input)];
									player.bag.armor = atoi(input);
									printf("You purchased %s\n",armorTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that armor, or currently have equivalent or better armor already equipped.\n");
								}
								break;
							case 4:
								if(checkForGold(player,armorCosts[atoi(input)]) && checkForBetterArmor(player,atoi(input))){
									player.bag.gold -= armorCosts[atoi(input)];
									player.bag.armor = atoi(input);
									printf("You purchased %s\n",armorTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that armor, or currently have equivalent or better armor already equipped.\n");
								}
								break;
							case 5:
								if(checkForGold(player,armorCosts[atoi(input)]) && checkForBetterArmor(player,atoi(input))){
									player.bag.gold -= armorCosts[atoi(input)];
									player.bag.armor = atoi(input);
									printf("You purchased %s\n",armorTypes[atoi(input)]);
								}
								else{
									printf("You either do not have enough gold to purchase that armor, or currently have equivalent armor already equipped.\n");
								}
								break;
							case sizeof(armorCosts)/sizeof(int):
								break;
							default:
								printf("Invalid input!\n");
								break;
							
						}
						break;
					case 3: 
						//show potions
						for(int i = 1; i < sizeof(potCosts)/sizeof(int); i++) {
							printf("%d. Buy %s - %d gold\n",i,potTypes[i],potCosts[i]);
						}
						printf("%d. Return to main menu\n",sizeof(potCosts)/sizeof(int));
						
						fgets(input, 8, stdin);
						switch (atoi(input)) {
							case 1:
								if(checkForGold(player,potCosts[atoi(input)])){
									player.bag.gold -= potCosts[atoi(input)];
									player.bag.pots[atoi(input)]++;
									printf("You purchased a(n) %s\n",potTypes[atoi(input)]);
								}
								else{
									printf("You do not have enough gold to purchase that potion.\n");
								}
								break;
							case 2:
								if(checkForGold(player,potCosts[atoi(input)])){
									player.bag.gold -= potCosts[atoi(input)];
									player.bag.pots[atoi(input)]++;
									printf("You purchased a(n) %s\n",potTypes[atoi(input)]);
								}
								else{
									printf("You do not have enough gold to purchase that potion.\n");
								}
								break;
							case 3:
								if(checkForGold(player,potCosts[atoi(input)])){
									player.bag.gold -= potCosts[atoi(input)];
									player.bag.pots[atoi(input)]++;
									printf("You purchased a(n) %s\n",potTypes[atoi(input)]);
								}
								else{
									printf("You do not have enough gold to purchase that potion.\n");
								}
								break;
							case 4:
								if(checkForGold(player,potCosts[atoi(input)])){
									player.bag.gold -= potCosts[atoi(input)];
									player.bag.pots[atoi(input)]++;
									printf("You purchased a(n) %s\n",potTypes[atoi(input)]);
								}
								else{
									printf("You do not have enough gold to purchase that potion.\n");
								}
								break;
							case sizeof(potCosts)/sizeof(int):
								break;
							default:
								printf("Invalid input!\n");
								break;		
						}
						break;
					case 4:
						//do nothing
						break;
				}
				break;
			case 3:
				//open potions list and prompt dialog
				for(int j = 1; j < sizeof(potTypes)/sizeof(potTypes[0]); j++) {
					printf("%d. %s\n", j, potTypes[j]);
				}
				printf("%d. Return to Main Menu\n", sizeof(potTypes)/sizeof(potTypes[0]));
				fgets(input, 8, stdin);
				switch(atoi(input)) {
					case 1:
						usePot(&player, atoi(input));
						break;
					case 2:
						usePot(&player, atoi(input));
						break;
					case 3:
						usePot(&player, atoi(input));
						break;
					case 4:
						usePot(&player, atoi(input));
						break;
					default:
						break;
				}
				
				break;
			case 4:
				getCharacter(player);
				break;
			case 5:
				//logout(player)
				encryptAndSend(player, player.pass);
				exit(0);
			default:
				printf("Invalid input!\n");
				break;
			break;
		}
		//logout() i.e. send of player data
		//return EXIT_SUCCESS;
	}
	return EXIT_SUCCESS;
}
