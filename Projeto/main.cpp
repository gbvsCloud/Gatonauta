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

#define display_width 1408
#define display_heigth 960
#define tilesize 32

bool isMoving = false;
int pg = 1;

int score = 0;

void* LoadSprite(char* address, int width, int heigth, double scale = 1){
	void *image;
	
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
		
	return image;		
}




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
	
	bool wPress, sPress, aPress, dPress;
	
	
	Player(){
		movesPending = 0;
		maxLife = 1;
		life = 1;
		x = display_width / 2 - tilesize;
		y = display_heigth - tilesize;
		speed = 64;
		sprite = LoadSprite("frog.jpg", 32, 32, tilesize / 32);
	}
	
	
	bool IsPressing(int _key){
		if(GetKeyState(_key) & 0x80)
			return true;
		else 
			return false;
	}	
	
	
	void RemoveFirstMove(){
		for(int i = 0; i < movesPending; i++){
			moves[i] =  moves[i+1];
		}
	}
	void Movement(){
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

struct Log{
	int x;
	int y;
	int tick;
	
	int speed;
	void *sprite;
	int direction;

	
	Log(int _speed, char *_spriteAddress, int _direction = 0, int _layer = 0, int _x = 0){
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
	
	bool CheckCollision(int playerX, int playerY, int sizeX, int sizeY, int ticks){
		if(ticks % (60 / speed) == 0){
			for(int i = 0; i < sizeX; i++){
			if(abs(x + i * tilesize - playerX) + abs(y - playerY) < tilesize){							
				return true;
			}
				
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
}

Enemy AddEnemy(int speed, char *spriteAddress, int direction, int layer, int x){
	Enemy enemy(speed, spriteAddress, direction, layer, x);
	return enemy;
}


void fase1(){
	
	Player player;
	player.x = display_width / 2 - 32;
	player.y = display_heigth - 32;
	
	int enemyQuantity = 17;
	int enemyAdded = 0;
	Enemy *enemies = (Enemy *) malloc(sizeof(Enemy) * enemyQuantity);
	
	void *grass = LoadSprite("grass.jpg", 32, 32, tilesize / 32);
	void *water = LoadSprite("water.jpg", 32, 32, tilesize / 32);	
	void *tiles[2] = {grass, water};
	int  tileset[display_heigth / tilesize] = {0, 1, 1, 1, 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	
	unsigned long tickCount;
	unsigned long initTick;
	
	int ticks = 0;
	
	int timer = 60;
	
	
	initTick = GetTickCount();
	
	Log log (1, "log.jpg", 0, 2, 0);
	
	for(int i =0; i < 4; i++){
		enemies[enemyAdded] = AddEnemy(6, "car.jpg", 0, 9, i);
		enemyAdded++;
	}		
	for(int i =0; i < 4; i++){
		enemies[enemyAdded] = AddEnemy(6, "car.jpg", 0, 9, i+8);
		enemyAdded++;
	}	
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(5, "car.jpg", 0, 8, i+2);
		enemyAdded++;
	}	
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(5, "car.jpg", 0, 8, i+10);
		enemyAdded++;
	}		
	
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(4, "car.jpg", 0, 7, i);
		enemyAdded++;
	}
		
	for(int i =0; i < 2; i++){
		enemies[enemyAdded] = AddEnemy(4, "car.jpg", 0, 7, i+15);
		enemyAdded++;
	}
		

	enemies[16] = AddEnemy(1, "car.jpg", 0, 6, 0);
	
	while(true){
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			if(pg == 1) pg = 2; else pg = 1;
			initTick = tickCount;			
			ticks++;
			
			//printf("Time:%i\n", timer);
			printf("%i\n", score);
						
			setvisualpage(pg);
			cleardevice();
			RenderTilemap(tiles, tileset);
			
			printf("a");		
			log.Movement(ticks);
			log.Draw();
			
			
			if(log.CheckCollision(player.x + tilesize, player.y, 3, 1, ticks)){
				player.x += tilesize - (log.direction * 2 * tilesize);
			}
			
			player.Movement();
			
			for(int i = 0; i < enemyQuantity; i++){
				enemies[i].Draw();
				enemies[i].Movement(ticks);
				if(enemies[i].CheckCollision(player.x, player.y, 1, 1)){
					player.x = display_width / 2 - tilesize;
					player.y = display_heigth - tilesize;	
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

}
	

int main() {
		
	initwindow(display_width, display_heigth, "Gatonauta");
		
	setvisualpage(pg);
	setactivepage(pg);


		
	while(true){
	
		fase1();
			
	}
	
	closegraph();
	
	
	return 0;
}
