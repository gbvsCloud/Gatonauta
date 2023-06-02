#include <stdlib.h>
#include <stdio.h>
#include <graphics.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#define display_width 1680

#define display_height 1050
#define tilesize 128


//Apelidos para facilitar na hora de visualizar 
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
	Level4,
	Level5,
	Win,
	Dead,	
	Infinite
};
Gamestate gs = MainMenu;

POINT P;

//PLAYER STATE

//lista com todos os inputs de movimento do usuario
char playerMoves[10];
//quantidade de movimentos ainda pendentes
int movesPending = 0;
//variavel responsável por dizer o estado de pressionamento de cada tecla
bool wPress, aPress, dPress, sPress, spacePress;

bool playing = true;

int playerX = display_width / 2 - tilesize + 64;
int playerY = display_height - tilesize;
bool onMeteor = true;
int score = 0;
int life = 7;
//

//SCORE DISPLAY
void *scoreText;
void *scoreMask;

int nDigits = 0;
int nDigitsCheck;
//


//BUTTON IMAGE

void* playBtn;
void* exitBtn;

//PLAYER SPRITES
const int spriteQuantity = 12;
void* playerSprites[spriteQuantity];
int activeSprite = 0;

void** numbers;
void** numbersMask;

void** playerSheet;
void** playerMask;
//


//GLOBAL ENTITIES DATA
void* GreenLeftImage;
void* GreenRightImage;

void* OrangeLeftImage;
void* OrangeRightImage;

void* PurpleLeftImage;
void* PurpleRightImage;

void* SmallMeteorImage;
void* GreatMeteorImage;

void* SmallMeteorMask;
void* GreatMeteorMask;

void* AlienLeftMask;
void* AlienRightMask;

void* buttonImage;
void* buttonImageMask;
void* buttonPressedImage;
void* buttonPressedMask;

void* catImage;
void* catMask;

int alienQuantity;
int meteorQuantity;

int buttonQuantity;
int buttonPressedQnt = 0;


int tileset[7] = {0, 0, 0, 0, 0, 0, 0};
//

void* floorBackground;
void* spaceBackground;
void* victoryScreen;
void* loseScreen;

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
	void* mask;
};

Meteor* meteors;

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

void MeowSound(){
	int x = rand() % 2;
	
	if(x == 0){
		mciSendString("seek gato1 to start", NULL, 0, 0);
		mciSendString("play gato1", NULL, 0, 0);
	}else{
		mciSendString("seek gato2 to start", NULL, 0, 0);
		mciSendString("play gato2", NULL, 0, 0);
	}	
	
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
	if(xSize == 1)
		meteor.mask = SmallMeteorMask;
	else
		meteor.mask = GreatMeteorMask;
	meteor.xSize = xSize;
	meteor.ySize = ySize;
	
	return meteor;
}



bool CheckCollision(int objectX, int objectY, int sizeX, int sizeY){
		//checa colisões por tick do jogo (60 fps)
	for(int i = 0; i < sizeX; i++){
		if(abs(objectX + i * tilesize - playerX) + abs(objectY - playerY) == 0)						
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
	//gerencia a fila de inputs para caso o jogador apertez muitas teclas em um curto periodo de tempo todas ainda sim sejam computadas
	//verifica se a lista tem espaço para mais inputs
	//checa cada tecla e se é a primeira vez que ela é pressionada ou se ela está sendo pressionada continuamente
	//caso seja a primeira vez pressionada adiciona o movimento na lista e declara a tecla como pressionada
	if(movesPending < 10){
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
	//checa se tem algum movimento pendente
	// se tiver verifica o primeiro input e executa o movimento
	// atualiza o sprite do player dependendo do movimento
	// e chama uma função que atualiza a lista de inputs
	if(movesPending > 0){
		switch(playerMoves[0]){	
			case('w'):
				activeSprite = 0;
				if(playerY > 192)playerY -= tilesize;
				movesPending--;
				RemoveFirstMove();				
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
							
				break;		
			
			case('d'):
				activeSprite = 1;
				if(playerX < display_width - tilesize - 32) playerX += tilesize;				
				movesPending--;
				RemoveFirstMove();									
			break;
				
		}
	}
	//checa o estado de cada tecla por tick
	wPress = IsPressing(key_W);
	aPress = IsPressing(key_A);
	dPress = IsPressing(key_D);
	sPress = IsPressing(key_S);
}

void MainMenuState(){
	void* menuBg = LoadSprite(".\\Sprites\\menu.bmp", 1680, 1050, 1, 0, 0, 1680, 1050);
	
	//mode 0 == normal mode
	//mode 1 == infinite mode
	int mode = 0;
	
	bool arrowPressed = false;
	
	HWND janela;
	janela = GetForegroundWindow();
	
	while(gs == MainMenu){
		SwitchPage();
		
		
		life = 7;
		spacePress = IsPressing(VK_SPACE);
		
		if (GetCursorPos(&P));
		if (ScreenToClient(janela, &P));
		
		//printf("%i\n", P.x);
		putimage(0, 0, menuBg, COPY_PUT);
		
		int x = P.x, y = P.y;
		
		putimage(display_width / 2 - 250, display_height - 250 - 30, playBtn, COPY_PUT);
		putimage(display_width / 2 - 250, display_height - 125 - 20, exitBtn, COPY_PUT);
		
		
		if(P.x > display_width / 2 - 250 && P.x < (display_width / 2 + 250)){
			//checagem de sobreposição do mouse sobre o botão jogar
			if(P.y > display_height - 250 && P.y < display_height - 145){
				if(GetKeyState(VK_LBUTTON)&0x80){
					mciSendString("stop menu", NULL, 0, 0);
					mciSendString("seek happy to start", NULL, 0, 0);
					mciSendString("play happy repeat", NULL, 0, 0);
					
					gs = Level1;
				}
			}else if(P.y > display_height - 125 - 30 && P.y < display_height){
				if(GetKeyState(VK_LBUTTON)&0x80){
					playing = false;
					gs = Level1;
				}
				
			}			
		}
			
		/*if(GetKeyState(VK_SPACE) & 0x80 && !spacePress){
			life = 7;
			if(mode == 0)
				gs = Level1;
			else
				gs = Infinite;
				
		} */
		if(GetKeyState(VK_LEFT) || GetKeyState(VK_RIGHT) & 0x80){
			if(mode == 0 && !arrowPressed) 
				mode = 1; 
			else if(mode == 1 && !arrowPressed) 
				mode = 0;
			
			arrowPressed = true;
			
		}else arrowPressed = false;
			
		
		setactivepage(pg);
	}
	free(menuBg);
	
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

void MeteorLevel(){
	
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	int cicleCounter = 0;
	
	buttonPressedQnt = 0;
	
	Cat cat = AddCat(6, 1, catImage);

	int tileset[7] = {0, 1, 1, 1, 1, 1, 1};	
	
	int seconds = 30;
	gameplayTimer[0] = 3;
	gameplayTimer[1] = 0;
	
	
	while(true){
		tickCount = GetTickCount();
		if(initTick - tickCount > 1000/60){
			initTick = tickCount;
			cicleCounter++;
			SwitchPage();
			putimage(0, 0, spaceBackground, COPY_PUT);
			
			
			if(life <= 0 || (gameplayTimer[0] <= 0 && gameplayTimer[1] <= 0)){
				gs = Dead;
				
				break;
			}
			
			
			//TIMER VISUAL
			DrawnSprite(display_width / 2-128, 0, numbers[gameplayTimer[0]], numbersMask[gameplayTimer[0]], 1);
			
			DrawnSprite(display_width / 2, 0, numbers[gameplayTimer[1]], numbersMask[gameplayTimer[1]], 1);
			
			if(tileset[playerY / 128] == 1)
				onMeteor = false;
			
			DrawnSprite(cat.x, cat.y, cat.sprite, catMask, 1);
			//PASSAR DE FASE
			if(CheckCollision(cat.x, cat.y, 1, 1)){
				gs = (Gamestate)((int)gs + 1);
				score += seconds * 15;
				MeowSound();
				break;
				
			};
			
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
				DrawnSprite(meteors[i].x, meteors[i].y, meteors[i].sprite, meteors[i].mask, 1);
				if(CheckCollision(meteors[i].x, meteors[i].y, meteors[i].xSize, meteors[i].ySize)) onMeteor = true;
				
				if(meteors[i].x <= 0 + 16 - (meteors[i].xSize) * 128 || meteors[i].x >= display_width - 16){
					if(meteors[i].direction == 1)
						meteors[i].x = 0 + 8;
					else
						meteors[i].x = display_width - 8 - meteors[i].xSize * 128;
				}	
			}
			
				
			PlayerInput();
			PlayerMovement();	
			
			DrawnSprite(playerX, playerY, playerSheet[activeSprite], playerMask[activeSprite], 1);
					
			char* scoreArr = convertIntegerToChar(score);
			nDigits = floor(log10(abs(score))) + 1;
			if(score > 0){
				for(int i = 0; i < nDigits; i++)
					DrawnSprite(i * 128, 128, numbers[(scoreArr[i] - '0')], numbersMask[(scoreArr[i] - '0')], 1);
			}		
			
			
			DrawnSprite(0, 0, scoreText, scoreMask, 1);
			
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


void AlienLevel(){
	
	unsigned long tickCount;
	unsigned long initTick;
	
	initTick = GetTickCount();
	
	int cicleCounter = 0;
	
	buttonPressedQnt = 0;
	
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
			DrawnSprite(display_width / 2-128, 0, numbers[gameplayTimer[0]], numbersMask[gameplayTimer[0]], 1);
			
			DrawnSprite(display_width / 2, 0, numbers[gameplayTimer[1]], numbersMask[gameplayTimer[1]], 1);
			
			
			if(tileset[playerY / 128] == 1)
				onMeteor = false;
			
			DrawnSprite(cat.x, cat.y, cat.sprite, catMask, 1);
			//PASSAR DE FASE
			if(CheckCollision(cat.x, cat.y, 1, 1) && cat.unlocked){
				gs = (Gamestate)((int)gs + 1);
				score += seconds * 15;
				MeowSound();
				break;
				
			};
			
			
			for(int i = 0; i < buttonQuantity; i++){
				if(!buttons[i].isPressed) DrawnSprite(buttons[i].x, buttons[i].y, buttons[i].sprite, buttons[i].mask, 1);
				else DrawnSprite(buttons[i].x, buttons[i].y, buttons[i].pressedSprite, buttons[i].pressedMask, 1);	
				if(CheckCollision(buttons[i].x + 8, buttons[i].y, 1, 1) && !buttons[i].isPressed){
					buttons[i].isPressed = true;
					score += 10;
					buttonPressedQnt++;		
					MeowSound();		
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
					
			//Desenhar o score
			char* scoreArr = convertIntegerToChar(score);
			nDigits = floor(log10(abs(score))) + 1;
			if(score > 0){
				for(int i = 0; i < nDigits; i++)
					DrawnSprite(i * 128, 128, numbers[(scoreArr[i] - '0')], numbersMask[(scoreArr[i] - '0')], 1);
			}		
			
			
			DrawnSprite(0, 0, scoreText, scoreMask, 1);
			
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

	playerSheet = LoadSpriteSheet(4, ".\\Sprites\\gato.bmp", 512, 128);
	playerMask = LoadSpriteSheet(4,".\\Sprites\\gatoMask.bmp", 512, 128);
	
	numbers = LoadSpriteSheet(10, ".\\Sprites\\numbers.bmp", 1280, 128);
	numbersMask = LoadSpriteSheet(10, ".\\Sprites\\numbersmask.bmp", 1280, 128);
	
	srand(time(NULL));
		
	int x =0;
		
	
	GreenLeftImage = LoadSprite(".\\Sprites\\GreenLeft.bmp", 128, 128, 1);
	GreenRightImage = LoadSprite(".\\Sprites\\GreenRight.bmp", 128, 128, 1);
	
	OrangeLeftImage = LoadSprite(".\\Sprites\\OrangeLeft.bmp", 128, 128, 1);
	OrangeRightImage = LoadSprite(".\\Sprites\\OrangeRight.bmp", 128, 128, 1);	
	
	PurpleLeftImage = LoadSprite(".\\Sprites\\PurpleLeft.bmp", 128, 128, 1);
	PurpleRightImage = LoadSprite(".\\Sprites\\PurpleRight.bmp", 128, 128, 1);	
	
	AlienLeftMask = LoadSprite(".\\Sprites\\AlienLeftMask.bmp", 128, 128, 1);
	AlienRightMask = LoadSprite(".\\Sprites\\AlienRightMask.bmp", 128, 128, 1);
	
	SmallMeteorImage = LoadSprite(".\\Sprites\\SmallMeteor.bmp", 128, 128, 1);
	SmallMeteorMask = LoadSprite(".\\Sprites\\SmallMeteorMask.bmp", 128, 128, 1);
	
	GreatMeteorImage = LoadSprite(".\\Sprites\\GreatMeteor.bmp", 256, 128, 1, 0, 0, 256, 128);
	GreatMeteorMask = LoadSprite(".\\Sprites\\GreatMeteorMask.bmp", 256, 128, 1, 0, 0, 256, 128);
	
	buttonImage = LoadSprite(".\\Sprites\\Button.bmp", 128, 128);
	buttonImageMask = LoadSprite(".\\Sprites\\ButtonMask.bmp", 128, 128);
	buttonPressedImage = LoadSprite(".\\Sprites\\ButtonPressed.bmp", 128, 128);
	buttonPressedMask = LoadSprite(".\\Sprites\\ButtonPressedMask.bmp", 128, 128);
	
	catImage = LoadSprite(".\\Sprites\\cat.bmp", 192, 128, 1, 0, 0, 192, 128);
	catMask = LoadSprite(".\\Sprites\\catMask.bmp", 192, 128, 1, 0, 0, 192, 128);
	floorBackground = LoadSprite(".\\Sprites\\background.bmp", 1680, 1050, 1, 0, 0, 1680-1, 1050-1);
	spaceBackground = LoadSprite(".\\Sprites\\spacenebula.bmp", 1680, 1050, 1, 0, 0, 1680-1, 1050-1);
	loseScreen = LoadSprite(".\\Sprites\\loseScreen.bmp", 1680, 1050, 1, 0, 0, 1680-1, 1050-1);
	victoryScreen = LoadSprite(".\\Sprites\\victoryScreen.bmp", 1680, 1050, 1, 0, 0, 1680-1, 1050-1);
	scoreText = LoadSprite(".\\Sprites\\score.bmp", 256, 128, 1, 0, 0, 256, 128);
	scoreMask = LoadSprite(".\\Sprites\\scoreMask.bmp", 256, 128, 1, 0, 0, 256, 128);
	
	playBtn = LoadSprite(".\\Sprites\\jogarBtn.bmp", 500, 125, 1, 0, 0, 500, 125);
	exitBtn = LoadSprite(".\\Sprites\\exitBtn.bmp", 500, 125, 1, 0, 0, 500, 125);
	
	waveOutSetVolume(0,0x01111111);
	
	mciSendString("open .\\Musica\\mainmenu.mp3 type MPEGVideo alias menu", NULL, 0, 0); 
	mciSendString("open .\\Musica\\happycat.mp3 type MPEGVideo alias happy", NULL, 0, 0);
	mciSendString("open .\\Musica\\gatinho1.mp3 type MPEGVideo alias gato1", NULL, 0, 0);
	mciSendString("open .\\Musica\\gatinho2.mp3 type MPEGVideo alias gato2", NULL, 0, 0);
	
	mciSendString("seek menu to start", NULL, 0, 0);
	mciSendString("play menu repeat", NULL, 0, 0);
	
	while(playing){
		SwitchPage();			
		switch(gs){
			case(MainMenu):				
				MainMenuState();			
				break;		
			case(Level1):
				
				//ALIENS CONFIG	
				meteors = NULL;														
				aliens = NULL;	
				alienQuantity = 6;
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(5, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[1] = AddAlien(0, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[2] = AddAlien(3, 3, -1, 2, GreenLeftImage, 1, 1);			
				aliens[3] = AddAlien(5, 5, 1, 2, GreenRightImage, 1, 1);
				aliens[4] = AddAlien(0, 5, 1, 2, GreenRightImage, 1, 1);			
				aliens[5] = AddAlien(13, 6, -1, 3, PurpleLeftImage, 1, 1);
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
				alienQuantity = 8;
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(5, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[1] = AddAlien(0, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[2] = AddAlien(3, 3, -1, 5, OrangeLeftImage, 1, 1);
				aliens[3] = AddAlien(9, 4, 1, 5, OrangeRightImage, 1, 1);
				aliens[4] = AddAlien(4, 4, 1, 5, OrangeRightImage, 1, 1);				
				aliens[5] = AddAlien(5, 5, 1, 2, GreenRightImage, 1, 1);
				aliens[6] = AddAlien(0, 5, 1, 2, GreenRightImage, 1, 1);			
				aliens[7] = AddAlien(13, 6, -1, 3, PurpleLeftImage, 1, 1);
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
				alienQuantity = 11;
				aliens = (Alien *) realloc(aliens, sizeof(Alien) * alienQuantity);
				
				aliens[0] = AddAlien(5, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[1] = AddAlien(0, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[2] = AddAlien(3, 2, -1, 3, PurpleLeftImage, 1, 1);
				aliens[3] = AddAlien(3, 3, -1, 5, OrangeLeftImage, 1, 1);
				aliens[4] = AddAlien(6, 3, -1, 5, OrangeLeftImage, 1, 1);
				aliens[5] = AddAlien(4, 4, 1, 5, OrangeRightImage, 1, 1);
				aliens[6] = AddAlien(2, 5, 1, 2, GreenRightImage, 1, 1);
				aliens[7] = AddAlien(3, 5, 1, 2, GreenRightImage, 1, 1);				
				aliens[8] = AddAlien(8, 5, 1, 2, GreenRightImage, 1, 1);
				aliens[9] = AddAlien(0, 5, 1, 2, GreenRightImage, 1, 1);			
				aliens[10] = AddAlien(13, 6, -1, 3, PurpleLeftImage, 1, 1);

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
			case (Level5):
				
				
				meteorQuantity = 22;													
				meteors = NULL;	
				meteors = (Meteor *) realloc(meteors, sizeof(Meteor) * meteorQuantity);
				//QUANTO MENOR O NUMERO Y DO OBJETO MAIS ALTO ELE ESTÁ
				
				meteors[0] = AddMeteor(0, 5, 1, 2, SmallMeteorImage, 1, 1);
				meteors[1] = AddMeteor(2, 5, 1, 2, SmallMeteorImage, 1, 1);
				meteors[18] = AddMeteor(4, 5, 1, 2, SmallMeteorImage, 1, 1);
				meteors[19] = AddMeteor(6, 5, 1, 2, SmallMeteorImage, 1, 1);
				meteors[20] = AddMeteor(8, 5, 1, 2, SmallMeteorImage, 1, 1);
				meteors[21] = AddMeteor(10, 5, 1, 2, SmallMeteorImage, 1, 1);
				
				meteors[2] = AddMeteor(3, 4, 1, 2, GreatMeteorImage, 2, 1);
				meteors[3] = AddMeteor(0, 4, 1, 2, SmallMeteorImage, 1, 1);
				meteors[14] = AddMeteor(5, 4, 1, 2, GreatMeteorImage, 2, 1);
				meteors[15] = AddMeteor(7, 4, 1, 2, GreatMeteorImage, 2, 1);
				
				
				meteors[4] = AddMeteor(9, 2, -1, 2, GreatMeteorImage, 2, 1);				
				meteors[5] = AddMeteor(3, 2, -1, 2, GreatMeteorImage, 2, 1);
				meteors[6] = AddMeteor(6, 2, -1, 2, GreatMeteorImage, 2, 1);
				meteors[7] = AddMeteor(11, 2, -1, 2, GreatMeteorImage, 2, 1);
				
				meteors[8] = AddMeteor(5, 3, -1, 3, SmallMeteorImage, 1, 1);
				meteors[9] = AddMeteor(3, 3, -1, 3, SmallMeteorImage, 1, 1);
				meteors[16] = AddMeteor(5, 3, -1, 3, SmallMeteorImage, 1, 1);
				meteors[17] = AddMeteor(7, 3, -1, 3, SmallMeteorImage, 1, 1);
				
				meteors[10] = AddMeteor(0, 6, -1, 2, GreatMeteorImage, 2, 1);
				meteors[11] = AddMeteor(3, 6, -1, 2, GreatMeteorImage, 2, 1);
				meteors[12] = AddMeteor(5, 6, -1, 2, GreatMeteorImage, 2, 1);
				meteors[13] = AddMeteor(7, 6, -1, 2, GreatMeteorImage, 2, 1);
				//
				
							
				MeteorLevel();
				break;
				case (Level4):
				
				
				
				aliens = NULL;		
				
				meteorQuantity = 16;													
				meteors = NULL;	
				meteors = (Meteor *) realloc(meteors, sizeof(Meteor) * meteorQuantity);
				//QUANTO MENOR O NUMERO Y DO OBJETO MAIS ALTO ELE ESTÁ
				
				meteors[0] = AddMeteor(0, 5, 1, 2, GreatMeteorImage, 2, 1);
				meteors[1] = AddMeteor(8, 5, 1, 2, GreatMeteorImage, 2, 1);
				
				meteors[2] = AddMeteor(3, 4, 1, 2, GreatMeteorImage, 2, 1);
				meteors[3] = AddMeteor(0, 4, 1, 2, SmallMeteorImage, 1, 1);
				meteors[14] = AddMeteor(5, 4, 1, 2, GreatMeteorImage, 2, 1);
				meteors[15] = AddMeteor(7, 4, 1, 2, GreatMeteorImage, 2, 1);
				
				
				meteors[4] = AddMeteor(9, 2, -1, 2, GreatMeteorImage, 2, 1);				
				meteors[5] = AddMeteor(3, 2, -1, 2, GreatMeteorImage, 2, 1);
				meteors[6] = AddMeteor(6, 2, -1, 2, GreatMeteorImage, 2, 1);
				meteors[7] = AddMeteor(11, 2, -1, 2, GreatMeteorImage, 2, 1);
				
				meteors[8] = AddMeteor(5, 3, -1, 2, GreatMeteorImage, 2, 1);
				meteors[9] = AddMeteor(3, 3, -1, 2, GreatMeteorImage, 2, 1);
				
				meteors[10] = AddMeteor(0, 6, -1, 2, GreatMeteorImage, 2, 1);
				meteors[11] = AddMeteor(3, 6, -1, 2, GreatMeteorImage, 2, 1);
				meteors[12] = AddMeteor(5, 6, -1, 2, GreatMeteorImage, 2, 1);
				meteors[13] = AddMeteor(7, 6, -1, 2, GreatMeteorImage, 2, 1);
										
				MeteorLevel();
				break;
			case(Dead):
				DrawnSprite(0, 0, loseScreen, NULL, 0);
				if(GetKeyState(VK_SPACE) & 0x80){
					gs = MainMenu;
					mciSendString("stop happy", NULL, 0, 0);
					mciSendString("seek happy to start", NULL, 0, 0);
					mciSendString("seek menu to start", NULL, 0, 0);
					mciSendString("play menu repeat", NULL, 0, 0);
				}
				break;
			case(Win):
				DrawnSprite(0, 0, victoryScreen, NULL, 0);
				if(GetKeyState(VK_SPACE) & 0x80){
					gs = MainMenu;		
					mciSendString("stop happy", NULL, 0, 0);
					mciSendString("seek happy to start", NULL, 0, 0);	
					mciSendString("seek menu to start", NULL, 0, 0);
					mciSendString("play menu repeat", NULL, 0, 0);		
				}
				break;
		}
					
		setactivepage(pg);
	};
	
	
	free(GreenLeftImage);
	free(GreenRightImage);
	free(OrangeLeftImage);
	free(OrangeRightImage);
	free(PurpleLeftImage);
	free(PurpleRightImage);
	free(AlienLeftMask);
	free(AlienRightMask);
	free(SmallMeteorImage);
	free(GreatMeteorImage);
	free(SmallMeteorMask);
	free(GreatMeteorMask);
	free(buttonImage);
	free(buttonImageMask);
	free(buttonPressedImage);
	free(buttonPressedMask);
	free(catImage);
	free(floorBackground);
	free(spaceBackground);
	free(loseScreen);
	free(scoreText);
	free(scoreMask);
	free(aliens);
	free(meteors);
	free(numbers);
	free(numbersMask);
	free(playerSheet);
	free(playerMask);
	free(buttons);
	
	closegraph();
	return 0;
}
