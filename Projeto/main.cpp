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

bool isMoving = false;
int pg = 1;




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
	
	Player(){
		maxLife = 1;
		life = 1;
		x = display_width / 2 - 64;
		y = display_heigth - 64;
		speed = 64;
		sprite = LoadSprite("frog.jpg", 32, 32, 2);
	}
	
	
	bool IsPressing(){
		if(GetKeyState(key_W) & 0x80 || GetKeyState(key_S) & 0x80 || GetKeyState(key_A) & 0x80 || GetKeyState(key_D) & 0x80)
			return true;
		else 
			return false;
	}	
	
	
	void Movement(){
		if(GetKeyState(key_W) & 0x80 && !isMoving){
			y -= speed;
		} 
		else if(GetKeyState(key_S) & 0x80 && !isMoving){
			y += speed;
		} 
		else if(GetKeyState(key_A) & 0x80 && !isMoving){
			x -= speed;
		} 
		else if(GetKeyState(key_D) & 0x80 && !isMoving){
			x += speed;
		}
			isMoving = IsPressing();
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
		x = _x * 64;
		y = _layer * 64;
		speed = _speed;
		sprite = LoadSprite(_spriteAddress, 32, 32, 2);
		direction = _direction;	
	}
	
	void Draw(){
		putimage(x, y, sprite, COPY_PUT);
	}
	
	void Movement(int tick){
		if(tick % (60 / speed) == 0){		
			if(direction == 0)
				x += 64;
			else
				x -= 64;
		}
		
		if(x > display_width && direction == 0)
			x = 0;
		if(x < 0 && direction == 1)
			x = display_width - 64;
	
	}
	
	//checa colisão com o player e com base no tamanho do inimigo checa se o player tocou em uma de suas partes
	bool CheckCollision(int playerX, int playerY, int sizeX, int sizeY){
		for(int i = 0; i < sizeX; i++){
			if(abs(x + i * 64 - playerX) + abs(y - playerY) < 64)
				return true;
		}
		
		for(int i = 0; i < sizeY; i++){
			if(abs(x - playerX) + abs(y + i * 64 - playerY) < 64)
				return true;
				for(int i = 0; i < sizeY; i++){
				if(abs(x + i * 64 - playerX) + abs(y + i * 64 - playerY) < 64)
					return true;
			}
		}
			
			
	}
	
	
};


void RenderTilemap(void* tiles[]){

	int *column = (int *) malloc(sizeof(int) * 15);
	int currentColumn = 0;
	

	for(int i = 0; i < 15; i++){
		column[i] = 0;
	}
	
	for(int i = 1; i < 3; i++){
		column[i] = 1;
	}
	
	for(int i = 0; i < (display_heigth / 64); i++){
		
		putimage(0, i * 64, tiles[column[currentColumn]], COPY_PUT);
			
			
		for(int j = 0; j < display_width / 64; j++)
			putimage(j * 64, i * 64, tiles[column[currentColumn]], COPY_PUT);
				
		currentColumn++;	

	}
	currentColumn = 0;
}

Enemy AddEnemy(int speed, char *spriteAddress, int direction, int layer, int x){
	Enemy enemy(speed, spriteAddress, direction, layer, x);
	return enemy;
}

struct fase{
	
	
};
	

int main() {
		
	initwindow(display_width, display_heigth, "Gatonauta");
	
	int enemyQuantity = 17;
	Enemy *enemies = (Enemy *) malloc(sizeof(Enemy) * enemyQuantity);
	
	
	
	for(int i =0; i < 4; i++)
		enemies[i] = AddEnemy(6, "car.jpg", 0, 9, i);
	for(int i =0; i < 4; i++)
		enemies[i+4] = AddEnemy(6, "car.jpg", 0, 9, i+8);
		
	for(int i =0; i < 2; i++)
		enemies[i+8] = AddEnemy(5, "car.jpg", 0, 8, i+2);
	for(int i =0; i < 2; i++)
		enemies[i+10] = AddEnemy(5, "car.jpg", 0, 8, i+10);
	
	for(int i =0; i < 2; i++)
		enemies[i+12] = AddEnemy(4, "car.jpg", 0, 7, i);
	for(int i =0; i < 2; i++)
		enemies[i+14] = AddEnemy(4, "car.jpg", 0, 7, i+15);

	enemies[16] = AddEnemy(60, "car.jpg", 0, 6, 0);
	
	void *grass = LoadSprite("grass.jpg", 32, 32, 2);
	void *water = LoadSprite("water.jpg", 32, 32, 2);	
	
	
	
	
	void *tiles[2] = {grass, water};
	
	setvisualpage(pg);
	setactivepage(pg);
	
	unsigned long tickCount;
	unsigned long initTick;
	initTick = GetTickCount();
	

	//variaveis do player

	int x = 0;
	int y = 0;
	
	int ticks = 0;
	
	Player player;
	
		
	while(true){
		tickCount = GetTickCount();
			
		if(initTick - tickCount > 1000/60){
			if(pg == 1) pg = 2; else pg = 1;
			initTick = tickCount;
			
			ticks++;
			
			//if(ticks > 60)	
			//	printf("\nDistance:%i", abs(enemies[0].x - player.x) + abs(enemies[0].y - player.y));

			setvisualpage(pg);
			cleardevice();
			RenderTilemap(tiles);
			

			
			
			player.Movement();
			for(int i = 0; i < enemyQuantity; i++){
				enemies[i].Draw();
				enemies[i].Movement(ticks);
				if(enemies[i].CheckCollision(player.x, player.y, 1, 1)){
					player.x = display_width / 2 - 64;
					player.y = display_heigth - 64;
				}
					
			}	
			

				
			putimage(player.x, player.y, player.sprite, COPY_PUT);
			setactivepage(pg);
			if(ticks > 60) ticks = 0;
		}
			
	}
	
	closegraph();
	
	
	return 0;
}
