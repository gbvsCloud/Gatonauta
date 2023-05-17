#include <stdlib.h>
#include <stdio.h>
#include <graphics.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#define display_width 1680

#define display_height 1050
#define tilesize 128

#define key_W VK_UP
#define key_S VK_DOWN
#define key_D VK_RIGHT
#define key_A VK_LEFT
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
	Win,
	Infinite
};
Gamestate gs = MainMenu;

//PLAYER STATE
char playerMoves[10];
int movesPending = 0;
bool wPress, aPress, dPress, sPress;
bool playing = true;

int playerX = display_width / 2 - tilesize + 64;
int playerY = display_height - tilesize;
bool onMeteor = true;
int score = 0;
int life = 3;
//




//SCORE DISPLAY
void *scoreText;
int nDigits = 0;
int nDigitsCheck;
//

//PLAYER SPRITES
const int spriteQuantity = 12;
void* playerSprites[spriteQuantity];
int activeSprite = 0;

void** numbers;

void** playerSheet;
void** playerMask;
//


//GLOBAL ENTITIES DATA
void* GreenLeftImage;
void* GreenRightImage;

void* AlienLeftMask;
void* AlienRightMask;

void* buttonImage;
void* buttonImageMask;
void* buttonPressedImage;
void* buttonPressedMask;

void* catImage;

int alienQuantity;


int buttonQuantity;
int buttonPressedQnt = 0;


int tileset[7] = {0, 0, 0, 0, 0, 0, 0};
//

void* floorBackground;

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
	bool isPressed;
	void* sprite;
	void* pressedSprite;
	void* mask;
	void* pressedMask;
	
};

Button* buttons;

struct Alien{
	int x;
	int y;
	int direction;
	int speed;
	int xSize;
	int ySize;
	void* sprite;
	void* mask;
};

Alien* aliens;

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

Button AddButton(void* sprite, void* pressedSprite, void* mask, void* pressedMask){
	Button button;
	button.sprite = sprite;
	button.mask = mask;
	button.pressedSprite = pressedSprite;
	button.pressedMask = pressedMask;
	button.isPressed = false;
	
	do{
		button.x = (rand() % 13) * 128;
		button.y = ((rand() % 5 + 2) * 128 + 26);		
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
	
	if(alien.direction == 1)
		alien.mask = AlienRightMask;
	else
		alien.mask = AlienLeftMask;
	
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
		//checa colisões por tick do jogo (60 fps)
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
	//checa se uma tecla passa pelo parâmetro está sendo pressionada
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
	
	//mode 0 == normal mode
	//mode 1 == infinite mode
	int mode = 0;
	
	bool arrowPressed = false;
	
	while(gs == MainMenu){
		SwitchPage();
		
		
		
		printf("%i", mode);
		putimage(0, 0, menuBg, COPY_PUT);
		
		if(GetKeyState(VK_SPACE) & 0x80){
			if(mode == 0)
				gs = Level1;
			else
				gs = Infinite;
				
		} 
		if(GetKeyState(VK_LEFT) || GetKeyState(VK_RIGHT) & 0x80){
			if(mode == 0 && !arrowPressed) 
				mode = 1; 
			else if(mode == 1 && !arrowPressed) 
				mode = 0;
			
			arrowPressed = true;
			
		}else arrowPressed = false;
			
		
		setactivepage(pg);
	}
	
}

void DrawnSprite(int x, int y, void* sprite, void* mask, int type){
	if(type == 0){
		putimage(x, y, sprite, COPY_PUT);
	}
	else{
		putimage(x, y, mask, AND_PUT);
		putimage(x, y, sprite, OR_PUT);
	}
	
}

char* convertIntegerToChar(int N)
{
 
    // Count digits in number N
    int m = N;
    int digit = 0;
    while (m) {
 
        // Increment number of digits
        digit++;
 
        // Truncate the last
        // digit from the number
        m /= 10;
    }
 
    // Declare char array for result
    char* arr;
 
    // Declare duplicate char array
    char arr1[digit];
 
    // Memory allocation of array
    arr = (char*)malloc(digit);
 
    // Separating integer into digits and
    // accommodate it to character array
    int index = 0;
    while (N) {
 
        // Separate last digit from
        // the number and add ASCII
        // value of character '0' is 48
        arr1[++index] = N % 10 + '0';
 
        // Truncate the last
        // digit from the number
        N /= 10;
    }
 
    // Reverse the array for result
    int i;
    for (i = 0; i < index; i++) {
        arr[i] = arr1[index - i];
    }
 
    // Char array truncate by null
    arr[i] = '\0';
 
    // Return char array
    return (char*)arr;
}

void AlienLevel(){
	
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	int cicleCounter = 0;
	
	
	
	Cat cat = AddCat(6, 1, catImage);

		
	
	int seconds = 30;
	gameplayTimer[0] = 3;
	gameplayTimer[1] = 0;
	
	
	while(true){
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			initTick = tickCount;
			cicleCounter++;
			SwitchPage();
			putimage(0, 0, floorBackground, COPY_PUT);
			
			
			if(life <= 0 || (gameplayTimer[0] <= 0 && gameplayTimer[1] <= 0)){
				gs = Dead;
				break;
			}
			
			
			//TIMER VISUAL
			putimage(display_width / 2-128, 0, numbers[gameplayTimer[0]], COPY_PUT);
			putimage(display_width / 2, 0, numbers[gameplayTimer[1]], COPY_PUT);
			
			if(tileset[playerY / 128] == 1)
				onMeteor = false;
			
			DrawnSprite(cat.x, cat.y, cat.sprite, NULL, 0);
			//PASSAR DE FASE
			if(CheckCollision(cat.x, cat.y, 1, 1) && cat.unlocked){
				gs = (Gamestate)((int)gs + 1);
				score += seconds * 15;
				break;
				
			};
			
			
			for(int i = 0; i < buttonQuantity; i++){
				if(!buttons[i].isPressed) DrawnSprite(buttons[i].x, buttons[i].y, buttons[i].sprite, buttons[i].mask, 1);
				else DrawnSprite(buttons[i].x, buttons[i].y, buttons[i].pressedSprite, buttons[i].pressedMask, 1);	
				if(CheckCollision(buttons[i].x + 8, buttons[i].y, 1, 1) && !buttons[i].isPressed){
					buttons[i].isPressed = true;
					score += 10;
					buttonPressedQnt++;				
				}
							
			}
				

			//DRAW E UPDATE DOS ALEINS
			for(int i =0; i < alienQuantity; i++){
				if(cicleCounter % (60 / aliens[i].speed) == 0) aliens[i].x += tilesize * aliens[i].direction;
				DrawnSprite(aliens[i].x, aliens[i].y, aliens[i].sprite, aliens[i].mask, 1);
				if(CheckCollision(aliens[i].x, aliens[i].y, aliens[i].xSize, aliens[i].ySize)){
					life--;
					PlayerReturn();	
				} 
				if(aliens[i].x <= 0 + 16 || aliens[i].x >= display_width - 16){
					if(aliens[i].direction == 1)
						aliens[i].x = 0 + 8;
					else
						aliens[i].x = display_width - tilesize - 8;
				}	
			}
			PlayerInput();
			PlayerMovement();	
			
			DrawnSprite(playerX, playerY, playerSheet[activeSprite], playerMask[activeSprite], 1);
					
			char* scoreArr = convertIntegerToChar(score);
			nDigits = floor(log10(abs(score))) + 1;
			if(score > 0){
				for(int i = 0; i < nDigits; i++)
					DrawnSprite(i * 128, 128, numbers[(scoreArr[i] - '0')], NULL, 0);
			}		
			
			
			DrawnSprite(0, 0, scoreText, NULL, 0);
			
			if(IsPressing(key_ESCAPE)){
				playing = false;
				gs = MainMenu;
			};
			
			if(!onMeteor){
				PlayerReturn();
				onMeteor = true;
			} 
			
			if(buttonPressedQnt >= buttonQuantity){
				cat.unlocked = true;
			}
			
			
			
			setactivepage(pg);
			if(cicleCounter >= 60){
				seconds--;
				cicleCounter = 0;
				
					if(gameplayTimer[1] != 0 || gameplayTimer[0])gameplayTimer[1]--;
					if(gameplayTimer[1] <= -1){
						if(gameplayTimer[0] > 0) gameplayTimer[0]--;
						gameplayTimer[1] = 9;
					}
			
				
			} 
		}
	}
	PlayerReturn();
}



int main(){
	
	initwindow(display_width, display_height, "");
	
	playerSheet = LoadSpriteSheet(4, "gato.bmp", 512, 128);
	playerMask = LoadSpriteSheet(4, "gatoMask.bmp", 512, 128);
	
	numbers = LoadSpriteSheet(10, "numbers.jpg", 1280, 128);
	
	srand(time(NULL));
		
	int x =0;
	
	
	
	alienQuantity = 4;
	GreenLeftImage = LoadSprite("GreenLeft.bmp", 128, 128, 1);
	GreenRightImage = LoadSprite("GreenRight.bmp", 128, 128, 1);
	AlienLeftMask = LoadSprite("AlienLeftMask.bmp", 128, 128, 1);
	AlienRightMask = LoadSprite("AlienRightMask.bmp", 128, 128, 1);
	
	buttonImage = LoadSprite("Button.bmp", 128, 128);
	buttonImageMask = LoadSprite("ButtonMask.bmp", 128, 128);
	buttonPressedImage = LoadSprite("ButtonPressed.bmp", 128, 128);
	buttonPressedMask = LoadSprite("ButtonPressedMask.bmp", 128, 128);
	
	
	
	catImage = LoadSprite("gato.jpg", 128, 128);
	floorBackground = LoadSprite("fundo.bmp", 1680, 1050, 1, 0, 0, 1680, 1050);
	scoreText = LoadSprite("score.bmp", 256, 128, 1, 0, 0, 256, 128);
	
	while(playing){
		SwitchPage();			
		switch(gs){
			case(MainMenu):				
				MainMenuState();			
				break;		
			case(Level1):
				
				//ALIENS CONFIG															
				aliens = NULL;	
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(0, 5, 1, 2, GreenRightImage, 1, 1);
				aliens[1] = AddAlien(13, 2, -1, 3, GreenLeftImage, 1, 1);
				aliens[2] = AddAlien(0, 4, 1, 5, GreenRightImage, 1, 1);
				aliens[3] = AddAlien(13, 6, -1, 4, GreenLeftImage, 1, 1);
				//
				
				//BUTTONS CONFING 
				buttonQuantity = 3;	
				buttons = NULL;
				buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
				for(int i = 0; i < buttonQuantity; i++)
					buttons[i] = AddButton(buttonImage, buttonPressedImage, buttonImageMask, buttonPressedMask);
				//
				
				AlienLevel();
				break;
			case(Level2):
				//ALIENS CONFIG
				aliens = NULL;	
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(0, 5, 1, 5, GreenRightImage, 1, 1);
				aliens[1] = AddAlien(13, 2, -1, 3, GreenLeftImage, 1, 1);
				aliens[2] = AddAlien(0, 4, 1, 5, GreenRightImage, 1, 1);
				aliens[3] = AddAlien(13, 6, 1, 4, GreenRightImage, 1, 1);
				//
				
				//BUTTONS CONFIG
				buttonQuantity = 6;	
				buttons = NULL;
				buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
				for(int i = 0; i < buttonQuantity; i++)
					buttons[i] = AddButton(buttonImage, buttonPressedImage, buttonImageMask, buttonPressedMask);
				//
				
				AlienLevel();
				break;
			case(Level3):
				//ALIENS CONFIG
				aliens = NULL;	
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(0, 5, 1, 10, GreenRightImage, 1, 1);
				aliens[1] = AddAlien(13, 2, -1, 3, GreenLeftImage, 1, 1);
				aliens[2] = AddAlien(0, 4, -1, 5, GreenLeftImage, 1, 1);
				aliens[3] = AddAlien(13, 6, 1, 4, GreenRightImage, 1, 1);

				//
				
				//BUTTONS CONFIG
				buttonQuantity = 9;	
				buttons = NULL;
				buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
				for(int i = 0; i < buttonQuantity; i++)
					buttons[i] = AddButton(buttonImage, buttonPressedImage, buttonImageMask, buttonPressedMask);
				//
				
				AlienLevel();
				break;
			case(Dead):
				break;
			case(Infinite):
				//ALIENS CONFIG															
				aliens = NULL;	
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(0, 5, 1, 2, GreenLeftImage, 1, 1);
				aliens[1] = AddAlien(13, 2, -1, 3, GreenLeftImage, 1, 1);
				aliens[2] = AddAlien(0, 4, 1, 5, GreenLeftImage, 1, 1);
				aliens[3] = AddAlien(13, 6, 1, 4, GreenLeftImage, 1, 1);
				//
				
				//BUTTONS CONFING 
				buttonQuantity = 3;	
				buttons = NULL;
				buttons = (Button *) realloc(buttons, sizeof(Button) * buttonQuantity);
				for(int i = 0; i < buttonQuantity; i++)
					buttons[i] = AddButton(buttonImage, buttonPressedImage, buttonImageMask, buttonPressedMask);
				
				//
				
				
				break;
		}
					
		setactivepage(pg);
	};
	
	closegraph();
	return 0;
}
