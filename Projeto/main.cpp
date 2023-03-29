#include <stdio.h>
#include <stdlib.h>
#include <graphics.h>
#include <time.h>


#define NANOSECS 1000000000

#define key_W 87
#define key_S 83
#define key_D 68
#define key_A 65
#define key_SPACE 32

#define display_width 1024
#define display_heigth 960
#define tilesize 64

bool isMoving = false;
int pg = 1;

int score = 0;

enum gamestate{
	Menu,
	Gameplay,
	Dead
}gamestate;


void* LoadSprite(char* address, int width, int heigth, double scale = 1){
	void *image;
	//Cria um ponteiro do tipo void para armazenar a imagem
	if(scale <= 1){		
		//calcula o tamanho da imagem em bytes com base no tamanho em pixels
		int aux = imagesize(0, 0, width, heigth);
		//aloca memória no ponteiro com base no calculo
		image = malloc(aux);
		//imprime a imagem na tela com base na resolução base
		readimagefile(address, 0, 0, width - 1, heigth - 1);
		//recorta parte da tela e insere no ponteiro
		getimage(0, 0, width * scale - 1, heigth * scale - 1, image);
		//limpa a tela
		cleardevice();
	}else{
		int aux = imagesize(0, 0, width * scale, heigth * scale);
		image = malloc(aux);
		readimagefile(address, 0, 0, width * scale - 1, heigth * scale - 1);
		getimage(0, 0, width * scale - 1, heigth * scale - 1, image);
		cleardevice();
	}
		
	return image;		
}

/*
void* LoadAnimation(char* address, int width, int height, double scale = 1){
	void *sprites;
	if(scale <= 1){		
		int aux = imagesize(0, 0, width, heigth);
		image = malloc(aux);
		readimagefile(address, 0, 0, width * scale - 1, heigth * scale - 1);
		getimage(0, 0, width * scale - 1, heigth * scale - 1, image);
		cleardevice();
	}else{
		int aux = imagesize(0, 0, width * scale, heigth * scale);
		image = malloc(aux);
		readimagefile(address, 0, 0, width * scale - 1, heigth * scale - 1);
		getimage(0, 0, width * scale - 1, heigth * scale - 1, image);
		cleardevice();
	}
}*/


struct Player{
	int maxLife;
	int life;
	int speed;
	
	int x;
	int y;
	
	int xSize;
	int ySize;
	void *sprite;
	
	bool isMoving;
	
	char moves[40];
	int movesPending;
	
	bool onMeteor;
	bool wPress, sPress, aPress, dPress;
	
	
	Player(){
		movesPending = 0;
		maxLife = 9999;
		life = maxLife;
		onMeteor = false;	
		x = display_width / 2 - tilesize;
		y = display_heigth - tilesize;
		speed = 64;
		sprite = LoadSprite("frog.jpg", 32, 32, tilesize / 32);
	}
	
	
	bool IsPressing(int _key){
		//checa se uma tecla passa pelo parâmetro está sendo pressionada
		if(GetKeyState(_key) & 0x80)
			return true;
		else 
			return false;
	}	
	
	
	void Return(bool takeDamage){
		x = display_width / 2 - tilesize;
		y = display_heigth - tilesize * 2;	
		if(takeDamage)
			life--;
		else
			score += 150;
		if(life <= 0)
			gamestate = Dead;
	}
	void RemoveFirstMove(){
		//remove o primeiro da fila de inputs e puxa todos os outros para frente
		for(int i = 0; i < movesPending; i++){
			moves[i] =  moves[i+1];
		}
	}
	void Movement(){
		//gerencia a fila de inputs para caso o jogador aperta muitas teclas em um curto periodo de tempo todas ainda sim sejam computadas
		if(movesPending < 41){
			if(GetKeyState(key_W) & 0x80 && !wPress){
				moves[movesPending] = 'w';
				wPress = true;
				movesPending++;
			}
			/*if(GetKeyState(key_S) & 0x80 && !sPress){
				moves[movesPending] = 's';
				movesPending++;
				sPress = true;
			} */
			if(GetKeyState(key_A) & 0x80 && !aPress){
				moves[movesPending] = 'a';
				movesPending++;
				aPress = true;
			} 
			if(GetKeyState(key_D) & 0x80 && !dPress){
				moves[movesPending] = 'd';
				movesPending++;
				dPress = true;
			}	
		}
		
		
		wPress = IsPressing(key_W);
		//sPress = IsPressing(key_S);
		aPress = IsPressing(key_A);
		dPress = IsPressing(key_D);
		
		if(movesPending > 0){
			switch(moves[0]){
				
				case('w'):
					y -= tilesize;
					movesPending--;
					RemoveFirstMove();	
					score += 10;				
					break;
				/*case('s'):
					y += 64;
					movesPending--;
					RemoveFirstMove();					
					break;*/
				case('a'):
					x -= tilesize;
					movesPending--;
					RemoveFirstMove();					
					break;
				case('d'):
					x += tilesize;
					movesPending--;
					RemoveFirstMove();					
					break;
				
			}
		}	
	}
	
};

struct Meteor{
	int x;
	int y;
	int tick;
	
	int speed;
	void *sprite;
	int direction;

	
	Meteor(int _speed, char *_spriteAddress, int _direction = 0, int _layer = 0, int _x = 0){
		tick = 0;
		x = _x * 64;
		y = _layer * 64;
		speed = _speed;
		sprite = LoadSprite(_spriteAddress, 32, 32, tilesize / 32);
		direction = _direction;	
	}
	
	void Draw(){
		putimage(x, y, sprite, COPY_PUT);
	}
	
	void Movement(int tick){
		if(speed > 0){
			if(tick % (60 / speed) == 0){		
				if(direction == 1)
					x += tilesize;
				else
					x -= tilesize;
				}
		
		
		
			if(x > display_width && direction == 1)
				x = 0;
			if(x < 0 && direction == -1)
				x = display_width - tilesize;
		}
	}
	
	
	bool CheckCollisionPerTick(int playerX, int playerY, int sizeX, int sizeY){
		//checa colisões por tick do jogo (60 fps)
		for(int i = 0; i < sizeX; i++){
			if(abs(x + i * tilesize - playerX) + abs(y - playerY) < tilesize){							
				return true;
		}
				
		
		
		for(int i = 0; i < sizeY; i++){
			if(abs(x - playerX) + abs(y + i * tilesize - playerY) < tilesize){
				return true;
			}else{
				for(int i = 0; i < sizeY; i++){
					if(abs(x + i * tilesize - playerX) + abs(y + i * tilesize - playerY) < tilesize){						
						return true;						
					}
						
				}
			}				
		}
		}
			
		return false;	
	}
	
	bool CheckCollisionPerSec(int playerX, int playerY, int sizeX, int sizeY, int ticks){
		if(ticks % (60 / speed) == 0){
			for(int i = 0; i < sizeX; i++){
				if(abs(x + i * tilesize - playerX) + abs(y - playerY) < tilesize)							
					return true;
			}
		
			for(int i = 0; i < sizeY; i++){
				if(abs(x - playerX) + abs(y + i * tilesize - playerY) < tilesize){
					return true;
				}else{
					for(int i = 0; i < sizeY; i++){
						if(abs(x + i * tilesize - playerX) + abs(y + i * tilesize - playerY) < tilesize){						
							return true;						
						}
							
					}
				}				
			}
		}
			
		return false;	
	}
};

struct Enemy{
	int x;
	int y;
	int tick;
	
	int speed;
	void *sprite;
	int direction;

	
	Enemy(int _speed, char *_spriteAddress, int _direction = 0, int _layer = 0, int _x = 0){
		tick = 0;
		x = _x * tilesize;
		y = _layer * tilesize;
		speed = _speed;
		sprite = LoadSprite(_spriteAddress, 32, 32, tilesize/32);
		direction = _direction;	
	}
	
	void Draw(){
		putimage(x, y, sprite, COPY_PUT);
	}
	
	void Movement(int tick){
		if(tick % (60 / speed) == 0){		
			if(direction == 0)
				x += tilesize;
			else
				x -= tilesize;
		}
		
		if(x > display_width && direction == 0)
			x = 0;
		if(x < 0 && direction == 1)
			x = display_width - tilesize;
	
	}
	
	//checa colisão com o player e com base no tamanho do inimigo checa se o player tocou em uma de suas partes
	bool CheckCollision(int playerX, int playerY, int sizeX, int sizeY){
		for(int i = 0; i < sizeX; i++){
			if(abs(x + i * tilesize - playerX) + abs(y - playerY) < tilesize)
				return true;
		}
		
		for(int i = 0; i < sizeY; i++){
			if(abs(x - playerX) + abs(y + i * tilesize - playerY) < tilesize)
				return true;
				for(int i = 0; i < sizeY; i++){
				if(abs(x + i * tilesize - playerX) + abs(y + i * tilesize - playerY) < tilesize)
					return true;
			}
		}
		
		return false;	
			
	}
	
	
};

/*
void RenderTilemap(void* tiles[], int tileset[]){

	int *line = (int *) malloc(sizeof(int) * display_heigth / tilesize);
	int currentLine = 0;
	

	for(int i = 0; i < display_heigth / tilesize; i++){
		line[i] = tileset[i];
	}

	
	for(int i = 0; i < (display_heigth / tilesize); i++){
		
		putimage(0, i * tilesize, tiles[line[currentLine]], COPY_PUT);
			
			
		for(int j = 0; j < display_width / tilesize; j++)
			putimage(j * tilesize, i * tilesize, tiles[line[currentLine]], COPY_PUT);
				
		currentLine++;	

	}
	currentLine = 0;
}*/

Enemy AddEnemy(int speed, char *spriteAddress, int direction, int layer, int x){
	Enemy enemy(speed, spriteAddress, direction, layer, x);
	return enemy;
}

Meteor AddMeteor(int speed, char *spriteAddress, int direction, int layer, int x){
	Meteor meteor(speed, spriteAddress, direction, layer, x);
	return meteor;
}


void fase1(){
	
	Player player;
	player.x = display_width / 2 - tilesize;
	player.y = display_heigth - tilesize * 2;
	
	void *map;
	map = LoadSprite("map2.jpg", 1024, 960, 1);
	int enemyQuantity = 16;
	int enemyAdded = 0;
	
	int meteorQuantity = 5;
	int meteorAdded = 0;
	
	Enemy *enemies = (Enemy *) malloc(sizeof(Enemy) * enemyQuantity);
	Meteor *meteors = (Meteor *) malloc(sizeof(Meteor) * meteorQuantity);
	
	void *grass = LoadSprite("grass.jpg", 32, 32, tilesize / 32);
	void *water = LoadSprite("water.jpg", 32, 32, tilesize / 32);	
	void *tiles[2] = {grass, water};
	int  tileset[display_heigth / tilesize] = { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 ,0};
	
	unsigned long tickCount;
	unsigned long initTick;
	
	int ticks = 0;
	
	int timer = 60;
	
	
	initTick = GetTickCount();
	
	meteors[meteorAdded] = AddMeteor(1, "log.bmp", -1, 2, 0);
	meteorAdded++;
	meteors[meteorAdded] = AddMeteor(3, "log.bmp", 1, 3, 5);
	
	meteorAdded++;
	meteors[meteorAdded] = AddMeteor(1, "log.bmp", -1, 4, 0);
	meteorAdded++;
	meteors[meteorAdded] = AddMeteor(3, "log.bmp", 1, 5, 5);
	meteorAdded++; 
	meteors[meteorAdded] = AddMeteor(1, "log.bmp", 1, 6, 0);

	
	
	for(int i =0; i < 4; i++){
		enemies[enemyAdded] = AddEnemy(6, "car.jpg", 0, 8, i);
		enemyAdded++;
	}		
	for(int i =0; i < 4; i++){
		enemies[enemyAdded] = AddEnemy(6, "car.jpg", 0, 9, i+8);
		enemyAdded++;
	}	
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(5, "car.jpg", 0, 10, i+2);
		enemyAdded++;
	}	
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(5, "car.jpg", 0, 11, i+10);
		enemyAdded++;
	}		
	
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(4, "car.jpg", 0, 12, i);
		enemyAdded++;
	}
		
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(4, "car.jpg", 0, 8, i+15);
		enemyAdded++;
	}
		


	while(gamestate == Gameplay){

		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){

			initTick = tickCount;			
			ticks++;
			if(pg == 1) pg = 2; else pg = 1;
			//printf("Time:%i\n", timer);
			//printf("%i\n", score);
						
			setvisualpage(pg);
			cleardevice();
			putimage(0, 0, map, COPY_PUT);
			//RenderTilemap(tiles, tileset);
				
				
			player.onMeteor = false;		
			for(int i = 0; i < meteorQuantity; i++){
				meteors[i].Movement(ticks);
				meteors[i].Draw();
				
				if(meteors[i].CheckCollisionPerTick(player.x, player.y, 1, 1)){
					player.onMeteor = true;							
				}
				
				if(meteors[i].CheckCollisionPerSec(player.x + 64 * meteors[i].direction, player.y, 1, 1, ticks)){
					player.x += (meteors[i].direction * tilesize);
					player.onMeteor = true;								
				}
				
			}
			
			
			
		
			int playerLine = abs((((display_heigth - player.y)) / 64) - 15);
			
			
			if(tileset[playerLine] == 1 && player.onMeteor == false)
				player.Return(true);
			
			player.Movement();
				
			
			printf("%i", playerLine);
			
			for(int i = 0; i < enemyQuantity; i++){
				enemies[i].Draw();
				enemies[i].Movement(ticks);
				if(enemies[i].CheckCollision(player.x, player.y, 1, 1)){
					player.Return(true);
				}
					
			}				
			putimage(player.x, player.y, player.sprite, COPY_PUT);
			setactivepage(pg);
			if(ticks > 60){
				ticks = 0;
				timer--;
			} 
		}
		
		
		
	}
	
	free(water);
	free(grass);
	free(enemies);

}

void MainMenu(){
		
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	void *menu;
	
	menu = LoadSprite("menu.jpg", 1024, 960, 1);
	
	
	while(gamestate == Menu){	
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			if(pg == 1) pg = 2; else pg = 1;
			initTick = tickCount;	
			setvisualpage(pg);
			cleardevice();
			
			putimage(0, 0, menu, COPY_PUT);
					
			
			
			
			
			if(kbhit())
				gamestate = Gameplay;
				
			//printf("Time:%i\n", timer);
			//printf("%i\n", score);
						
			
			//RenderTilemap(tiles, tileset);
				
			setactivepage(pg);		
		}
	}
}
	

int main() {
		
	initwindow(display_width, display_heigth, "Gatonauta");
		
	setvisualpage(pg);
	setactivepage(pg);

	gamestate = Menu;
		
	while(true){
		switch(gamestate){
			case(Menu):
				MainMenu();
				break;
			case(Gameplay):
				fase1();
				break;
			case(Dead):
				cleardevice();
				break;
			
			
		}
		
			
	}
	
	closegraph();
	
	
	return 0;
}
