#include <stdlib.h>
#include <stdio.h>
#include <graphics.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#define display_width 1680

#define display_height 1050
#define tilesize 128

#define key_W 87
#define key_S 83
#define key_D 68
#define key_A 65
#define key_SPACE 32
#define key_ESCAPE 27


//WIDTH DIFFERENCE 64
//HEIGHT DIFFENRENCE 4

int pg = 1;

enum Gamestate{
	MainMenu,
	Level1,
	Level2,
	Level3,
	Dead,
	Win	
};
Gamestate gs = MainMenu;
char playerMoves[10];
int movesPending = 0;
bool wPress, aPress, dPress, sPress;
bool playing = true;

int playerX = display_width / 2 - tilesize + 64;
int playerY = display_height - tilesize;
bool onMeteor = true;
int score = 0;

//PLAYER SPRITES

const int spriteQuantity = 12;
void* playerSprites[spriteQuantity];
int activeSprite = 0;

void** numbers;

void** playerSheet;


int gameplayTimer[2];

struct Cat{
	int x;
	int y;
	bool unlocked;
	void* sprite;
};

struct Button{
	int x;
	int y;
	void* sprite;
	
};

struct Alien{
	int x;
	int y;
	int direction;
	int speed;
	int xSize;
	int ySize;
	void* sprite;
};
struct Meteor{
	int x;
	int y;
	int direction;
	int speed;
	int xSize;
	int ySize;
	void* sprite;
};

void* LoadSprite(const char* spriteAddress, int spriteWidth, int spriteHeight, int scale = 1, int init_width = 0, int init_height = 0,
	int final_width = tilesize, int final_height = tilesize){	
	
	void* image;
	
	int aux = imagesize(0, 0, spriteWidth, spriteHeight);
	image = malloc(aux);
	readimagefile(spriteAddress, 0, 0, spriteWidth, spriteHeight);
	getimage(init_width, init_height, final_width - 1, final_height - 1, image);
			
	/*if(scale <= 1){
		int aux = imagesize(0, 0, spriteWidth, spriteHeight);
		image = malloc(aux);
		readimagefile(spriteAddress, 0, 0, spriteWidth, spriteHeight);
		getimage(init_width, init_height, final_width - 1, final_height - 1, image);
		
	}else{
		int aux = imagesize(0, 0, spriteWidth * scale, spriteHeight * scale);
		image = malloc(aux);
		readimagefile(spriteAddress, 0, 0, spriteWidth * scale, spriteHeight * scale);
		getimage(init_width, init_height, final_width * scale - 1, final_height * scale - 1, image);	
	}*/
	cleardevice();	
	return image;
}

void** LoadSpriteSheet(int spriteQuantity, const char* spriteAddress, int spriteWidth, int spriteHeight){	
	void** image;		
	
	image = (void**)malloc(spriteQuantity * sizeof(void*));
	readimagefile(spriteAddress, 0, 0, spriteWidth, spriteHeight);
	
	int aux = imagesize(0, 0, 128, 128);
	for(int i = 0; i < spriteQuantity; i++){
		image[i] = malloc(aux);
		getimage(i * 128, 0, (i + 1) * 128 - 1, 128 - 1, image[i]);
	}
			
	cleardevice();	
	return image;
}

Button AddButton(void* sprite){
	Button button;
	button.sprite = sprite;
	do{
		button.x = (rand() % 13) * 128;
		button.y = (rand() % 6 + 1) * 128;		
	}while((button.x == display_width / 2 + 64 && button.y == display_height - 128) || (button.x == display_width / 2 + 64 && button.y == 128));
	
	return button;
}

Cat AddCat(int x, int y, void* sprite){
	Cat cat;
	cat.x = x * 128;
	cat.y = y * 128 + 26;
	cat.sprite = sprite;
	cat.unlocked = false;
	
	return cat;
}

Alien AddAlien(int x, int y, int direction, int speed, void* sprite, int xSize, int ySize){
	Alien alien;
	alien.x = x * 128;
	alien.y = y * 128 + 26;
	
	alien.direction = direction;
	alien.speed = speed;
	alien.sprite = sprite;
	alien.xSize = xSize;
	alien.ySize = ySize;
	
	return alien;
}

Meteor AddMeteor(int x, int y, int direction, int speed, void* sprite, int xSize, int ySize){
	Meteor meteor;
	meteor.x = x * 128;
	meteor.y = y * 128 + 26;
	meteor.direction = direction;
	meteor.speed = speed;
	meteor.sprite = sprite;
	meteor.xSize = xSize;
	meteor.ySize = ySize;
	
	return meteor;
}



bool CheckCollision(int objectX, int objectY, int sizeX, int sizeY){
		//checa colis�es por tick do jogo (60 fps)
	for(int i = 0; i < sizeX; i++){
		if(abs(objectX + i * tilesize - playerX) + abs(objectY - playerY) < tilesize)						
			return true;
	}
				
		
		
	for(int i = 0; i < sizeY; i++){
		if(abs(objectX - playerX) + abs(objectY + i * tilesize - playerY) < tilesize){
			return true;
		}else{
			for(int i = 0; i < sizeY; i++){
				if(abs(objectX + i * tilesize - playerX) + abs(objectY + i * tilesize - playerY) < tilesize)						
					return true;															
			}
		}				
	}
		
			
	return false;	
}


/*bool CheckCollision(int objectX, int objectY, int sizeX, int sizeY){
	

	
	for(int i = 0; i < sizeX; i++){
		if(abs((objectX + i * tilesize) - playerX) + abs(objectY - playerY) < tilesize)
			return true;
	}
		
	for(int i = 0; i < sizeY; i++){
		if(abs(objectX - playerX) + abs(objectY + i * tilesize - playerY) < tilesize)
			return true;
		for(int j = 0; j < sizeX; j++){
			if((abs(objectX + j * tilesize - playerX) + abs(objectY + i * tilesize - playerY)) < tilesize)
				return true;
		}	
	}
		
	return false;
}*/

void SwitchPage(){
	if(pg == 1) pg = 2; else pg = 1;
	
	setvisualpage(pg);
	cleardevice();

}

void RemoveFirstMove(){
		//remove o primeiro da fila de inputs e puxa todos os outros para frente
	for(int i = 0; i < movesPending; i++){
		playerMoves[i] =  playerMoves[i+1];
	}
}

void PlayerInput(){
		//gerencia a fila de inputs para caso o jogador aperta muitas teclas em um curto periodo de tempo todas ainda sim sejam computadas
	if(movesPending < 41){
		if(GetKeyState(key_W) & 0x80 && !wPress){
			playerMoves[movesPending] = 'w';
			wPress = true;
			movesPending++;
		}
		if(GetKeyState(key_S) & 0x80 && !sPress){
			playerMoves[movesPending] = 's';
			sPress = true;
			movesPending++;
		}
		if(GetKeyState(key_A) & 0x80 && !aPress){
			playerMoves[movesPending] = 'a';
			movesPending++;
			aPress = true;
		} 
		if(GetKeyState(key_D) & 0x80 && !dPress){
			playerMoves[movesPending] = 'd';
			movesPending++;
			dPress = true;
		}	
	}
}

void PlayerReturn(){
	playerX = display_width / 2 - tilesize + 64;
	playerY = display_height - tilesize;
	activeSprite = 0;
}

bool IsPressing(int _key){
	//checa se uma tecla passa pelo par�metro est� sendo pressionada
	if(GetKeyState(_key) & 0x80)
		return true;
	else 
		return false;
}

void PlayerMovement(){
	if(movesPending > 0){
		switch(playerMoves[0]){	
			case('w'):
				activeSprite = 0;
				playerY -= tilesize;
				movesPending--;
				RemoveFirstMove();	
				
				//score += 10;				
				break;				
			case('a'):
				activeSprite = 2;
				if(playerX > 0 + tilesize/32 + 32) playerX -= tilesize;				
				movesPending--;
				RemoveFirstMove();								
			break;
			
			case('s'):
				activeSprite = 3;
				if(playerY < display_height - tilesize)playerY += tilesize;
				movesPending--;
				RemoveFirstMove();	
				
				//score += 10;				
				break;		
			
			case('d'):
				activeSprite = 1;
				if(playerX < display_width - tilesize - 32) playerX += tilesize;				
				movesPending--;
				RemoveFirstMove();									
			break;
				
		}
	}
	wPress = IsPressing(key_W);
	aPress = IsPressing(key_A);
	dPress = IsPressing(key_D);
	sPress = IsPressing(key_S);
}

void MainMenuState(){
	void* menuBg = LoadSprite("menu.jpg", 1024, 960, 1, 0, 0, 1024, 960);
	
	while(gs == MainMenu){
		SwitchPage();
		
		putimage(0, 0, menuBg, COPY_PUT);
		
		if(kbhit()) gs = Level1;
		
		
		setactivepage(pg);
	}
	
}

void DrawSprite(int x, int y, void* sprite){
	putimage(x, y, sprite, COPY_PUT);
}

void Level3State(){
	
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	int cicleCounter = 0;
	
	void* background = LoadSprite("map2.jpg", 1600, 900, 1, 0, 0, 1600, 900);
	
	Alien* aliens;
	int alienQuantity = 4;
	aliens = NULL;	
	void* alienImage = LoadSprite("alien.jpg", 128, 128, 1);	
	aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
	aliens[0] = AddAlien(0, 5, 1, 2, alienImage, 1, 1);
	aliens[1] = AddAlien(13, 2, -1, 3, alienImage, 1, 1);
	aliens[2] = AddAlien(0, 4, 1, 5, alienImage, 1, 1);
	aliens[3] = AddAlien(13, 6, 1, 4, alienImage, 1, 1);
	
	void* catImage = LoadSprite("gato.jpg", 128, 128);
	Cat cat = AddCat(6, 1, catImage);
	void* buttonImage = LoadSprite("button.jpg", 128, 128);
	
	
	Button* buttons;
	int buttonQuantity = 6;
	int buttonPressed = 0;
	buttons = NULL;
	buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
	for(int i = 0; i < buttonQuantity; i++)
		buttons[i] = AddButton(buttonImage);
	
	
	/*
	Meteor* meteors;
	int meteorQuantity = 4;
	void* meteorImage = LoadSprite("car.jpg", 128, 128, 1);
	meteors = NULL;
	
	meteors = (Meteor *) realloc(meteors, sizeof(Meteor) * meteorQuantity);
	meteors[0] = AddMeteor(0, 3, 1, 2, meteorImage, 1, 1);
	meteors[1] = AddMeteor(12, 2, -1, 3, meteorImage, 1, 1);
	meteors[2] = AddMeteor(0, 1, 1, 4, meteorImage, 1, 1);
	meteors[3] = AddMeteor(12, 0, -1, 1, meteorImage, 1, 1);*/
	
	int tileset[7] = {0, 0, 0, 0, 0, 0, 0};
	
	
	gameplayTimer[0] = 6;
	gameplayTimer[1] = 0;
	
	int nDigits = floor(log10(abs(139))) + 1;
	

	while(gs == Level3){
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			initTick = tickCount;
			cicleCounter++;
			SwitchPage();
			putimage(0, 0, background, COPY_PUT);
			
			
			//TIMER VISUAL
			putimage(display_width / 2-128, 0, numbers[gameplayTimer[0]], COPY_PUT);
			putimage(display_width / 2, 0, numbers[gameplayTimer[1]], COPY_PUT);
			
			if(tileset[playerY / 128] == 1)
				onMeteor = false;
			
			DrawSprite(cat.x, cat.y, cat.sprite);
			//PASSAR DE FASE
			if(CheckCollision(cat.x, cat.y, 1, 1) && cat.unlocked){
				gs = Level1;
				PlayerReturn();
			};
			
			
			for(int i = 0; i < buttonQuantity; i++){
				DrawSprite(buttons[i].x, buttons[i].y, buttons[i].sprite);	
				if(CheckCollision(buttons[i].x, buttons[i].y, 1, 1)){
					buttons[i].y = - 128;	
					buttonPressed++;				
				}
							
			}
				
			/*
			for(int i =0; i < meteorQuantity; i++){
				
				if(cicleCounter % (60 / meteors[i].speed) == 0){
					meteors[i].x += tilesize * meteors[i].direction;
					if(meteors[i].direction == 1){
						if(CheckCollision(meteors[i].x - tilesize, meteors[i].y, meteors[i].xSize, meteors[i].ySize))
							playerX += tilesize * meteors[i].direction;
					}else{
						if(CheckCollision(meteors[i].x + tilesize, meteors[i].y, meteors[i].xSize, meteors[i].ySize))
							playerX += tilesize * meteors[i].direction;
					}
					
					
				} 
				DrawSprite(meteors[i].x, meteors[i].y, meteors[i].sprite);
				if(CheckCollision(meteors[i].x, meteors[i].y, meteors[i].xSize, meteors[i].ySize)) onMeteor = true;
				
				if(meteors[i].x <= 0 + 16 || meteors[i].x >= display_width - 16){
					if(meteors[i].direction == 1)
						meteors[i].x = 0 + 8
						;
					else
						meteors[i].x = display_width - tilesize - 8;
				}	
			}*/
			
			//DRAW E UPDATE DOS ALEINS
			for(int i =0; i < alienQuantity; i++){
				if(cicleCounter % (60 / aliens[i].speed) == 0) aliens[i].x += tilesize * aliens[i].direction;
				DrawSprite(aliens[i].x, aliens[i].y, aliens[i].sprite);
				if(CheckCollision(aliens[i].x, aliens[i].y, aliens[i].xSize, aliens[i].ySize)) PlayerReturn(); ;
				if(aliens[i].x <= 0 + 16 || aliens[i].x >= display_width - 16){
					if(aliens[i].direction == 1)
						aliens[i].x = 0 + 8;
					else
						aliens[i].x = display_width - tilesize - 8;
				}	
			}
			PlayerInput();
			PlayerMovement();	
			DrawSprite(playerX, playerY, playerSheet[activeSprite]);
			
			if(IsPressing(key_ESCAPE)){
				playing = false;
				gs = MainMenu;
			};
			
			if(!onMeteor){
				PlayerReturn();
				onMeteor = true;
			} 
			
			if(buttonPressed >= buttonQuantity){
				cat.unlocked = true;
			}
			
			setactivepage(pg);
			if(cicleCounter >= 60){
				cicleCounter = 0;
				
					if(gameplayTimer[1] != 0 || gameplayTimer[0])gameplayTimer[1]--;
					if(gameplayTimer[1] <= -1){
						if(gameplayTimer[0] > 0) gameplayTimer[0]--;
						gameplayTimer[1] = 9;
					}
			
				
			} 
		}
	}
	free(aliens);
	free(background);
	free(catImage);
	free(alienImage);
	free(buttonImage);
	//free(meteors);
}

void Level2State(){
	
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	int cicleCounter = 0;
	
	void* background = LoadSprite("map2.jpg", 1600, 900, 1, 0, 0, 1600, 900);
	
	Alien* aliens;
	int alienQuantity = 4;
	aliens = NULL;	
	void* alienImage = LoadSprite("alien.jpg", 128, 128, 1);	
	aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
	aliens[0] = AddAlien(0, 5, 1, 2, alienImage, 1, 1);
	aliens[1] = AddAlien(13, 2, -1, 3, alienImage, 1, 1);
	aliens[2] = AddAlien(0, 4, 1, 5, alienImage, 1, 1);
	aliens[3] = AddAlien(13, 6, 1, 4, alienImage, 1, 1);
	
	void* catImage = LoadSprite("gato.jpg", 128, 128);
	Cat cat = AddCat(6, 1, catImage);
	void* buttonImage = LoadSprite("button.jpg", 128, 128);
	
	
	Button* buttons;
	int buttonQuantity = 4;
	int buttonPressed = 0;
	buttons = NULL;
	buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
	for(int i = 0; i < buttonQuantity; i++)
		buttons[i] = AddButton(buttonImage);
	
	
	/*
	Meteor* meteors;
	int meteorQuantity = 4;
	void* meteorImage = LoadSprite("car.jpg", 128, 128, 1);
	meteors = NULL;
	
	meteors = (Meteor *) realloc(meteors, sizeof(Meteor) * meteorQuantity);
	meteors[0] = AddMeteor(0, 3, 1, 2, meteorImage, 1, 1);
	meteors[1] = AddMeteor(12, 2, -1, 3, meteorImage, 1, 1);
	meteors[2] = AddMeteor(0, 1, 1, 4, meteorImage, 1, 1);
	meteors[3] = AddMeteor(12, 0, -1, 1, meteorImage, 1, 1);*/
	
	int tileset[7] = {0, 0, 0, 0, 0, 0, 0};
	
	
	gameplayTimer[0] = 6;
	gameplayTimer[1] = 0;
	
	int nDigits = floor(log10(abs(139))) + 1;
	
	while(gs == Level2){
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			initTick = tickCount;
			cicleCounter++;
			SwitchPage();
			putimage(0, 0, background, COPY_PUT);
			
			
			//TIMER VISUAL
			putimage(display_width / 2-128, 0, numbers[gameplayTimer[0]], COPY_PUT);
			putimage(display_width / 2, 0, numbers[gameplayTimer[1]], COPY_PUT);
			
			if(tileset[playerY / 128] == 1)
				onMeteor = false;
			
			DrawSprite(cat.x, cat.y, cat.sprite);
			//PASSAR DE FASE
			if(CheckCollision(cat.x, cat.y, 1, 1) && cat.unlocked){
				gs = Level3;
				PlayerReturn();
			};
			
			
			for(int i = 0; i < buttonQuantity; i++){
				DrawSprite(buttons[i].x, buttons[i].y, buttons[i].sprite);	
				if(CheckCollision(buttons[i].x, buttons[i].y, 1, 1)){
					buttons[i].y = - 128;	
					buttonPressed++;				
				}
							
			}
				
			/*
			for(int i =0; i < meteorQuantity; i++){
				
				if(cicleCounter % (60 / meteors[i].speed) == 0){
					meteors[i].x += tilesize * meteors[i].direction;
					if(meteors[i].direction == 1){
						if(CheckCollision(meteors[i].x - tilesize, meteors[i].y, meteors[i].xSize, meteors[i].ySize))
							playerX += tilesize * meteors[i].direction;
					}else{
						if(CheckCollision(meteors[i].x + tilesize, meteors[i].y, meteors[i].xSize, meteors[i].ySize))
							playerX += tilesize * meteors[i].direction;
					}
					
					
				} 
				DrawSprite(meteors[i].x, meteors[i].y, meteors[i].sprite);
				if(CheckCollision(meteors[i].x, meteors[i].y, meteors[i].xSize, meteors[i].ySize)) onMeteor = true;
				
				if(meteors[i].x <= 0 + 16 || meteors[i].x >= display_width - 16){
					if(meteors[i].direction == 1)
						meteors[i].x = 0 + 8
						;
					else
						meteors[i].x = display_width - tilesize - 8;
				}	
			}*/
			
			//DRAW E UPDATE DOS ALEINS
			for(int i =0; i < alienQuantity; i++){
				if(cicleCounter % (60 / aliens[i].speed) == 0) aliens[i].x += tilesize * aliens[i].direction;
				DrawSprite(aliens[i].x, aliens[i].y, aliens[i].sprite);
				if(CheckCollision(aliens[i].x, aliens[i].y, aliens[i].xSize, aliens[i].ySize)) PlayerReturn(); ;
				if(aliens[i].x <= 0 + 16 || aliens[i].x >= display_width - 16){
					if(aliens[i].direction == 1)
						aliens[i].x = 0 + 8;
					else
						aliens[i].x = display_width - tilesize - 8;
				}	
			}
			PlayerInput();
			PlayerMovement();	
			DrawSprite(playerX, playerY, playerSheet[activeSprite]);
			
			if(IsPressing(key_ESCAPE)){
				playing = false;
				gs = MainMenu;
			};
			
			if(!onMeteor){
				PlayerReturn();
				onMeteor = true;
			} 
			
			if(buttonPressed >= buttonQuantity){
				cat.unlocked = true;
			}
			
			setactivepage(pg);
			if(cicleCounter >= 60){
				cicleCounter = 0;
				
					if(gameplayTimer[1] != 0 || gameplayTimer[0])gameplayTimer[1]--;
					if(gameplayTimer[1] <= -1){
						if(gameplayTimer[0] > 0) gameplayTimer[0]--;
						gameplayTimer[1] = 9;
					}
			
				
			} 
		}
	}
	free(aliens);
	free(background);
	free(catImage);
	free(alienImage);
	free(buttonImage);
	//free(meteors);
}


void Level1State(){
	
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	int cicleCounter = 0;
	
	void* background = LoadSprite("map2.jpg", 1600, 900, 1, 0, 0, 1600, 900);
	
	
	Alien* aliens;
	int alienQuantity = 4;
	aliens = NULL;	
	void* alienImage = LoadSprite("Green_Left.bmp", 128, 128, 1);	
	aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
	aliens[0] = AddAlien(0, 5, 1, 2, alienImage, 1, 1);
	aliens[1] = AddAlien(13, 2, -1, 3, alienImage, 1, 1);
	aliens[2] = AddAlien(0, 4, 1, 5, alienImage, 1, 1);
	aliens[3] = AddAlien(13, 6, 1, 4, alienImage, 1, 1);
	
	void* catImage = LoadSprite("gato.jpg", 128, 128);
	Cat cat = AddCat(6, 1, catImage);
	void* buttonImage = LoadSprite("button.jpg", 128, 128);
	
	
	Button* buttons;
	int buttonQuantity = 2;
	int buttonPressed = 0;
	buttons = NULL;
	buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
	for(int i = 0; i < buttonQuantity; i++)
		buttons[i] = AddButton(buttonImage);
	
	
	/*
	Meteor* meteors;
	int meteorQuantity = 4;
	void* meteorImage = LoadSprite("car.jpg", 128, 128, 1);
	meteors = NULL;
	
	meteors = (Meteor *) realloc(meteors, sizeof(Meteor) * meteorQuantity);
	meteors[0] = AddMeteor(0, 3, 1, 2, meteorImage, 1, 1);
	meteors[1] = AddMeteor(12, 2, -1, 3, meteorImage, 1, 1);
	meteors[2] = AddMeteor(0, 1, 1, 4, meteorImage, 1, 1);
	meteors[3] = AddMeteor(12, 0, -1, 1, meteorImage, 1, 1);*/
	
	int tileset[7] = {0, 0, 0, 0, 0, 0, 0};
	
	score = 0;
	
	gameplayTimer[0] = 6;
	gameplayTimer[1] = 0;
	
	int nDigits = floor(log10(abs(139))) + 1;
	while(gs == Level1){
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			initTick = tickCount;
			cicleCounter++;
			SwitchPage();
			putimage(0, 0, background, COPY_PUT);
			
		
			
			//TIMER VISUAL
			putimage(display_width / 2-128, 0, numbers[gameplayTimer[0]], COPY_PUT);
			putimage(display_width / 2, 0, numbers[gameplayTimer[1]], COPY_PUT);
			
			if(tileset[playerY / 128] == 1)
				onMeteor = false;
			
			DrawSprite(cat.x, cat.y, cat.sprite);
			//PASSAR DE FASE
			if(CheckCollision(cat.x, cat.y, 1, 1) && cat.unlocked){
				gs = Level2;
				
			};
			
			
			for(int i = 0; i < buttonQuantity; i++){
				DrawSprite(buttons[i].x, buttons[i].y, buttons[i].sprite);	
				if(CheckCollision(buttons[i].x, buttons[i].y, 1, 1)){
					buttons[i].y = - 128;	
					buttonPressed++;				
				}
							
			}
				
			/*
			for(int i =0; i < meteorQuantity; i++){
				
				if(cicleCounter % (60 / meteors[i].speed) == 0){
					meteors[i].x += tilesize * meteors[i].direction;
					if(meteors[i].direction == 1){
						if(CheckCollision(meteors[i].x - tilesize, meteors[i].y, meteors[i].xSize, meteors[i].ySize))
							playerX += tilesize * meteors[i].direction;
					}else{
						if(CheckCollision(meteors[i].x + tilesize, meteors[i].y, meteors[i].xSize, meteors[i].ySize))
							playerX += tilesize * meteors[i].direction;
					}
					
					
				} 
				DrawSprite(meteors[i].x, meteors[i].y, meteors[i].sprite);
				if(CheckCollision(meteors[i].x, meteors[i].y, meteors[i].xSize, meteors[i].ySize)) onMeteor = true;
				
				if(meteors[i].x <= 0 + 16 || meteors[i].x >= display_width - 16){
					if(meteors[i].direction == 1)
						meteors[i].x = 0 + 8
						;
					else
						meteors[i].x = display_width - tilesize - 8;
				}	
			}*/
			
			//DRAW E UPDATE DOS ALEINS
			for(int i =0; i < alienQuantity; i++){
				if(cicleCounter % (60 / aliens[i].speed) == 0) aliens[i].x += tilesize * aliens[i].direction;
				DrawSprite(aliens[i].x, aliens[i].y, aliens[i].sprite);
				if(CheckCollision(aliens[i].x, aliens[i].y, aliens[i].xSize, aliens[i].ySize)) PlayerReturn(); ;
				if(aliens[i].x <= 0 + 16 || aliens[i].x >= display_width - 16){
					if(aliens[i].direction == 1)
						aliens[i].x = 0 + 8;
					else
						aliens[i].x = display_width - tilesize - 8;
				}	
			}
			PlayerInput();
			PlayerMovement();	
			DrawSprite(playerX, playerY, playerSheet[activeSprite]);
			
			if(IsPressing(key_ESCAPE)){
				playing = false;
				gs = MainMenu;
			};
			
			if(!onMeteor){
				PlayerReturn();
				onMeteor = true;
			} 
			
			if(buttonPressed >= buttonQuantity){
				cat.unlocked = true;
			}
			
			setactivepage(pg);
			if(cicleCounter >= 60){
				cicleCounter = 0;
				
					if(gameplayTimer[1] != 0 || gameplayTimer[0])gameplayTimer[1]--;
					if(gameplayTimer[1] <= -1){
						if(gameplayTimer[0] > 0) gameplayTimer[0]--;
						gameplayTimer[1] = 9;
					}
			
				
			} 
		}
	}
	free(aliens);
	free(background);
	free(catImage);
	free(alienImage);
	free(buttons);
	free(buttonImage);
	PlayerReturn();
	//free(meteors);
}



int main(){
	
	initwindow(display_width, display_height, "");
	
	playerSheet = LoadSpriteSheet(4, "gato.bmp", 512, 128);
	
	numbers = LoadSpriteSheet(10, "numbers.jpg", 1280, 128);
	
	srand(time(NULL));
		
	int x =0;
	while(playing){
		SwitchPage();			
		switch(gs){
			case(MainMenu):	
				MainMenuState();			
				break;		
			case(Level1):
				Level1State();
				break;
			case(Level2):
				Level2State();
				break;
			case(Level3):
				Level3State();
				break;
		}
					
		setactivepage(pg);
	};
	
	closegraph();
	return 0;
}