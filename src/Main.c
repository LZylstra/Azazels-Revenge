#define SDL_MAIN_HANDLED
#include<SDL2/SDL.h>
#include<GL/glew.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include"DrawUtils.h"
#include <stdbool.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRIDWIDTH 70
#define GRIDHEIGHT 70
#define TILE_SIZE 70
#define GAME_WIDTH 70 * 70
#define GAME_HEIGHT 70 * 70
#define ENEMYSIZE 15
#define ESPEED 0.2
#define COINSIZE 100
#define MAGICSIZE 5
#define POTIONSIZE 5
 
typedef struct AABB {
	int x, y, w, h;
}AABB;
// Defines an animation frame.
typedef struct Frame {
	int texIndex;
	float duration;
} Frame;

// Defines an animation and contains animation frames.
typedef struct AnimationDef {
	Frame frame[200];
	int totalFrames;
} AnimationDef;

// Stores the run-time state of an animation.
typedef struct AnimationData {
	AnimationDef* animation;
	int frameIndex;
	float timeToNextFrame;
	char isMoving;
} AnimationData;
typedef struct Projectile{
	AnimationData animData;
	//int damage;
	int xPos;
	int yPos;
	int xPosCam;
	int yPosCam;
	bool isVisible;
	bool initialized;
	int speed; 
	int distance;
	AABB box;

}Projectile;
typedef struct Character {
	AnimationData animData;
	int health;
	int magic;
	double xPos;
	double yPos;
	//int xPosCam;
	//int yPosCam;
	AABB box;
	int width;
	int height;
	bool isWalking;
	bool isDying;
	bool isAttacking;
	int prevX;
	int prevY;
	Projectile spells[5];
	//for platformer
	bool isGrounded;
	double yVelocity;
	double xVelocity;
	int direction; //1 up, 2 right, 3 down, 4 left
} Character;

typedef struct Camera {  
	int xPos;
	int yPos;
	int width;
	int height;

} Camera;

typedef struct Tile {
	int texIndex;
	int collision; //0 no collision, 1 top, 2 right, 3 left, 4 bottom
	int hasEnemy;
	int height;
	AABB box;
	int xPos, yPos;
	int object; //0 no object, 1 coin
} Tile;
typedef struct Behavior{
	int value;
	int timer;
	float weight;
	double behaviorMaxTime;
}Behavior;
typedef struct Enemy{
	AnimationData animData;
	int health;
	int xPosCam;
	int yPosCam;
	int yPos;
	int xPos;
	bool isGrounded;
	double yVelocity;
	bool hurt;
	AABB box;
	bool collided;
	int prevX;
	int prevY;
	bool evil;
	//AI
	double moveSpeed;
	double patrolDistance;
	double maxPatrolDistance;
	Behavior currentBehavior;
	Behavior actions[7];
	double axis;
	double direction;
	float behaviorTimer;//time left
	double initialBehaviorTimer;
}Enemy;

typedef struct Object{
	AnimationData animData;
	int xPos;
	int yPos;
	int value;
	bool collected;
	char type;
	AABB box;
}Object;


// Set this to 0 (false) to make the game loop exit.
char isGameLooping = 1;
char isLtr = 1;
// New.
Tile tile[GRIDHEIGHT][GRIDWIDTH];
Tile objectsTile[GRIDHEIGHT][GRIDWIDTH];
Tile collisionTile[GRIDHEIGHT][GRIDWIDTH];
Tile behindTile[GRIDHEIGHT][GRIDWIDTH];
Tile tile2[GRIDHEIGHT][GRIDWIDTH];
Tile backTile[GRIDHEIGHT][GRIDWIDTH];
GLuint tileTex[500];
GLuint spriteTex[300];
GLuint numTex[25];
int spriteWidth[300];
int spriteHeight[300];
int numWidth[25];
int numHeight[25];
Character demon;
Enemy cat;
Enemy cat2;
Enemy cat3;
Enemy cat4;
Enemy bossCat;
Camera camera;
Enemy enemies[ENEMYSIZE]; //int size = sizeof(a) / sizeof(a[0]);
AABB catAABB;
AABB cat2AABB;
AABB cat3AABB;
AABB cat4AABB;
AABB cameraAABB;
AABB demonAABB;
AABB tileAABB;
//Projectile magicball;
//AABB projectileAABB;
double jumpVelocity = -.6;//-1.2;//-.045; 
double yGravity = 0.0010;//.005;//0.000008;
bool jumpButtonPressed;
bool noCollision = true;
bool hasBeenHurt3 = false;
bool hasBeenHurt4= false;
int tileX;
int tileY;	
Tile t1,t2,t3,t4,t5,t6,t7,t8, t9;
time_t t; //RANDOM
bool death = false;
int gameState = 0; // 0 start menu, 1 play, 2 pause menu
bool pause = false;
AnimationDef demonIdleAnim, projectileAnim, catIdleAnim, coinAnim;
int grassMap2[GRIDHEIGHT][GRIDWIDTH] = {0};	
int mouseX, mouseY, mouseDeltaX, mouseDeltaY;
Uint32 mouseButtons;
//int buttonState = 0; //1 button1, 2 button2, 3 button3
//char FileName[] = "MyGame.svg";
FILE *SaveFile;
FILE *LoadFile;
Character Players[1]; 
const int PLAYERS = 1;
const int ENEMIES = 10; //ENEMYSIZE altered need to change this
char  SIGNATURE[4];
bool GameLoaded = false;
bool Saved = false;
struct Character *player = &demon;
//struct Enemy[] *enemy = &enemies;
const int playerStartX = 0;
const int playerStartY = 4690;
//Projectile projectiles[10];
Object coins[100];
Object potions[5];
int coinScore = 0;
int magicTimer;
int spellNum = 0;
AnimationDef potionAnim;
bool spacePressed;
int prevVel;
bool gameWon;
int bossStartX;
AnimationDef bcIdleAnim;
AnimationDef bcDieAnim, bcAttackAnim, demonEatAnim, catSplodeAnim, catAttackR;
AnimationDef catAttack, evilcatLoveRAnim, evilcatLoveAnim, evilcatIdleAnim;
AnimationDef evilcatWalkAnim, evilcatIdleRAnim, evilcatWalkRAnim, catIdleRAnim;
AnimationDef catWalkRAnim, catWalkAnim, demonMoveAnim, demonIdleAnim2;

//{ create method headers
// Functions.
void setAnimation(AnimationData*, AnimationDef*);
void setNewAnimation(AnimationData*, AnimationDef*);
void resetAnimation(AnimationData*);
void iterateAnimation(AnimationData*, float);
void drawCharacter(AnimationData*, int, int, int, int);
void drawObject(AnimationData*, int, int, int, int);
void drawBackground();

void moveCharacterPos(const unsigned char*, float);
void characterBounds();
void updateCameraPos(const unsigned char*, float);
void cameraBounds();
void updateCharacterPosInCam();

void updateCamera(float);
bool AABBIntersect(const AABB, const AABB);
void setCameraAABB(AABB);
void updateCharacterAABB();
void updateEnemyPrevPos();
void updateAABB();
void updateProjPos();
void drawProjectile(int);
void updateProjDraw();

int getTileX();
int getTileY();
void setTileBox();

void updateEnemy(int, float);
void makeNewDecision();
void executeDecision(int, float);

void colliding(Character, Tile, int, int);
//void setBox(Tile*, int, int);
int getOverlap(const AABB, const AABB);
double toDegrees(double);
void rollBehavior(int);
void enemyBounds(int);
void setEnemyAnimation(int, AnimationDef*);
float distance(int, int, int, int);
void resetGame();
//void mousePress(SDL_MouseButtonEvent*);
void saveGame();
void loadGame();
bool checkButton(int, int, int, int, int, int);
void mousePress(int, int);
void updateCoin();
void drawHUD();
void updateProjAABB();
void updateMagic(float);
void newDrawCharacter(AnimationData*, AnimationDef*, int, int, int, int);
void updatePotion();
void shoot();
void bossMovement(float);
//}

int main(void) {

//{	set up window
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL. ErrorCode=%s\n", SDL_GetError());
        return 1;
    }

    // Create the window and OpenGL context.
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window* window = SDL_CreateWindow(
            "lzylstra's Game",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_OPENGL);
    if (!window) {
        fprintf(stderr, "Could not create window. ErrorCode=%s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);  //1 to activate vsync,0 to disable

    // Make sure we have a recent version of OpenGL.
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Could not initialize glew. ErrorCode=%s\n", glewGetErrorString(glewError));
        SDL_Quit();
        return 1;
    }
    if (GLEW_VERSION_2_0) {
        fprintf(stderr, "OpenGL 2.0 or greater supported: Version=%s\n",
                 glGetString(GL_VERSION));
    } else {
        fprintf(stderr, "OpenGL max supported version is too low.\n");
        SDL_Quit();
        return 1;
    }

    // Setup OpenGL state.
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 100);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//}
//{	loading art
	// Loading art for drawing.
	//background tiles
	//tileTex[0] = glTexImageTGAFile("art/background/tile-water.tga", NULL, NULL);
	tileTex[0] = glTexImageTGAFile("art/Platformer/tiles/Tiles/blank.tga", NULL, NULL);
	//tileTex[1] = glTexImageTGAFile("art/background/purpletopedge.tga", NULL, NULL);
	tileTex[1] = glTexImageTGAFile("art/Platformer/tiles/Tiles/1.tga", NULL, NULL);
	//tileTex[2] = glTexImageTGAFile("art/background/flower1grass.tga", NULL, NULL);
	tileTex[2] = glTexImageTGAFile("art/Platformer/tiles/Tiles/2.tga", NULL, NULL);
	//tileTex[3] = glTexImageTGAFile("art/background/purplegrass.tga", NULL, NULL);
	tileTex[3] = glTexImageTGAFile("art/Platformer/tiles/Tiles/3.tga", NULL, NULL);
	//tileTex[4] = glTexImageTGAFile("art/background/purplebottomedge.tga", NULL, NULL);
	tileTex[4] = glTexImageTGAFile("art/Platformer/tiles/Tiles/4.tga", NULL, NULL);
	//tileTex[5] = glTexImageTGAFile("art/background/lava.tga", NULL, NULL);
	tileTex[5] = glTexImageTGAFile("art/Platformer/tiles/Tiles/5.tga", NULL, NULL);
	//tileTex[6] = glTexImageTGAFile("art/background/leftedge.tga", NULL, NULL);
	tileTex[6] = glTexImageTGAFile("art/Platformer/tiles/Tiles/6.tga", NULL, NULL);
	tileTex[7] = glTexImageTGAFile("art/background/rightedge.tga", NULL, NULL);
	//tileTex[8] = glTexImageTGAFile("art/background/topleftcorner.tga", NULL, NULL);
	tileTex[8] = glTexImageTGAFile("art/Platformer/tiles/Tiles/8.tga", NULL, NULL);
	//tileTex[9] = glTexImageTGAFile("art/background/toprightcorner.tga", NULL, NULL);
	tileTex[9] = glTexImageTGAFile("art/Platformer/tiles/Tiles/9.tga", NULL, NULL);
    //tileTex[10] = glTexImageTGAFile("art/background/bottomleftcorner.tga", NULL, NULL);
	tileTex[10] = glTexImageTGAFile("art/Platformer/tiles/Tiles/10.tga", NULL, NULL);
	tileTex[11] = glTexImageTGAFile("art/background/bottomrightcorner.tga", NULL, NULL);
	//tileTex[12] = glTexImageTGAFile("art/background/tree1.tga", NULL, NULL);
	tileTex[12] = glTexImageTGAFile("art/Platformer/tiles/Tiles/12.tga", NULL, NULL);
	//tileTex[13] = glTexImageTGAFile("art/background/tree2.tga", NULL, NULL);
	tileTex[13] = glTexImageTGAFile("art/Platformer/tiles/Tiles/13.tga", NULL, NULL);
	//tileTex[14] = glTexImageTGAFile("art/background/tree3.tga", NULL, NULL);
	tileTex[14] = glTexImageTGAFile("art/Platformer/tiles/Tiles/14.tga", NULL, NULL);
	//tileTex[15] = glTexImageTGAFile("art/background/tree4.tga", NULL, NULL);
	tileTex[15] = glTexImageTGAFile("art/Platformer/tiles/Tiles/15.tga", NULL, NULL);
	//tileTex[16] = glTexImageTGAFile("art/background/tree5.tga", NULL, NULL);
	tileTex[16] = glTexImageTGAFile("art/Platformer/tiles/Tiles/16.tga", NULL, NULL);
	//tileTex[17] = glTexImageTGAFile("art/background/devilsymbol.tga", NULL, NULL);
	tileTex[17] = glTexImageTGAFile("art/Platformer/tiles/Tiles/17.tga", NULL, NULL);
	tileTex[18] = glTexImageTGAFile("art/Platformer/tiles/Tiles/18.tga", NULL, NULL);
	
	tileTex[19] = glTexImageTGAFile("art/Platformer/tiles/Tiles/19.tga", NULL, NULL);
	tileTex[20] = glTexImageTGAFile("art/Platformer/tiles/Tiles/20.tga", NULL, NULL);
	tileTex[21] = glTexImageTGAFile("art/Platformer/tiles/Tiles/21.tga", NULL, NULL);
	tileTex[22] = glTexImageTGAFile("art/Platformer/tiles/Tiles/22.tga", NULL, NULL);
	tileTex[23] = glTexImageTGAFile("art/Platformer/tiles/Tiles/23.tga", NULL, NULL);
	tileTex[24] = glTexImageTGAFile("art/Platformer/tiles/Object/Bush3.tga", NULL, NULL);
	tileTex[25] = glTexImageTGAFile("art/Platformer/tiles/BG/BG.tga", NULL, NULL);
	tileTex[26] = glTexImageTGAFile("art/Platformer/grave/BG.tga", NULL, NULL);
	
	
	//main character
		//demon idle
	spriteTex[0] = glTexImageTGAFile("art/Demon/Idle/stand0.tga", &spriteWidth[0], &spriteHeight[0]);
	spriteTex[1] = glTexImageTGAFile("art/Demon/Idle/stand1.tga", &spriteWidth[1], &spriteHeight[1]);
	spriteTex[2] = glTexImageTGAFile("art/Demon/Idle/stand2.tga", &spriteWidth[2], &spriteHeight[2]);
	spriteTex[3] = glTexImageTGAFile("art/Demon/Idle/stand3.tga", &spriteWidth[3], &spriteHeight[3]);
	spriteTex[4] = glTexImageTGAFile("art/Demon/Idle/stand4.tga", &spriteWidth[4], &spriteHeight[4]);
	spriteTex[5] = glTexImageTGAFile("art/Demon/Idle/stand5.tga", &spriteWidth[5], &spriteHeight[5]);
	spriteTex[6] = glTexImageTGAFile("art/Demon/Idle/stand6.tga", &spriteWidth[6], &spriteHeight[6]);
	spriteTex[7] = glTexImageTGAFile("art/Demon/Idle/stand7.tga", &spriteWidth[7], &spriteHeight[7]);
	
		//demon move
	spriteTex[8] = glTexImageTGAFile("art/Demon/move/dmove0.tga", &spriteWidth[8], &spriteHeight[8]);
	spriteTex[9] = glTexImageTGAFile("art/Demon/move/dmove1.tga", &spriteWidth[9], &spriteHeight[9]);
	spriteTex[10] = glTexImageTGAFile("art/Demon/move/dmove2.tga", &spriteWidth[10], &spriteHeight[10]);
	spriteTex[11] = glTexImageTGAFile("art/Demon/move/dmove3.tga", &spriteWidth[11], &spriteHeight[11]);
	spriteTex[12] = glTexImageTGAFile("art/Demon/move/dmove4.tga", &spriteWidth[12], &spriteHeight[12]);
	spriteTex[13] = glTexImageTGAFile("art/Demon/move/dmove5.tga", &spriteWidth[13], &spriteHeight[13]);
	spriteTex[14] = glTexImageTGAFile("art/Demon/move/dmove6.tga", &spriteWidth[14], &spriteHeight[14]);
	spriteTex[15] = glTexImageTGAFile("art/Demon/move/dmove7.tga", &spriteWidth[15], &spriteHeight[15]);
	
		//demon fight  !! have to fix the size of the sprite before can use!!!
	spriteTex[16] = glTexImageTGAFile("art/Demon/fight/dattack0.tga", &spriteWidth[16], &spriteHeight[16]);
	spriteTex[17] = glTexImageTGAFile("art/Demon/fight/dattack1.tga", &spriteWidth[17], &spriteHeight[17]);
	spriteTex[18] = glTexImageTGAFile("art/Demon/fight/dattack2.tga", &spriteWidth[18], &spriteHeight[18]);
	spriteTex[19] = glTexImageTGAFile("art/Demon/fight/dattack3.tga", &spriteWidth[19], &spriteHeight[19]);
	spriteTex[20] = glTexImageTGAFile("art/Demon/fight/dattack4.tga", &spriteWidth[20], &spriteHeight[20]);
	spriteTex[21] = glTexImageTGAFile("art/Demon/fight/dattack5.tga", &spriteWidth[21], &spriteHeight[21]);
	spriteTex[22] = glTexImageTGAFile("art/Demon/fight/dattack6.tga", &spriteWidth[22], &spriteHeight[22]);
	spriteTex[23] = glTexImageTGAFile("art/Demon/fight/dattack7.tga", &spriteWidth[23], &spriteHeight[23]);
	spriteTex[24] = glTexImageTGAFile("art/Demon/fight/dattack8.tga", &spriteWidth[24], &spriteHeight[24]);
	spriteTex[25] = glTexImageTGAFile("art/Demon/fight/dattack9.tga", &spriteWidth[25], &spriteHeight[25]);
	spriteTex[26] = glTexImageTGAFile("art/Demon/fight/dattack10.tga", &spriteWidth[26], &spriteHeight[26]);
	
		//demon death  !! have to fix the size of the sprite before can use!!!
	spriteTex[27] = glTexImageTGAFile("art/Demon/die/adj/ddeath0.tga", &spriteWidth[27], &spriteHeight[27]);
	spriteTex[28] = glTexImageTGAFile("art/Demon/die/adj/ddeath1.tga", &spriteWidth[28], &spriteHeight[28]);
	spriteTex[29] = glTexImageTGAFile("art/Demon/die/adj/ddeath2.tga", &spriteWidth[29], &spriteHeight[29]);
	spriteTex[30] = glTexImageTGAFile("art/Demon/die/adj/ddeath3.tga", &spriteWidth[30], &spriteHeight[30]);
	spriteTex[31] = glTexImageTGAFile("art/Demon/die/adj/ddeath4.tga", &spriteWidth[31], &spriteHeight[31]);
	spriteTex[32] = glTexImageTGAFile("art/Demon/die/adj/ddeath5.tga", &spriteWidth[32], &spriteHeight[32]);
	spriteTex[33] = glTexImageTGAFile("art/Demon/die/adj/ddeath6.tga", &spriteWidth[33], &spriteHeight[33]);
	spriteTex[34] = glTexImageTGAFile("art/Demon/die/adj/ddeath7.tga", &spriteWidth[34], &spriteHeight[34]);
	spriteTex[35] = glTexImageTGAFile("art/Demon/die/adj/ddeath8.tga", &spriteWidth[35], &spriteHeight[35]);
	spriteTex[36] = glTexImageTGAFile("art/Demon/die/adj/ddeath9.tga", &spriteWidth[36], &spriteHeight[36]);
	spriteTex[37] = glTexImageTGAFile("art/Demon/die/adj/ddeath10.tga", &spriteWidth[37], &spriteHeight[37]);
	spriteTex[38] = glTexImageTGAFile("art/Demon/die/adj/ddeath11.tga", &spriteWidth[38], &spriteHeight[38]);
	spriteTex[39] = glTexImageTGAFile("art/Demon/die/adj/ddeath12.tga", &spriteWidth[39], &spriteHeight[39]);
	spriteTex[40] = glTexImageTGAFile("art/Demon/die/adj/ddeath13.tga", &spriteWidth[40], &spriteHeight[40]);
	spriteTex[41] = glTexImageTGAFile("art/Demon/die/adj/ddeath14.tga", &spriteWidth[41], &spriteHeight[41]);
	spriteTex[42] = glTexImageTGAFile("art/Demon/die/adj/ddeath15.tga", &spriteWidth[42], &spriteHeight[42]);
	
	//enemies
		//cat idle blink 
	spriteTex[43] = glTexImageTGAFile("art/enemies/cat/adj/catblink0.tga", &spriteWidth[43], &spriteHeight[43]);
	spriteTex[44] = glTexImageTGAFile("art/enemies/cat/adj/catblink1.tga", &spriteWidth[44], &spriteHeight[44]);
	spriteTex[45] = glTexImageTGAFile("art/enemies/cat/adj/catblink2.tga", &spriteWidth[45], &spriteHeight[45]);
	spriteTex[46] = glTexImageTGAFile("art/enemies/cat/adj/cattail0.tga", &spriteWidth[46], &spriteHeight[46]);
	spriteTex[47] = glTexImageTGAFile("art/enemies/cat/adj/cattail1.tga", &spriteWidth[47], &spriteHeight[47]);
	spriteTex[48] = glTexImageTGAFile("art/enemies/cat/adj/cattail2.tga", &spriteWidth[48], &spriteHeight[48]);
	spriteTex[49] = glTexImageTGAFile("art/enemies/cat/adj/cattail3.tga", &spriteWidth[49], &spriteHeight[49]);
	spriteTex[50] = glTexImageTGAFile("art/enemies/cat/adj/cattail4.tga", &spriteWidth[50], &spriteHeight[50]);
	spriteTex[51] = glTexImageTGAFile("art/enemies/cat/adj/cattail5.tga", &spriteWidth[51], &spriteHeight[51]);
	spriteTex[52] = glTexImageTGAFile("art/enemies/cat/adj/cattail6.tga", &spriteWidth[52], &spriteHeight[52]);
		//cat dead
	spriteTex[53] = glTexImageTGAFile("art/enemies/cat/adj/catdead.tga", &spriteWidth[53], &spriteHeight[53]);
		//cat hurt
	spriteTex[54] = glTexImageTGAFile("art/enemies/cat/adj/cathurt.tga", &spriteWidth[54], &spriteHeight[54]);
		//cat attack
	spriteTex[55] = glTexImageTGAFile("art/enemies/cat/adj/catthrowattack.tga", &spriteWidth[55], &spriteHeight[55]);
		// cat walking
	spriteTex[56] = glTexImageTGAFile("art/enemies/cat/adj/catwalk0.tga", &spriteWidth[56], &spriteHeight[56]);
	spriteTex[57] = glTexImageTGAFile("art/enemies/cat/adj/catwalk1.tga", &spriteWidth[57], &spriteHeight[57]);
	spriteTex[58] = glTexImageTGAFile("art/enemies/cat/adj/catwalk2.tga", &spriteWidth[58], &spriteHeight[58]);
	
	//projectile
	spriteTex[59] = glTexImageTGAFile("art/Projectiles/magicball0.tga", &spriteWidth[59], &spriteHeight[59]);
	spriteTex[60] = glTexImageTGAFile("art/Projectiles/magicball1.tga", &spriteWidth[60], &spriteHeight[60]);
	
	//demon idle reverse
	spriteTex[61] = glTexImageTGAFile("art/Demon/Idle/stand0L.tga", &spriteWidth[0], &spriteHeight[0]);
	spriteTex[62] = glTexImageTGAFile("art/Demon/Idle/stand1L.tga", &spriteWidth[1], &spriteHeight[1]);
	spriteTex[63] = glTexImageTGAFile("art/Demon/Idle/stand2L.tga", &spriteWidth[2], &spriteHeight[2]);
	spriteTex[64] = glTexImageTGAFile("art/Demon/Idle/stand3L.tga", &spriteWidth[3], &spriteHeight[3]);
	spriteTex[65] = glTexImageTGAFile("art/Demon/Idle/stand4L.tga", &spriteWidth[4], &spriteHeight[4]);
	spriteTex[66] = glTexImageTGAFile("art/Demon/Idle/stand5L.tga", &spriteWidth[5], &spriteHeight[5]);
	spriteTex[67] = glTexImageTGAFile("art/Demon/Idle/stand6L.tga", &spriteWidth[6], &spriteHeight[6]);
	spriteTex[68] = glTexImageTGAFile("art/Demon/Idle/stand7L.tga", &spriteWidth[7], &spriteHeight[7]);
	
	//cat idle blink reverse
	spriteTex[69] = glTexImageTGAFile("art/enemies/cat/adj/catblink0R.tga", &spriteWidth[43], &spriteHeight[43]);
	spriteTex[70] = glTexImageTGAFile("art/enemies/cat/adj/catblink1R.tga", &spriteWidth[44], &spriteHeight[44]);
	spriteTex[71] = glTexImageTGAFile("art/enemies/cat/adj/catblink2R.tga", &spriteWidth[45], &spriteHeight[45]);
	spriteTex[72] = glTexImageTGAFile("art/enemies/cat/adj/cattail0R.tga", &spriteWidth[46], &spriteHeight[46]);
	spriteTex[73] = glTexImageTGAFile("art/enemies/cat/adj/cattail1R.tga", &spriteWidth[47], &spriteHeight[47]);
	spriteTex[74] = glTexImageTGAFile("art/enemies/cat/adj/cattail2R.tga", &spriteWidth[48], &spriteHeight[48]);
	spriteTex[75] = glTexImageTGAFile("art/enemies/cat/adj/cattail3R.tga", &spriteWidth[49], &spriteHeight[49]);
	spriteTex[76] = glTexImageTGAFile("art/enemies/cat/adj/cattail4R.tga", &spriteWidth[50], &spriteHeight[50]);
	spriteTex[77] = glTexImageTGAFile("art/enemies/cat/adj/cattail5R.tga", &spriteWidth[51], &spriteHeight[51]);
	spriteTex[78] = glTexImageTGAFile("art/enemies/cat/adj/cattail6R.tga", &spriteWidth[52], &spriteHeight[52]);
	
	// cat reverse walking
	spriteTex[79] = glTexImageTGAFile("art/enemies/cat/adj/catwalk0R.tga", &spriteWidth[56], &spriteHeight[56]);
	spriteTex[80] = glTexImageTGAFile("art/enemies/cat/adj/catwalk1R.tga", &spriteWidth[57], &spriteHeight[57]);
	spriteTex[81] = glTexImageTGAFile("art/enemies/cat/adj/catwalk2R.tga", &spriteWidth[58], &spriteHeight[58]);
	
	//evil cat idle blink reverse
	spriteTex[82] = glTexImageTGAFile("art/enemies/cat/adj/evil/catblink0R.tga", &spriteWidth[43], &spriteHeight[43]);
	spriteTex[83] = glTexImageTGAFile("art/enemies/cat/adj/evil/catblink1R.tga", &spriteWidth[44], &spriteHeight[44]);
	spriteTex[84] = glTexImageTGAFile("art/enemies/cat/adj/evil/catblink2R.tga", &spriteWidth[45], &spriteHeight[45]);
	spriteTex[85] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail0R.tga", &spriteWidth[46], &spriteHeight[46]);
	spriteTex[86] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail1R.tga", &spriteWidth[47], &spriteHeight[47]);
	spriteTex[87] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail2R.tga", &spriteWidth[48], &spriteHeight[48]);
	spriteTex[88] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail3R.tga", &spriteWidth[49], &spriteHeight[49]);
	spriteTex[89] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail4R.tga", &spriteWidth[50], &spriteHeight[50]);
	spriteTex[90] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail5R.tga", &spriteWidth[51], &spriteHeight[51]);
	spriteTex[91] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail6R.tga", &spriteWidth[52], &spriteHeight[52]);
	
	//evil cat reverse walking
	spriteTex[92] = glTexImageTGAFile("art/enemies/cat/adj/evil/catwalk0R.tga", &spriteWidth[56], &spriteHeight[56]);
	spriteTex[93] = glTexImageTGAFile("art/enemies/cat/adj/evil/catwalk1R.tga", &spriteWidth[57], &spriteHeight[57]);
	spriteTex[94] = glTexImageTGAFile("art/enemies/cat/adj/evil/catwalk2R.tga", &spriteWidth[58], &spriteHeight[58]);
	
	//evil cat walking
	spriteTex[95] = glTexImageTGAFile("art/enemies/cat/adj/evil/catwalk0.tga", &spriteWidth[56], &spriteHeight[56]);
	spriteTex[96] = glTexImageTGAFile("art/enemies/cat/adj/evil/catwalk1.tga", &spriteWidth[57], &spriteHeight[57]);
	spriteTex[97] = glTexImageTGAFile("art/enemies/cat/adj/evil/catwalk2.tga", &spriteWidth[58], &spriteHeight[58]);
	
	//evil cat idle blink 
	spriteTex[98] = glTexImageTGAFile("art/enemies/cat/adj/evil/catblink0.tga", &spriteWidth[43], &spriteHeight[43]);
	spriteTex[99] = glTexImageTGAFile("art/enemies/cat/adj/evil/catblink1.tga", &spriteWidth[44], &spriteHeight[44]);
	spriteTex[100] = glTexImageTGAFile("art/enemies/cat/adj/evil/catblink2.tga", &spriteWidth[45], &spriteHeight[45]);
	spriteTex[101] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail0.tga", &spriteWidth[46], &spriteHeight[46]);
	spriteTex[102] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail1.tga", &spriteWidth[47], &spriteHeight[47]);
	spriteTex[103] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail2.tga", &spriteWidth[48], &spriteHeight[48]);
	spriteTex[104] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail3.tga", &spriteWidth[49], &spriteHeight[49]);
	spriteTex[105] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail4.tga", &spriteWidth[50], &spriteHeight[50]);
	spriteTex[106] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail5.tga", &spriteWidth[51], &spriteHeight[51]);
	spriteTex[107] = glTexImageTGAFile("art/enemies/cat/adj/evil/cattail6.tga", &spriteWidth[52], &spriteHeight[52]);
	
	//evil cat love
	spriteTex[108] = glTexImageTGAFile("art/enemies/cat/adj/evil/love0.tga", &spriteWidth[49], &spriteHeight[49]);
	spriteTex[109] = glTexImageTGAFile("art/enemies/cat/adj/evil/love1.tga", &spriteWidth[50], &spriteHeight[50]);
	spriteTex[110] = glTexImageTGAFile("art/enemies/cat/adj/evil/love2.tga", &spriteWidth[51], &spriteHeight[51]);
	spriteTex[111] = glTexImageTGAFile("art/enemies/cat/adj/evil/love3.tga", &spriteWidth[52], &spriteHeight[52]);
	spriteTex[112] = glTexImageTGAFile("art/enemies/cat/adj/evil/love4.tga", &spriteWidth[52], &spriteHeight[52]);
	
	//evil cat love reverse
	spriteTex[113] = glTexImageTGAFile("art/enemies/cat/adj/evil/love0R.tga", &spriteWidth[49], &spriteHeight[49]);
	spriteTex[114] = glTexImageTGAFile("art/enemies/cat/adj/evil/love1R.tga", &spriteWidth[50], &spriteHeight[50]);
	spriteTex[115] = glTexImageTGAFile("art/enemies/cat/adj/evil/love2R.tga", &spriteWidth[51], &spriteHeight[51]);
	spriteTex[116] = glTexImageTGAFile("art/enemies/cat/adj/evil/love3R.tga", &spriteWidth[52], &spriteHeight[52]);
	spriteTex[117] = glTexImageTGAFile("art/enemies/cat/adj/evil/love4R.tga", &spriteWidth[52], &spriteHeight[52]);
	
	//reverse cat attack
	spriteTex[118] = glTexImageTGAFile("art/enemies/cat/adj/catthrowattackR.tga", &spriteWidth[55], &spriteHeight[55]);
	
	//menu sprites
	spriteTex[119] = glTexImageTGAFile("art/GUI/startmenu.tga", &spriteWidth[119], &spriteHeight[119]);
	spriteTex[120] = glTexImageTGAFile("art/GUI/loadbtn.tga", &spriteWidth[120], &spriteHeight[120]);
	spriteTex[121] = glTexImageTGAFile("art/GUI/playbtn.tga", &spriteWidth[121], &spriteHeight[121]);
	spriteTex[122] = glTexImageTGAFile("art/GUI/quitbtn.tga", &spriteWidth[122], &spriteHeight[122]);
	spriteTex[123] = glTexImageTGAFile("art/GUI/pausemenu.tga", &spriteWidth[123], &spriteHeight[123]);
	spriteTex[124] = glTexImageTGAFile("art/GUI/resumebtn.tga", &spriteWidth[124], &spriteHeight[124]);
	spriteTex[125] = glTexImageTGAFile("art/GUI/restartbtn.tga", &spriteWidth[125], &spriteHeight[125]);
	spriteTex[126] = glTexImageTGAFile("art/GUI/exitbtn.tga", &spriteWidth[126], &spriteHeight[126]);
	spriteTex[127] = glTexImageTGAFile("art/GUI/savebtn.tga", &spriteWidth[127], &spriteHeight[127]);
	spriteTex[128] = glTexImageTGAFile("art/GUI/savebtnP.tga", &spriteWidth[128], &spriteHeight[128]);
	
	//coin
	spriteTex[129] = glTexImageTGAFile("art/objects/goldcoin0.tga", &spriteWidth[129], &spriteHeight[129]);
	spriteTex[130] = glTexImageTGAFile("art/objects/goldcoin1.tga", &spriteWidth[130], &spriteHeight[130]);
	spriteTex[131] = glTexImageTGAFile("art/objects/goldcoin2.tga", &spriteWidth[131], &spriteHeight[131]);
	spriteTex[132] = glTexImageTGAFile("art/objects/goldcoin3.tga", &spriteWidth[132], &spriteHeight[132]);
	
	//catsplode
	spriteTex[133] = glTexImageTGAFile("art/enemies/cat/adj/evil/catsplode0.tga", &spriteWidth[133], &spriteHeight[133]);
	spriteTex[134] = glTexImageTGAFile("art/enemies/cat/adj/evil/catsplode1.tga", &spriteWidth[134], &spriteHeight[134]);
	spriteTex[135] = glTexImageTGAFile("art/enemies/cat/adj/evil/catsplode2.tga", &spriteWidth[135], &spriteHeight[135]);
	spriteTex[136] = glTexImageTGAFile("art/enemies/cat/adj/evil/catsplode3.tga", &spriteWidth[136], &spriteHeight[136]);
	
	//demon eat
	spriteTex[137] = glTexImageTGAFile("art/Demon/eat/eat0.tga", &spriteWidth[137], &spriteHeight[137]);
	spriteTex[138] = glTexImageTGAFile("art/Demon/eat/eat1.tga", &spriteWidth[138], &spriteHeight[138]);
	
	//HUD
	spriteTex[139] = glTexImageTGAFile("art/HUD/blanktrans.tga", &spriteWidth[139], &spriteHeight[139]);
	spriteTex[140] = glTexImageTGAFile("art/HUD/healthbarT0.tga", &spriteWidth[140], &spriteHeight[140]);
	spriteTex[141] = glTexImageTGAFile("art/HUD/healthbarT1.tga", &spriteWidth[141], &spriteHeight[141]);
	spriteTex[142] = glTexImageTGAFile("art/HUD/healthbarT2.tga", &spriteWidth[142], &spriteHeight[142]);
	spriteTex[143] = glTexImageTGAFile("art/HUD/healthbarT3.tga", &spriteWidth[143], &spriteHeight[143]);
	spriteTex[144] = glTexImageTGAFile("art/HUD/healthbarT4.tga", &spriteWidth[144], &spriteHeight[144]);
	spriteTex[145] = glTexImageTGAFile("art/HUD/healthbarT5.tga", &spriteWidth[145], &spriteHeight[145]);
	spriteTex[146] = glTexImageTGAFile("art/HUD/healthbarT6.tga", &spriteWidth[146], &spriteHeight[146]);
	spriteTex[147] = glTexImageTGAFile("art/HUD/healthbarT7.tga", &spriteWidth[147], &spriteHeight[147]);
	spriteTex[148] = glTexImageTGAFile("art/HUD/healthbar8.tga", &spriteWidth[148], &spriteHeight[148]);
	spriteTex[149] = glTexImageTGAFile("art/HUD/dyingoverlay.tga", &spriteWidth[149], &spriteHeight[149]);	
	spriteTex[150] = glTexImageTGAFile("art/HUD/magicbarT0.tga", &spriteWidth[150], &spriteHeight[150]);
	spriteTex[151] = glTexImageTGAFile("art/HUD/magicbarT1.tga", &spriteWidth[151], &spriteHeight[151]);
	spriteTex[152] = glTexImageTGAFile("art/HUD/magicbarT2.tga", &spriteWidth[152], &spriteHeight[152]);		
	spriteTex[153] = glTexImageTGAFile("art/HUD/magicbarT3.tga", &spriteWidth[153], &spriteHeight[153]);		
	spriteTex[154] = glTexImageTGAFile("art/HUD/magicbarT4.tga", &spriteWidth[154], &spriteHeight[154]);		
	spriteTex[155] = glTexImageTGAFile("art/HUD/magicbar5.tga", &spriteWidth[155], &spriteHeight[155]);		
	spriteTex[156] = glTexImageTGAFile("art/HUD/gameover.tga", &spriteWidth[156], &spriteHeight[156]);
	
	spriteTex[157] = glTexImageTGAFile("art/objects/potion0.tga", &spriteWidth[157], &spriteHeight[157]);
	spriteTex[158] = glTexImageTGAFile("art/objects/potion1.tga", &spriteWidth[158], &spriteHeight[158]);
	
	spriteTex[159] = glTexImageTGAFile("art/HUD/win.tga", &spriteWidth[159], &spriteHeight[159]);
	
	spriteTex[160] = glTexImageTGAFile("art/enemies/bossCat/idle0.tga", &spriteWidth[160], &spriteHeight[160]);
	spriteTex[161] = glTexImageTGAFile("art/enemies/bossCat/idle1.tga", &spriteWidth[161], &spriteHeight[161]);
	spriteTex[162] = glTexImageTGAFile("art/enemies/bossCat/idle2.tga", &spriteWidth[162], &spriteHeight[162]);
	spriteTex[163] = glTexImageTGAFile("art/enemies/bossCat/idle3.tga", &spriteWidth[163], &spriteHeight[163]);
	spriteTex[164] = glTexImageTGAFile("art/enemies/bossCat/attack0.tga", &spriteWidth[164], &spriteHeight[164]);
	spriteTex[165] = glTexImageTGAFile("art/enemies/bossCat/attack1.tga", &spriteWidth[165], &spriteHeight[165]);
	spriteTex[166] = glTexImageTGAFile("art/enemies/bossCat/attack2.tga", &spriteWidth[166], &spriteHeight[166]);
	spriteTex[167] = glTexImageTGAFile("art/enemies/bossCat/attack3.tga", &spriteWidth[167], &spriteHeight[167]);
	spriteTex[168] = glTexImageTGAFile("art/enemies/bossCat/attack4.tga", &spriteWidth[168], &spriteHeight[168]);
	spriteTex[169] = glTexImageTGAFile("art/enemies/bossCat/attack5.tga", &spriteWidth[169], &spriteHeight[169]);
	spriteTex[170] = glTexImageTGAFile("art/enemies/bossCat/attack6.tga", &spriteWidth[170], &spriteHeight[170]);
	spriteTex[171] = glTexImageTGAFile("art/enemies/bossCat/attack7.tga", &spriteWidth[171], &spriteHeight[171]);
	spriteTex[172] = glTexImageTGAFile("art/enemies/bossCat/dying0.tga", &spriteWidth[172], &spriteHeight[172]);
	spriteTex[173] = glTexImageTGAFile("art/enemies/bossCat/dying1.tga", &spriteWidth[173], &spriteHeight[173]);
	spriteTex[174] = glTexImageTGAFile("art/enemies/bossCat/dying2.tga", &spriteWidth[174], &spriteHeight[174]);
	spriteTex[175] = glTexImageTGAFile("art/enemies/bossCat/dying3.tga", &spriteWidth[175], &spriteHeight[175]);
	spriteTex[176] = glTexImageTGAFile("art/enemies/bossCat/dying4.tga", &spriteWidth[176], &spriteHeight[176]);
	spriteTex[177] = glTexImageTGAFile("art/enemies/bossCat/dying5.tga", &spriteWidth[177], &spriteHeight[177]);
	spriteTex[178] = glTexImageTGAFile("art/enemies/bossCat/dying6.tga", &spriteWidth[178], &spriteHeight[178]);
	spriteTex[179] = glTexImageTGAFile("art/enemies/bossCat/dying7.tga", &spriteWidth[179], &spriteHeight[179]);
	spriteTex[180] = glTexImageTGAFile("art/enemies/bossCat/dying8.tga", &spriteWidth[180], &spriteHeight[180]);
	
	
	
	numTex[0] = glTexImageTGAFile("art/HUD/numbers/indiv/0.tga", &numWidth[0], &numHeight[0]);
	numTex[1] = glTexImageTGAFile("art/HUD/numbers/indiv/00.tga", &numWidth[1], &numHeight[1]);	
	numTex[2] = glTexImageTGAFile("art/HUD/numbers/indiv/100.tga", &numWidth[2], &numHeight[2]);
	numTex[3] = glTexImageTGAFile("art/HUD/numbers/indiv/200.tga", &numWidth[3], &numHeight[3]);	
	numTex[4] = glTexImageTGAFile("art/HUD/numbers/indiv/300.tga", &numWidth[4], &numHeight[4]);	
	numTex[5] = glTexImageTGAFile("art/HUD/numbers/indiv/400.tga", &numWidth[5], &numHeight[5]);
	numTex[6] = glTexImageTGAFile("art/HUD/numbers/indiv/500.tga", &numWidth[6], &numHeight[6]);	
	numTex[7] = glTexImageTGAFile("art/HUD/numbers/indiv/600.tga", &numWidth[7], &numHeight[7]);	
	numTex[8] = glTexImageTGAFile("art/HUD/numbers/indiv/700.tga", &numWidth[8], &numHeight[8]);	
	numTex[9] = glTexImageTGAFile("art/HUD/numbers/indiv/800.tga", &numWidth[9], &numHeight[9]);	
	numTex[10] = glTexImageTGAFile("art/HUD/numbers/indiv/900.tga", &numWidth[10], &numHeight[10]);
	numTex[11] = glTexImageTGAFile("art/HUD/numbers/indiv/,.tga", &numWidth[11], &numHeight[11]);
	numTex[12] = glTexImageTGAFile("art/HUD/numbers/indiv/1,000.tga", &numWidth[12], &numHeight[12]);
	numTex[13] = glTexImageTGAFile("art/HUD/numbers/indiv/2,000.tga", &numWidth[13], &numHeight[13]);	
	numTex[14] = glTexImageTGAFile("art/HUD/numbers/indiv/3,000.tga", &numWidth[14], &numHeight[14]);	
	numTex[15] = glTexImageTGAFile("art/HUD/numbers/indiv/4,000.tga", &numWidth[15], &numHeight[15]);	
	numTex[16] = glTexImageTGAFile("art/HUD/numbers/indiv/5,000.tga", &numWidth[16], &numHeight[16]);	
	numTex[17] = glTexImageTGAFile("art/HUD/numbers/indiv/6,000.tga", &numWidth[17], &numHeight[17]);	
	numTex[18] = glTexImageTGAFile("art/HUD/numbers/indiv/7,000.tga", &numWidth[18], &numHeight[18]);	
	numTex[19] = glTexImageTGAFile("art/HUD/numbers/indiv/8,000.tga", &numWidth[19], &numHeight[19]);	
	numTex[20] = glTexImageTGAFile("art/HUD/numbers/indiv/9,000.tga", &numWidth[20], &numHeight[20]);	
	numTex[21] = glTexImageTGAFile("art/HUD/numbers/indiv/0,000.tga", &numWidth[21], &numHeight[21]);	
	numTex[22] = glTexImageTGAFile("art/HUD/numbers/indiv/10,000.tga", &numWidth[22], &numHeight[22]);
	numTex[23] = glTexImageTGAFile("art/HUD/numbers/indiv/000.tga", &numWidth[23], &numHeight[23]);	
	
	/*
	spriteTex[180] = glTexImageTGAFile("art/HUD/numbers/2300T.tga", &spriteWidth[180], &spriteHeight[180]);	
	spriteTex[181] = glTexImageTGAFile("art/HUD/numbers/2400T.tga", &spriteWidth[181], &spriteHeight[181]);	
	spriteTex[182] = glTexImageTGAFile("art/HUD/numbers/2500T.tga", &spriteWidth[182], &spriteHeight[182]);	
	spriteTex[183] = glTexImageTGAFile("art/HUD/numbers/2600T.tga", &spriteWidth[183], &spriteHeight[183]);	
	spriteTex[184] = glTexImageTGAFile("art/HUD/numbers/2700T.tga", &spriteWidth[184], &spriteHeight[184]);	
	spriteTex[185] = glTexImageTGAFile("art/HUD/numbers/2800T.tga", &spriteWidth[185], &spriteHeight[185]);	
	spriteTex[186] = glTexImageTGAFile("art/HUD/numbers/2900T.tga", &spriteWidth[186], &spriteHeight[186]);	
	spriteTex[187] = glTexImageTGAFile("art/HUD/numbers/3000T.tga", &spriteWidth[187], &spriteHeight[187]);	
	*/
	
//}	
	
	// Setting up camera.
	// Camera dimen w=480 h=320
	// w=800=160+x+160 h=600=160+y+160
	spacePressed = false;
	gameWon = false;
	magicTimer = 10000;
	camera.xPos = 0;
	camera.yPos = GRIDHEIGHT * TILE_SIZE - WINDOW_HEIGHT;
	camera.width = WINDOW_WIDTH; // 2;
	camera.height = WINDOW_HEIGHT; // 2;

	demon.xPos = playerStartX;
	demon.yPos = playerStartY;
	demon.prevX = demon.xPos;
	demon.prevY = demon.yPos;
		
	demon.isWalking = false;
	demon.width = spriteWidth[0];
	demon.height = spriteHeight[0];
	demonAABB.x = demon.xPos;
	demonAABB.y = demon.yPos;
	demonAABB.w = demon.width;
	demonAABB.h = demon.height;
	demon.health = 160;
	demon.magic = 165;
	demon.isGrounded = true;
	jumpButtonPressed = false;
	//updateProjPos();
	
	
	int k;
	for (k = 0; k < MAGICSIZE; k++){
		demon.spells[k].isVisible = false;
		
		demon.spells[k].speed = 1;
		demon.spells[k].distance = 0;
		demon.spells[k].xPos = demon.xPos;
		demon.spells[k].yPos = demon.yPos;
		demon.spells[k].initialized = false;
		
		demon.spells[k].box.x = demon.spells[k].xPos;
		demon.spells[k].box.y = demon.spells[k].yPos;
		demon.spells[k].box.w = 24;
		demon.spells[k].box.h = 23;
	}
	
	//magicball.xPos = demon.xPos;
	//magicball.yPos = demon.yPos;
	//projectileAABB.x = magicball.xPos;
	//projectileAABB.y = magicball.yPos;
	//projectileAABB.w = 24;
	//projectileAABB.h = 23;
	//magicball.isVisible = false;
	//magicball.speed = 1;
	//magicball.distance = 0;


//{ setting up animations
	int i, j;
	// Setting up main character and its animation.
	//demon idle animation
	//AnimationDef demonIdleAnim;
	demonIdleAnim.totalFrames = 8;
	for (i = 0; i < 8; i++){
		demonIdleAnim.frame[i].texIndex = i;
		demonIdleAnim.frame[i].duration = .4;  
	}	

	//AnimationDef demonIdleAnim2;
	demonIdleAnim2.totalFrames = 8;
	for (i = 0; i < 8; i++){
		demonIdleAnim2.frame[i].texIndex = 61 + i;
		demonIdleAnim2.frame[i].duration = .4;  
	}
	
	/*
	demonIdleAnim.frame[0].texIndex = 0;
	demonIdleAnim.frame[0].duration = 1.0;
	demonIdleAnim.frame[1].texIndex = 1;
	demonIdleAnim.frame[1].duration = 1.0;	
	*/
	//AnimationDef projectileAnim;
	projectileAnim.totalFrames = 2;
	projectileAnim.frame[0].texIndex = 59;
	projectileAnim.frame[0].duration = 0.3;
	projectileAnim.frame[1].texIndex = 60;
	projectileAnim.frame[1].duration = 0.3;
	
	//demon move animation
	//AnimationDef demonMoveAnim;
	demonMoveAnim.totalFrames = 8;
	for (i = 0; i < 8; i++){
		demonMoveAnim.frame[i].texIndex = i + 8;
		demonMoveAnim.frame[i].duration = .5;
	}
	
	//demon attack animation
	AnimationDef demonAttackAnim;
	demonAttackAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		demonAttackAnim.frame[i].texIndex = i + 16;
		demonAttackAnim.frame[i].duration = .5;
	}
	
	//demon dying animation
	AnimationDef demonDyingAnim;
	demonDyingAnim.totalFrames = 15;
	for (i = 0; i < 15; i++){
		demonDyingAnim.frame[i].texIndex = i + 27;
		demonDyingAnim.frame[i].duration = .1;
	}
	
	// setting up enemies and their animations
	//AnimationDef catIdleAnim;
	catIdleAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		catIdleAnim.frame[i].texIndex = i + 43;
		catIdleAnim.frame[i].duration = 0.2;
	}
	// cat dead
	AnimationDef catDeadAnim;
	catDeadAnim.totalFrames = 2;
	catDeadAnim.frame[0].texIndex = 54;
	catDeadAnim.frame[0].duration = .7;
	catDeadAnim.frame[1].texIndex = 53;
	catDeadAnim.frame[1].duration = .9;
	
	// cat walking
	//AnimationDef catWalkAnim;
	catWalkAnim.totalFrames = 3;
	catWalkAnim.frame[0].texIndex = 56;
	catWalkAnim.frame[0].duration = .2;
	catWalkAnim.frame[1].texIndex = 57;
	catWalkAnim.frame[1].duration = .2;
	catWalkAnim.frame[2].texIndex = 58;
	catWalkAnim.frame[2].duration = .2;
	
	//cat walking reverse
	//AnimationDef catWalkRAnim;
	catWalkRAnim.totalFrames = 3;
	catWalkRAnim.frame[0].texIndex = 79;
	catWalkRAnim.frame[0].duration = .2;
	catWalkRAnim.frame[1].texIndex = 80;
	catWalkRAnim.frame[1].duration = .2;
	catWalkRAnim.frame[2].texIndex = 81;
	catWalkRAnim.frame[2].duration = .2;
	
	//reverse idle
	//AnimationDef catIdleRAnim;
	catIdleRAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		catIdleRAnim.frame[i].texIndex = i + 69;
		catIdleRAnim.frame[i].duration = 0.2;
	}

	//evil cat walking reverse
	//AnimationDef evilcatWalkRAnim;
	evilcatWalkRAnim.totalFrames = 3;
	evilcatWalkRAnim.frame[0].texIndex = 92;
	evilcatWalkRAnim.frame[0].duration = .2;
	evilcatWalkRAnim.frame[1].texIndex = 93;
	evilcatWalkRAnim.frame[1].duration = .2;
	evilcatWalkRAnim.frame[2].texIndex = 94;
	evilcatWalkRAnim.frame[2].duration = .2;
	
	//evil cat reverse idle
	//AnimationDef evilcatIdleRAnim;
	evilcatIdleRAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		evilcatIdleRAnim.frame[i].texIndex = i + 82;
		evilcatIdleRAnim.frame[i].duration = 0.2;
	}
	
	//evil cat walking 
	//AnimationDef evilcatWalkAnim;
	evilcatWalkAnim.totalFrames = 3;
	evilcatWalkAnim.frame[0].texIndex = 95;
	evilcatWalkAnim.frame[0].duration = .2;
	evilcatWalkAnim.frame[1].texIndex = 96;
	evilcatWalkAnim.frame[1].duration = .2;
	evilcatWalkAnim.frame[2].texIndex = 97;
	evilcatWalkAnim.frame[2].duration = .2;
	
	//evil cat idle
	//AnimationDef evilcatIdleAnim;
	evilcatIdleAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		evilcatIdleAnim.frame[i].texIndex = i + 98;
		evilcatIdleAnim.frame[i].duration = 0.2;
	}
	
	//evil cat love
	//AnimationDef evilcatLoveAnim;
	evilcatLoveAnim.totalFrames = 5;
	for (i = 0; i < 5; i++){
		evilcatLoveAnim.frame[i].texIndex = i + 108;
		evilcatLoveAnim.frame[i].duration = 0.2;
	}
	
	//evil cat love reverse
	//AnimationDef evilcatLoveRAnim;
	evilcatLoveRAnim.totalFrames = 5;
	for (i = 0; i < 5; i++){
		evilcatLoveRAnim.frame[i].texIndex = i + 113;
		evilcatLoveRAnim.frame[i].duration = 0.2;
	}
	
	//cat attack
	//AnimationDef catAttack;
	catAttack.totalFrames = 2;
	catAttack.frame[0].texIndex = 56;
	catAttack.frame[0].duration = .4;
	catAttack.frame[1].texIndex = 55;
	catAttack.frame[1].duration = .4;
	
	//cat attack reverse
	//AnimationDef catAttackR;
	catAttackR.totalFrames = 2;
	catAttackR.frame[0].texIndex = 69;
	catAttackR.frame[0].duration = .4;
	catAttackR.frame[1].texIndex = 118;
	catAttackR.frame[1].duration = .4;
	
	//coin
	//AnimationDef coinAnim;
	coinAnim.totalFrames = 4;
	coinAnim.frame[0].texIndex = 129;
	coinAnim.frame[0].duration = .4;
	coinAnim.frame[1].texIndex = 130;
	coinAnim.frame[1].duration = .4;
	coinAnim.frame[2].texIndex = 131;
	coinAnim.frame[2].duration = .4;
	coinAnim.frame[3].texIndex = 132;
	coinAnim.frame[3].duration = .4;
	
	//catsplode
	//AnimationDef catSplodeAnim;
	catSplodeAnim.totalFrames = 4;
	catSplodeAnim.frame[0].texIndex = 133;
	catSplodeAnim.frame[0].duration = .4;
	catSplodeAnim.frame[1].texIndex = 134;
	catSplodeAnim.frame[1].duration = .4;
	catSplodeAnim.frame[2].texIndex = 135;
	catSplodeAnim.frame[2].duration = .4;
	catSplodeAnim.frame[3].texIndex = 136;
	catSplodeAnim.frame[3].duration = .4;

	//demon eat
	//AnimationDef demonEatAnim;
	demonEatAnim.totalFrames = 2;
	demonEatAnim.frame[0].texIndex = 137;
	demonEatAnim.frame[0].duration = .4;
	demonEatAnim.frame[1].texIndex = 138;
	demonEatAnim.frame[1].duration = .4;
	
	//potion
	//AnimationDef potionAnim;
	potionAnim.totalFrames = 2;
	potionAnim.frame[0].texIndex = 157;
	potionAnim.frame[0].duration = .7;
	potionAnim.frame[1].texIndex = 158;
	potionAnim.frame[1].duration = .7;

	//AnimationDef bcIdleAnim;
	bcIdleAnim.totalFrames = 4;
	bcIdleAnim.frame[0].texIndex = 160;
	bcIdleAnim.frame[0].duration = .4;
	bcIdleAnim.frame[1].texIndex = 161;
	bcIdleAnim.frame[1].duration = .4;
	bcIdleAnim.frame[2].texIndex = 162;
	bcIdleAnim.frame[2].duration = .4;
	bcIdleAnim.frame[3].texIndex = 163;
	bcIdleAnim.frame[3].duration = .4;
	
	//AnimationDef bcAttackAnim;
	bcAttackAnim.totalFrames = 8;
	bcAttackAnim.frame[0].texIndex = 164;
	bcAttackAnim.frame[0].duration = .4;
	bcAttackAnim.frame[1].texIndex = 165;
	bcAttackAnim.frame[1].duration = .4;
	bcAttackAnim.frame[2].texIndex = 166;
	bcAttackAnim.frame[2].duration = .4;
	bcAttackAnim.frame[3].texIndex = 167;
	bcAttackAnim.frame[3].duration = .4;
	bcAttackAnim.frame[4].texIndex = 168;
	bcAttackAnim.frame[4].duration = .4;
	bcAttackAnim.frame[5].texIndex = 169;
	bcAttackAnim.frame[5].duration = .4;
	bcAttackAnim.frame[6].texIndex = 170;
	bcAttackAnim.frame[6].duration = .4;
	bcAttackAnim.frame[7].texIndex = 171;
	bcAttackAnim.frame[7].duration = .4;
	
	//AnimationDef bcDieAnim;
	bcDieAnim.totalFrames = 9;
	bcDieAnim.frame[0].texIndex = 172;
	bcDieAnim.frame[0].duration = .4;
	bcDieAnim.frame[1].texIndex = 173;
	bcDieAnim.frame[1].duration = .4;
	bcDieAnim.frame[2].texIndex = 174;
	bcDieAnim.frame[2].duration = .4;
	bcDieAnim.frame[3].texIndex = 175;
	bcDieAnim.frame[3].duration = .4;
	bcDieAnim.frame[4].texIndex = 176;
	bcDieAnim.frame[4].duration = .4;
	bcDieAnim.frame[5].texIndex = 177;
	bcDieAnim.frame[5].duration = .4;
	bcDieAnim.frame[6].texIndex = 178;
	bcDieAnim.frame[6].duration = .4;
	bcDieAnim.frame[7].texIndex = 179;
	bcDieAnim.frame[7].duration = .4;
	bcDieAnim.frame[8].texIndex = 180;
	bcDieAnim.frame[8].duration = 5;

	
//}	
	
	//set up enemy position
	/*
	cat.xPos = 200;
	cat.yPos = 100;
	cat.prevX = cat.xPos;
	cat.prevY = cat.yPos;	
	
	cat2.xPos = camera.xPos + 550; //top platform
	cat2.yPos = camera.yPos - 70;
	cat2.prevX = cat2.xPos;
	cat2.prevY = cat2.yPos;
	
	cat3.xPos = camera.xPos + 250;
	cat3.yPos = camera.yPos + 140;
	cat3.prevX = cat3.xPos;
	cat3.prevY = cat3.yPos;
	cat3.hurt = false;
	cat3.isGrounded = true;
	
	cat4.xPos = camera.xPos + 200;
	cat4.yPos = camera.yPos + 420;
	cat4.prevX = cat4.xPos;
	cat4.prevY = cat4.yPos;
	cat4.hurt = false;
	cat4.isGrounded = true;

	
	enemies[0] = cat;
	enemies[1] = cat2;
	enemies[2] = cat3;
	enemies[3] = cat4;
	*/
	
	//set up boss cat
		//Idle
		bossCat.actions[0].timer = 5000;
		bossCat.actions[0].weight = .15;
		bossCat.actions[0].behaviorMaxTime = 5000;
		
		//chase
		bossCat.actions[1].timer = 3000;
		bossCat.actions[1].weight = .45;
		bossCat.actions[1].behaviorMaxTime = 3000;
		
		//attack
		bossCat.actions[3].timer = 5000;
		bossCat.actions[3].weight = .85;
		bossCat.actions[3].behaviorMaxTime = 5000;
		
		//enemy intial values
		bossCat.health = 300;
		bossCat.evil = false;
		bossCat.hurt = false;
		
		bossCat.moveSpeed = 0.1;
		bossCat.currentBehavior = bossCat.actions[0];
		bossCat.currentBehavior.value = 0;
		bossCat.behaviorTimer = bossCat.currentBehavior.behaviorMaxTime;
	
	//int k;	
	srand((unsigned) time(&t)); //RANDOM
	//initialize enemy variables
	for (k = 0; k < ENEMYSIZE; k++){
		int a; //iterate through and set up possible actions (7)
		//index name(timer, weight)
		//0 idle(100, .15), 1 chase(200, .45), 2 run(200, .55), 
		//3 attack(100, .85), 4 evil roam(100, .50), 5 evil hug(100, .80)
		
		//Idle
		enemies[k].actions[0].timer = 4000;
		enemies[k].actions[0].weight = .15;
		enemies[k].actions[0].behaviorMaxTime = 4000;
		
		//chase
		enemies[k].actions[1].timer = 5000;
		enemies[k].actions[1].weight = .45;
		enemies[k].actions[1].behaviorMaxTime = 5000;
		
		//run
		enemies[k].actions[2].timer = 5000;
		enemies[k].actions[2].weight = .55;
		enemies[k].actions[2].behaviorMaxTime = 5000;
		
		//attack
		enemies[k].actions[3].timer = 5000;
		enemies[k].actions[3].weight = .85;
		enemies[k].actions[3].behaviorMaxTime = 5000;
		
		//evil roam
		enemies[k].actions[4].timer = 5000;
		enemies[k].actions[4].weight = .30;
		enemies[k].actions[4].behaviorMaxTime = 5000;
		
		//evil idle
		enemies[k].actions[5].timer = 5000;
		enemies[k].actions[5].weight = .60;
		enemies[k].actions[5].behaviorMaxTime = 5000;
		
		//evil hug
		enemies[k].actions[6].timer = 5000;
		enemies[k].actions[6].weight = 1.00;
		enemies[k].actions[6].behaviorMaxTime = 5000;
		
		//enemy intial values
		enemies[k].health = 100;
		enemies[k].evil = false;
		enemies[k].hurt = false;
		
		enemies[k].moveSpeed = 0.1;
		enemies[k].direction = rand();
		enemies[k].currentBehavior = enemies[k].actions[0];
		enemies[k].currentBehavior.value = 0;
		enemies[k].behaviorTimer = enemies[k].currentBehavior.behaviorMaxTime;
		enemies[k].patrolDistance = 0;
		enemies[k].maxPatrolDistance = 1024;
		//float printstate = enemies[k].behaviorTimer;
		//printf("%d ", printstate);
	}

	//cat.xPos = GRIDWIDTH - 100;
	//cat.yPos = GRIDHEIGHT - 100;
	//demon.xPos = playerStartX;//camera.xPos;
	//demon.yPos = playerStartY;//camera.yPos + 390;
	//demon.prevX = demon.xPos;
	//demon.prevY = demon.yPos;
	//if (demon.isWalking){
			
		setAnimation(&demon.animData, &demonIdleAnim);
		
		setAnimation(&bossCat.animData, &bcAttackAnim);

		for (k = 0; k < MAGICSIZE; k++){
			setAnimation(&demon.spells[k].animData, &projectileAnim);
		}
		
		for (k = 0; k < ENEMYSIZE; k++){
			setAnimation(&enemies[k].animData, &catIdleAnim);
		}
		for (k = 0; k < POTIONSIZE; k++){
			setAnimation(&potions[k].animData, &potionAnim);
		}
		

	
	// Mapping.
	int mapL1[GRIDHEIGHT][GRIDWIDTH] = {
		8,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,9,
		6,3,3,2,2,3,3,3,2,3, 3,3,3,2,3,3,3,3,3,2, 3,3,2,3,2,3,3,3,3,3, 3,2,3,3,3,3,3,3,2,7,
		6,3,3,2,3,3,2,3,3,3, 2,3,3,3,3,3,3,3,3,3, 2,3,3,3,3,3,3,3,3,3, 3,3,3,3,2,3,3,2,3,7,  
		6,2,3,3,2,3,3,3,3,3, 3,3,3,3,3,3,2,3,3,3, 3,3,3,3,2,3,3,3,3,3, 3,2,3,3,3,3,3,3,2,7,
		6,3,3,3,3,3,2,3,3,3, 2,3,3,3,2,3,3,3,3,2, 3,2,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,7, 
		6,3,3,2,3,3,3,3,2,3, 3,2,3,2,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,2, 3,3,3,2,3,3,3,3,2,7,
		6,3,3,3,3,3,2,3,3,3, 2,3,3,3,3,3,2,3,3,3, 3,3,3,3,2,3,3,2,3,3, 3,3,3,3,2,3,3,2,3,7, 
		6,3,3,3,2,3,3,3,2,3, 3,2,3,2,3,3,3,2,3,3, 3,3,2,3,3,3,3,3,3,3, 3,3,3,2,3,3,3,3,2,7,
		6,3,3,2,3,3,2,3,3,3, 2,3,3,3,3,2,3,3,3,2, 3,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,2,3,7, 
		6,3,3,2,2,3,3,3,2,3, 3,3,3,2,3,3,3,3,3,3, 3,3,2,3,2,3,3,3,3,3, 3,2,3,2,3,3,3,3,2,7,
		
		6,2,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,2,3,3,3,2,3, 3,3,3,3,3,3,3,2,3,7,
		6,3,3,3,3,3,3,3,3,3, 3,3,3,2,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,3, 3,2,3,2,3,3,3,3,2,7,
		6,3,3,2,3,3,2,3,3,3, 2,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,2,3,2,3, 3,3,3,3,2,3,3,2,3,7,  
		6,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,3,3,3, 3,3,2,3,2,3,3,3,3,3, 2,3,3,2,3,3,3,3,3,7,
		6,2,3,3,3,3,3,3,3,3, 3,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,3,2,7,
		6,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,3,3,2, 3,3,2,3,3,3,3,3,3,3, 2,3,3,3,3,3,3,3,3,7,
		6,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,2,3,3, 3,3,3,3,2,3,3,2,3,3, 3,2,3,3,3,2,3,2,2,7,
		6,2,3,2,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,3, 3,3,3,3,3,3,2,3,3,3, 3,3,3,2,3,3,3,3,3,7,
		6,3,3,3,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,2, 3,2,3,2,3,2,3,3,3,3, 3,3,2,3,3,3,2,3,2,7,
		6,3,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,2,3,7,
		
		6,2,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,2,3,3,3,2,3, 3,3,3,3,3,3,3,2,3,7,
		6,3,3,3,3,3,3,3,3,3, 3,3,3,2,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,3, 3,2,3,2,3,3,3,3,2,7,
		6,3,3,2,3,3,2,3,3,3, 2,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,2,3,2,3, 3,3,3,3,2,3,3,2,3,7,  
		6,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,3,3,3, 3,3,2,3,2,3,3,3,3,3, 2,3,3,2,3,3,3,3,3,7,
		6,2,3,3,3,3,3,3,3,3, 3,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,3,2,7,
		6,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,3,3,2, 3,3,2,3,3,3,3,3,3,3, 2,3,3,3,3,3,3,3,3,7,
		6,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,2,3,3, 3,3,3,3,2,3,3,2,3,3, 3,2,3,3,3,2,3,2,2,7,
		6,2,4,4,4,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,3, 3,3,3,3,3,3,2,3,3,3, 3,3,3,2,3,3,3,3,3,7,
		6,7,5,5,5,6,3,3,3,3, 3,3,2,3,3,3,3,3,3,2, 3,2,3,2,3,2,3,3,3,3, 3,3,2,3,3,3,2,3,2,7,
		6,7,5,5,5,6,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,2,3,7,
		
		6,2,1,1,1,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3, 3,3,3,3,2,3,3,3,2,3, 3,3,3,3,3,3,3,2,3,7,
		6,3,3,3,3,3,3,3,3,3, 3,3,3,2,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,3, 3,2,3,2,3,3,3,3,2,7,
		6,3,3,2,3,3,2,3,3,3, 2,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,2,3,2,3, 3,3,3,3,2,3,3,2,3,7,  
		6,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,3,3,3, 3,3,2,3,2,3,3,3,3,3, 2,3,3,2,3,3,3,3,3,7,
		6,2,3,3,3,3,3,3,3,3, 3,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,3,2,7,
		6,3,3,3,3,2,3,3,3,3, 3,3,3,3,3,3,3,3,3,2, 3,3,2,3,3,3,3,3,3,3, 2,3,3,3,3,3,3,3,3,7,
		6,3,3,3,3,3,3,3,3,2, 3,3,3,3,3,3,3,2,3,3, 3,3,3,3,2,3,3,2,3,3, 3,2,3,3,3,2,3,2,2,7,
		6,2,3,2,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,3, 3,3,3,3,3,3,2,3,3,3, 3,3,3,2,3,3,3,3,3,7,
		6,3,3,3,3,3,3,3,3,3, 3,3,2,3,3,3,3,3,3,2, 3,2,3,2,3,2,3,3,3,3, 3,3,2,3,3,3,2,3,2,7,
		10,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4,4,11,
	};
	int mapL2[GRIDHEIGHT][GRIDWIDTH] = {
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,  
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,  
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,  
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,17,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,  
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
	};	
/*
	int grassMap[GRIDHEIGHT][GRIDWIDTH] = {
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
				
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,24,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,22,23,23,23,23,21,  0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		23,21,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,4 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,4 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,22,23,23,21,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,4 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,4 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,22,21,0 ,0 ,  0 ,1 ,2 ,2 ,8 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  1 ,8 ,5 ,5 ,5 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		2 ,2 ,2 ,3 ,0 ,0 ,0 ,0 ,0 ,1 ,  8 ,5 ,5 ,5 ,5 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
		5 ,5 ,5 ,10,2 ,2 ,2 ,2 ,2 ,8 ,  5 ,5 ,5 ,5 ,5 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,  0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,
	};
*/
	int grassMap2[GRIDHEIGHT][GRIDWIDTH] = {
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
				
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,19,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
	};	
/*
	int collisionsMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1 },
		{ 1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1 },
		{ 1,0,0,0,0,0,1,1,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1 },
		{ 1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0 },
		{ 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};*/
	int collisionsMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,1,1,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,1,1,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,1,1,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1 },
		{ 2,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,1,1,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,3 },
		{ 2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,3 },
		{ 2,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,3 },
		{ 2,0,0,0,0,0,1,1,1,1,0,0,3,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 },
		{ 2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1 },
		{ 2,0,0,1,1,0,0,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0 },
		{ 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,3,2,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,3,2,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,0,0,0,1,1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1,1,1,1,1,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};	
	int objectsMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,2,0,0,1,3,1,0,0,2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,1,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,2,3,1,1,1,1,1,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0 },
		{ 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
		{ 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
		{ 0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,1,0,1,0,1,0,0,1,2,0,1,3,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,2,3,1,0,0,1,0,0,1,1,0,2,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};
/*	
	int grassMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 19,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,22,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,2,2,2,2,2,2,23,23,23,23,23,23,23,23,23,21,0,0,0,22,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,22,23,2,2,23,23,23,21,0,0,0,0,0,12,9,9,9,9,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 19,23,21,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,9,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,22,23,20,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 19,23,21,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,23,23,23,23,23,23,23,23,23,23,23,23,2,2,2,2,3,0,0,22,23,23,23,23,23,23,23,23,23,20 },
		{ 6,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,22,23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,4 },
		{ 19,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,10,2,3,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,10,3,0,0,0,0,0,0,4 },
		{ 6,0,0,22,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,23,23,23,23,23,23,23,23,23,23,23,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,22,2,2,2,2,2,2,2,2,23,23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,9,9,9,9,9,9,9,9,9,9,23,23,21,0,0,0,4 },
		{ 6,0,0,0,0,0,22,23,23,21,0,0,4,5,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,12,9,9,5,5,9,9,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,23,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8 },
		{ 6,0,0,1,3,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,5 },
		{ 23,23,23,23,23,23,3,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,23,21,0,0,0,0,0,0,0,1,2,23,23,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 },
		{ 0,0,0,0,0,0,19,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,0,0,0,0,0,0,0,0,1,8,16,0,0,0,0,4,5,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 },
		{ 0,0,0,0,0,0,23,23,23,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,22,23,23,9,16,0,0,0,0,0,0,0,0,4,16,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,4,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,8,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,22,21,0,0,4,6,0,0,0,0,0,0,0,1,8,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,22,9,9,9,16,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,1,8,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,23,23,23,21,0,0,0,0,0,4,6,0,0,0,0,1,2,8,5,5,5,5,10,23,23,21,0,0,0,0,0,0,0,22,23,21,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,1,8,5,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,10,2,2,2,8,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,22,23,23,21,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,4,10,2,2,2,3,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,9,9,9,9,9,9,9,9,9,9,9,16,0,0,0,0,0,0,1,8,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,22,21,0,0,0,1,2,2,2,8,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 2,2,2,3,0,0,0,0,0,1,8,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 5,5,5,10,2,2,2,2,2,8,5,5,5,5,5,5,10,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0 }
	};
*/
	int grassMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 19,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,22,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,2,2,2,2,2,2,23,23,23,23,23,23,23,23,23,21,0,0,0,22,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,2,2,23,23,23,21,0,0,0,0,0,12,9,9,9,9,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,23,21,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,9,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,20,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,23,21,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,23,23,23,23,23,23,23,23,23,23,23,23,2,2,2,2,3,0,0,22,23,23,23,23,23,23,23,23,23,23 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0 },
		{ 23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,10,2,3,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0 },
		{ 0,0,0,22,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,23,23,23,23,23,23,23,23,23,23,23,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,22,2,2,2,2,2,2,2,2,23,23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,9,9,9,9,9,9,9,9,9,9,23,23,21,0,0,0,0 },
		{ 0,0,0,0,0,0,22,23,23,21,0,0,4,5,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,12,9,9,5,5,9,9,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,23,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2 },
		{ 0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,5 },
		{ 23,23,23,23,23,23,3,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,23,21,0,0,0,0,0,0,0,1,2,23,23,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 },
		{ 0,0,0,0,0,0,19,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,0,0,0,0,0,0,0,0,1,8,16,0,0,0,0,4,5,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 },
		{ 0,0,0,0,0,0,23,23,23,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,22,23,23,9,16,0,0,0,0,0,0,0,0,4,16,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,4,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,8,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,22,21,0,0,4,6,0,0,0,0,0,0,0,1,8,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,22,9,9,9,16,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,1,8,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,23,23,23,23,0,0,0,0,0,4,6,0,0,0,0,1,2,8,5,5,5,5,10,23,23,21,0,0,0,0,0,0,0,22,23,21,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,1,8,5,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,10,2,2,2,8,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,22,23,23,21,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,4,10,2,2,2,3,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,9,9,9,9,9,9,9,9,9,9,9,16,0,0,0,0,0,0,1,8,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,22,21,0,0,0,1,2,2,2,8,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 2,2,2,3,0,0,0,0,0,1,8,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 5,5,5,10,2,2,2,2,2,8,5,5,5,5,5,5,10,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0 }
	};	
	// setting up the map and collisions
	int y,x;
	int decision;
	int totalCoins = COINSIZE;
	int totalCats = ENEMYSIZE;
	int totalPots = POTIONSIZE;
	int c = 0;
	int e = 0;
	int p = 0;
	for (x=0; x<GRIDWIDTH; x++) {
		for (y=0; y<GRIDHEIGHT; y++) {
			//tile[y][x].texIndex = mapL1[y][x];  
			//tile2[y][x].texIndex = mapL2[y][x];
			tile[y][x].texIndex = grassMap[y][x]; 
			objectsTile[y][x].texIndex = objectsMap[y][x];
			collisionTile[y][x].texIndex = collisionsMap[y][x];
			backTile[y][x].texIndex = grassMap2[y][x];
			//setBox(&tile[y][x], x, y);
			//0 no collision, 1 all sides
			/*
			if (collisionTile[y][x].texIndex == 1){ //all
				tile[y][x].collision = 1;
			}
			if (collisionTile[y][x].texIndex == 2){ //left only
				tile[y][x].collision = 2;
			}
			if (collisionTile[y][x].texIndex == 3){//right only
				
			}
			if (collisionTile[y][x].texIndex == 0){
				tile[y][x].collision = 0;
			}
		*/
			if(objectsTile[y][x].texIndex == 1){ //coin
				if (totalCoins > 0){
					tile[y][x].object = 1;
					coins[c].xPos = x*TILE_SIZE + 35; 
					coins[c].yPos = y*TILE_SIZE - 30;
					c++;
					totalCoins -=1;	
				}
			}
			if (objectsTile[y][x].texIndex == 2){
				if (totalCats > 0){
					tile[y][x].object = 2;
					enemies[e].xPos = x*TILE_SIZE + 35;
					enemies[e].yPos = y*TILE_SIZE - 40;
					e++;
					totalCats -=1;
				}
			}
			if (objectsTile[y][x].texIndex == 3){
				if (totalPots > 0){
					tile[y][x].object = 3;
					potions[p].xPos = x*TILE_SIZE;// + 35;
					potions[p].yPos = y*TILE_SIZE - 60;
				
					p++;
					totalPots -=1;
				}
			}
			if (objectsTile[y][x].texIndex == 4){
				tile[y][x].object = 4;
				bossCat.xPos = x*TILE_SIZE;
				bossCat.yPos = y*TILE_SIZE - 134;
				bossStartX = bossCat.xPos;
			}
			if(objectsTile[y][x].texIndex == 0){tile[y][x].object = 0;}
			
			if (tile[y][x].texIndex == 1 || tile[y][x].texIndex == 2 || tile[y][x].texIndex == 3 
			|| tile[y][x].texIndex == 4 || tile[y][x].texIndex == 13 || tile[y][x].texIndex == 14
			|| tile[y][x].texIndex == 15 || tile[y][x].texIndex == 21 || tile[y][x].texIndex == 22  
			|| tile[y][x].texIndex == 23 || tile[y][x].texIndex == 24||tile[y][x].texIndex == 6){
				tile[y][x].collision = 1;	//all collision
				//if (tile[y][x].texIndex != 4 && tile[y][x].texIndex != 23 ){
					//decision = (rand()%10)+1;
					//printf("%i\n", decision);
					//if (decision > 0 && decision < 6 && totalCoins > 0){
					//	tile[y][x].object = 1; //put a coin here
						//printf("coin placed \n");
						//printf("%i\n", totalCoins);
					//	coins[c].xPos = x*TILE_SIZE + 35; 
					//	coins[c].yPos = y*TILE_SIZE - 30;
					//	c++;
					//	totalCoins -=1;		
					//}
				}	
			else
			{
				tile[y][x].collision = 0;
			}
			
				tile[y][x].height = TILE_SIZE;
		}
	}
	
	
	//mapL1[25] = 1;
	
	// set up coins
	//int k;
	//int tx;
	//int ty;
	for (k = 0; k < COINSIZE; k++){
		setAnimation(&coins[k].animData, &coinAnim);
		coins[k].collected = false;
		coins[k].value = 100;

		coins[k].box.w = spriteWidth[129];
		coins[k].box.h = spriteHeight[129];
		coins[k].box.x = coins[k].xPos;
		coins[k].box.y = coins[k].yPos;
	}
	//set up potions
	for (k = 0; k < POTIONSIZE; k++){
		potions[k].collected = false;
		potions[k].value = 50;

		potions[k].box.w = spriteWidth[157];
		potions[k].box.h = spriteHeight[157];
		potions[k].box.x = potions[k].xPos;
		potions[k].box.y = potions[k].yPos;
	}
	
	// Current and previous frame time.
	int prevFrameTime;
	int currFrameTime = SDL_GetTicks();
	float deltaTime;
	int count = 0;
	float physicsDeltaMs = 10;
	float lastPhysicsFrameMs;
	
	// The previous frame's keyboard state.
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};
	// The current frame's keyboard state.
	const unsigned char* kbState = NULL;
	kbState = SDL_GetKeyboardState(NULL);
	

	// The game loop.
	while (isGameLooping) {
		if (GameLoaded == true){
			//do something TODO
		}
        assert(glGetError() == GL_NO_ERROR);
        memcpy(kbPrevState, kbState, sizeof(kbPrevState));
		
		// Get delta time.
		prevFrameTime = currFrameTime;
		currFrameTime = SDL_GetTicks();
		deltaTime = (currFrameTime - prevFrameTime);
        
        // Handle OS message pump.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    isGameLooping = 0;
					break;
				//case SDL_MOUSEBUTTONDOWN:
				//	mousePress(event.button);
				//	break;
				//case SDL_KEYDOWN:
					//switch (event.key.keysym.sym) {
						//case SDLK_ESCAPE:
						//case SDLK_q:
							//isGameLooping = 0;
							//break;
					//}
					//break;
            }
        }
		glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
		
		// Game logic GOES HERE.
        if (kbState[SDL_SCANCODE_ESCAPE]) {
            //isGameLooping = 0;
			gameState = 2;
        }
		mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
		SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
		//SDL_Event e;
		
		if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT)){
			mousePress(mouseX, mouseY);
		}
		
		
		switch(gameState){
		//start menu
		case 0:
			pause = true;
			//glDrawSprite(frameToDraw, x - camera.xPos, y - camera.yPos - 8, w, h); spriteTex[texIndex]
			glDrawSprite(tileTex[26], 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			glDrawSprite(spriteTex[119], 10, 35, spriteWidth[119], spriteHeight[119]);
			glDrawSprite(spriteTex[121], 135, 350, spriteWidth[121], spriteHeight[121]);
			glDrawSprite(spriteTex[120], 335, 350, spriteWidth[120], spriteHeight[120]);
			glDrawSprite(spriteTex[122], 535, 350, spriteWidth[122], spriteHeight[122]);
			SDL_GL_SwapWindow(window);
			break;
			
		// play game
		case 1:
			pause = false;
			//gameState = 4;
			break;
			
		//pause menu
		case 2:
			pause = true;
			glDrawSprite(tileTex[26], 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			glDrawSprite(spriteTex[123], 155, 0, spriteWidth[123], spriteHeight[123]);
			if (Saved == true){
				glDrawSprite(spriteTex[128], 328, 80, spriteWidth[128], spriteHeight[128]);
			}
			else{
				glDrawSprite(spriteTex[127], 328, 80, spriteWidth[127], spriteHeight[127]);
			}
			glDrawSprite(spriteTex[124], 275, 195, spriteWidth[124], spriteHeight[124]);
			glDrawSprite(spriteTex[125], 275, 310, spriteWidth[125], spriteHeight[125]);
			glDrawSprite(spriteTex[126], 275, 425, spriteWidth[126], spriteHeight[126]);
			SDL_GL_SwapWindow(window);
			break;
		case 4:
			break;
	

	}

	if (pause == false){
		
		
			//demon.prevX = demon.xPos;
			//demon.prevY = demon.yPos;
		//physics
		do{
			demon.box.x = demon.xPos;
			demon.box.y = demon.yPos;
			demon.box.h = 91;
			demon.box.w = 60;
			updateAABB();
			//updateProjPos();
			int b,a;



			characterBounds(); //keeps gravity from pulling character into abyss
			
			//cat gravity
			//if (cat3.isGrounded == true){cat3.yVelocity = 0;}
			//if (cat3.isGrounded == true && jumpButtonPressed == true){
			//	cat3.yVelocity = jumpVelocity;
			//	cat3.isGrounded = false;
			//	count++;
				//jumpButtonPressed = false;
			//}
			
			//cat3.yVelocity = cat3.yVelocity + yGravity * physicsDeltaMs;
			//cat3.yPos = cat3.yPos + demon.yVelocity * physicsDeltaMs;
			
			// iterate through the amount of projectiles demon has
			for (a = 0; a < MAGICSIZE; a++){

				if (demon.spells[a].isVisible){//} && demon.spells[a].initialized == true){
					
					//printf("%i\n", demon.magic);
					//if (demon.xVelocity == 1 || demon.xVelocity == 0){
					if (prevVel == 1 || prevVel == 0){
						demon.spells[a].xPos += demon.spells[a].speed *deltaTime;
					}
					else{demon.spells[a].xPos -= demon.spells[a].speed *deltaTime;}
					if (!AABBIntersect(cameraAABB, demon.spells[a].box)){
						//printf("in camera?");
						demon.spells[a].isVisible = false;
						
						//demon.spells[a].initialized = false;						
					}
					//projectileAABB.x = magicball.xPos;
					//projectileAABB.y = magicball.yPos;
					//projectileAABB.w = 28;
					//projectileAABB.h = 27;
					//int stop = magicball.xPosCam + 5;
					updateProjAABB();
					updateAABB();
					//enemy and projectile checking
					for (b = 0; b < ENEMYSIZE; b++){
						if (AABBIntersect(demon.spells[a].box, enemies[b].box) && enemies[b].health > 0){
							//printf("hits enemy\n");
							enemies[b].health -=25;
							demon.spells[a].isVisible = false;
							//demon.spells[a].initialized = false;
							updateProjPos();
						}
					}
					if (AABBIntersect(demon.spells[a].box, bossCat.box) && bossCat.health > 0){
						bossCat.health -=25;
						demon.spells[a].isVisible = false;
						updateProjPos();
					}
					if (bossCat.health == 0){
						gameWon = true;
					}
					/*
					int pTileX = floor(magicball.xPos / TILE_SIZE);
					int pTileY = floor(magicball.yPos/TILE_SIZE);
					Tile pTile = tile[pTileY][pTileX];
					
					AABB pTileBox;
					pTileBox.x = 
					//make a tile box and change all the ref to side tile box to that
					if (pTile.collision == 1){
						if(demon.xVelocity == 1){
							if ((!projectileAABB.x + projectileAABB.w < sideTileBoxR.x) || (!projectileAABB.x > sideTileBoxR.x + sideTileBoxR.w)){
								magicball.isVisible = false;
								updateProjPos();
							}
						}
					}
					if (pTile.collision == 1){
						if(demon.xVelocity == -1){
							if ((!sideTileBoxL.x + sideTileBoxL.w < projectileAABB.x) || (sideTileBoxL.x > projectileAABB.x + projectileAABB.w)){
								magicball.isVisible = false;
								updateProjPos();
							}
						}
					}
					
				*/
				}
			}
		//}

			if (demon.isGrounded == true){demon.yVelocity = 0;}
				if (demon.isGrounded == true && jumpButtonPressed == true){
					demon.yVelocity = jumpVelocity;
					demon.isGrounded = false;
					//count++;
					jumpButtonPressed = false;
			}
			
			demon.yVelocity = demon.yVelocity + yGravity * physicsDeltaMs;
			demon.yPos = demon.yPos + demon.yVelocity * physicsDeltaMs;

			
			// player and enemy collision checking and resolution
			for(b = 0; b < ENEMYSIZE; b++){
				if (AABBIntersect(demon.box, enemies[b].box) && enemies[b].health > 0){
					//going right					
					if (demon.xVelocity == 1){
						demon.xPos = demon.xPos -2;
						cat4.xPos = cat4.xPos +1; //if want cat to bounce
					}
					//going left
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + 2;
						cat4.xPos = cat4.xPos -1;						
					}
					//from above
					if (demon.yVelocity > 0 && demon.isGrounded == false){
						demon.yPos = demon.yPos - 5;
						//demon.xPos = demon.xPos +2;
						//enemies[b].hurt = true;
						//demon.isGrounded = true;
						//demon.yVelocity = 0;
					}
					
				}
			}
			
			// cat boss collision with player
			if (AABBIntersect(demon.box, bossCat.box) && bossCat.health > 0){
				if (demon.xVelocity == 1){
						demon.xPos = demon.xPos -2;
					}
					//going left
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + 2;					
					}
					if (demon.yVelocity > 0 && demon.isGrounded == false){
						demon.yPos = demon.yPos - 5;
					}
			}
			
	/*			
				int	demonGridX = getTileX();
				int	demonGridY = getTileY();	
				int tile_xStart = (int)fmax(demonGridX-2, 0);
				int tile_yStart = (int)fmax(demonGridY-2, 0);
				int tile_xEnd = (int)fmin(demonGridX +1, GRIDWIDTH-1);
				int tile_yEnd = (int)fmin(demonGridY +1, GRIDHEIGHT-1);
				bool pdidCollide = false;
				
				int y, x; 
				for (x = tile_xStart; x <= tile_xEnd; x++){
					for (y = tile_yStart; y <= tile_yEnd; y++){
					tile[y][x].box.x = x*TILE_SIZE;
					tile[y][x].box.y = y*TILE_SIZE;
					tile[y][x].box.w = TILE_SIZE;
					tile[y][x].box.h = TILE_SIZE;
					AABB smallDemonBox;
					smallDemonBox.x = demon.box.x + 10;
					smallDemonBox.y = demon.box.y + 10;
					smallDemonBox.w = demon.box.w - 20;
					smallDemonBox.h = demon.box.h - 40;
						  
					if (tile[y][x].collision == 1){
						if(AABBIntersect(demon.box, tile[y][x].box)){
							
							//pdidCollide = true;				
							//printf("did collide ");
							if (demonGridX < x){ //if demon to left of tile
								//if (demon.xVelocity == 1){
									//demon.xPos = demonGridX*TILE_SIZE;
									//demon.xPos -= 10;
									printf("collide left \n");
								//}
								//demon.xPos -=10; //move to left
							}
							if(demonGridX >= x){ //demon to right of tile
								//demon.xPos +=10; //move to right
								//if (demon.xVelocity == -1){
									//demon.xPos =  x*TILE_SIZE + TILE_SIZE;
									printf("collide right \n");
									//demon.xPos +=10;
								//}
							}
							if (demonGridY < y){ 
								//if (demon.yVelocity > 0){
									//demon.yPos -=5; 
									demon.yPos = (demonGridY*TILE_SIZE);
									demon.yVelocity = 0;
									demon.isGrounded = true;
									printf("collide down \n");
								//}
							}
							if(demonGridY >= y){ 
								demon.yPos +=10; 	
								printf("collide up? \n");
							}
							if(AABBIntersect(smallDemonBox, tile[y][x].box)){
								if (x == demonGridX + 1 && y == demonGridY + 1)
								{
								//demon.xPos -= 10;
								//demon.yPos -= 10;
								//printf("bottom right corner ");
								//demon.xPos = demon.prevX;
								//demon.yPos = demon.prevY;
								}

								if (x == demonGridX - 1 && y == demonGridY + 1)
								{
								//demon.xPos += 10;
								//demon.yPos -= 10;
								//printf("bottom left corner ");
								//demon.xPos = demon.prevX;
								//demon.yPos = demon.prevY;
								}

								if (x == demonGridX - 1 && y == demonGridY - 1)
								{
								//demon.xPos += 10;
								//demon.yPos += 10;
								//printf("top left corner ");
								}

								if (x == demonGridX + 1 && y == demonGridY - 1)
								{
								//demon.xPos += 10;
								//demon.yPos -= 10;
								//printf("top right corner  ");
								}
							
							}
						} 
					  } // end collision
					} // end y
				} //end x
				
				if(pdidCollide == true){
					
					demon.xPos = demon.prevX;
					demon.yPos = demon.prevY;
				}
				*/
			/*	
			//get grid positions
			tileX = getTileX();
			tileY = getTileY();				
			//int gridPosX = tileX;
			//int gridPosY = tileY;
			
			AABB tileBox; // check tile below
			tileBox.x = tileX*TILE_SIZE;
			tileBox.y = (tileY+1)*TILE_SIZE;
			tileBox.w = TILE_SIZE;
			tileBox.h = TILE_SIZE;
			
			AABB sideTileBoxR;
			sideTileBoxR.x = (tileX+1)*TILE_SIZE;
			sideTileBoxR.y = tileY* TILE_SIZE;
			sideTileBoxR.w = TILE_SIZE;
			sideTileBoxR.h = TILE_SIZE;
			
			AABB sideTileBoxL;
			sideTileBoxL.x = (tileX)*TILE_SIZE;
			sideTileBoxL.y = tileY* TILE_SIZE;
			sideTileBoxL.w = TILE_SIZE;
			sideTileBoxL.h = TILE_SIZE;
			
			demon.box.x = demon.xPos;
			demon.box.y = demon.yPos;
			demon.box.h = demon.height;
			
			Tile curTile = tile[tileY+1][tileX];
			Tile curTileR = tile[tileY][tileX+1];
			Tile curTileL = tile[tileY][tileX];
			demon.isGrounded = false;
			//check if demon is colliding
			if (curTile.collision == 1){
				// if demon is NOT above floor or below floor then it's colliding
				//if (!(demon.yPos + demon.height < tileY*TILE_SIZE) || !(demon.yPos > tileY*TILE_SIZE + TILE_SIZE)){
				//if (tileY + TILE_SIZE >= demon.yPos){
				//if (tileY + TILE_SIZE < demon.yPos){ //collides bottom of tile with top of demon
				//if (tileY <= demon.yPos + 91){	
				if (AABBIntersect(demon.box, tileBox)){				
					//demon.yPos = demon.prevY;// - TILE_SIZE;
					//if (!(demon.yPos + demon.height < tileBox.y) || !(demon.yPos > tileBox.y + tileBox.h) && demon.yVelocity > 0 ) {
					//printf("collision\n");
					if (demon.yVelocity > 0){
						demon.yPos = (tileY*TILE_SIZE);
						demon.yVelocity = 0;
						demon.isGrounded = true;	
					}													
				} 


					//demon.yPos = demon.prevY;
					//demon.yPos = (tileY*TILE_SIZE) - demon.height;						
			}
			if (curTileR.collision == 1){
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					if ((!demon.xPos + demon.width < sideTileBoxR.x) || (!demon.xPos > sideTileBoxR.x + sideTileBoxR.w)){
						//printf("Collision ");
						demon.xPos = tileX*TILE_SIZE;
					//demon.yVelocity = 0;
					}
				}
				else{
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (curTileL.collision == 1){
				if(demon.xVelocity == -1){
					// if demon is not to the left or right of the tile to the left
					//if ((!demon.xPos + demon.width < sideTileBoxL.x) || (!demon.xPos > sideTileBoxL.x + sideTileBoxL.w)){
					if ((!sideTileBoxL.x + sideTileBoxL.w < demon.xPos) || (sideTileBoxL.x > demon.xPos + demon.width)){
						//printf("Collision ");
						//demon.xPos = (tileX-1)*TILE_SIZE - TILE_SIZE;
						demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
					//demon.yVelocity = 0;
					}
				}
				else{
					demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
				}
			}
			*/

				
				/*
				for (x = tile_xStart; x < tile_xEnd; x++){
					for (y = tile_yStart; y < tile_yEnd; y++){
						colliding(demon, tile[y][x], x, y);
					}
				}
				*/
			
			
/*
			
			// start of new collision code
			//[t1][t2][t3]
			//[t4][t9][t5]
			//[t6][t7][t8]
			tileX = getTileX();
			tileY = getTileY();
			int	demonGridX = getTileX();
			int	demonGridY = getTileY();	
			t1 = tile[tileY-1][tileX-1];
			t2 = tile[tileY-1][tileX];
			t3 = tile[tileY-1][tileX+1];
			t4 = tile[tileY][tileX-1];
			t5 = tile[tileY][tileX+1];
			t6 = tile[tileY+1][tileX-1];
			t7 = tile[tileY+1][tileX];
			t8 = tile[tileY+1][tileX+1];
			t9 = tile[tileY][tileX];
			setTileBox();
			Tile surrounding[] = {t1,t2,t3,t4,t5,t6,t7,t8,t9};
			demon.isGrounded = false;
			
			AABB tileBox; // check tile below
			tileBox.x = tileX*TILE_SIZE;
			tileBox.y = (tileY+1)*TILE_SIZE;
			tileBox.w = TILE_SIZE;
			tileBox.h = TILE_SIZE;
			
			AABB sideTileBoxR;
			sideTileBoxR.x = (tileX+1)*TILE_SIZE;
			sideTileBoxR.y = tileY* TILE_SIZE;
			sideTileBoxR.w = TILE_SIZE;
			sideTileBoxR.h = TILE_SIZE;
			
			AABB sideTileBoxL;
			sideTileBoxL.x = (tileX)*TILE_SIZE;
			sideTileBoxL.y = tileY* TILE_SIZE;
			sideTileBoxL.w = TILE_SIZE;
			sideTileBoxL.h = TILE_SIZE;
			
			Tile curTile = tile[tileY+1][tileX];
			Tile curTileR = tile[tileY][tileX+1];
			Tile curTileL = tile[tileY][tileX];
			
			bool pdidCollide = false;
			int xOverlap, yOverlap;
			//player collision detection
			for (b = 0; b <= 9; b++){
				if (surrounding[b].collision == 1){
					if (AABBIntersect(demon.box, surrounding[b].box)){
						pdidCollide = true;
						xOverlap = getXOverlap(demon.box, surrounding[b].box);
						//printf("%i\n", xOverlap);
						if (curTileR.collision == 1){
							if(demon.xVelocity == 1){
								if (demonGridX < x){ //if demon to left of tile
							//if (demon.xVelocity == 1){
								demon.xPos = demon.xPos - xOverlap;
							//}
								}
							}
						}
						if(demonGridX >= x){ //demon to right of tile
							//demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
						//if(demon.xVelocity == -1){
							//demon.xPos = demon.xPos - surrounding[b].box.w + xOverlap;
							//demon.xPos = surrounding[b].box.x + surrounding[b].box.w - demon.width;
							//demon.xPos += 10;
						}
						//}
						//printf("collision ");
						if (curTile.collision == 1){	
							if (AABBIntersect(demon.box, tileBox)){				
								if (demon.yVelocity > 0){
									demon.yPos = (tileY*TILE_SIZE);
									demon.yVelocity = 0;
									demon.isGrounded = true;	
								}													
							} 						
						}
					}
				}
			}
			
			for(b = 0; b < ENEMYSIZE; b++){
				int tileEX = floor(enemies[b].xPos / TILE_SIZE);
				int tileEY = floor(enemies[b].yPos / TILE_SIZE);
				setTileBox();
				int a;
				for (a = 0; a < 8; a++){
					if (surrounding[a].collision == 1){
						if (AABBIntersect(enemies[b].box, surrounding[a].box)){
							enemies[b].collided = true;

						}
					}
				}
			}
			//player collision resolution
			//  1
			//4   2
			//  3
			if(pdidCollide == true){
				updateAABB();
				int xOverlap, yOverlap;
				for (b = 0; b <= 9; b++){
					//xOverlap = getXOverlap(demon.box, surrounding[b].box);
					//yOverlap = getYOverlap(demon.box, surrounding[b].box);
					//printf("%i\n", xOverlap);
					//printf("%i\n", yOverlap);
					//demon.xPos = demon.xPos - xOverlap;
					//demon.yPos = demon.yPos + yOverlap;
					//if (demon.yVelocity >  ){
						
					//}
					
				if (curTileR.collision == 1){
					if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
						if ((!demon.xPos + demon.width < sideTileBoxR.x) || (!demon.xPos > sideTileBoxR.x + sideTileBoxR.w)){
						//demon.xPos = tileX*TILE_SIZE;
							demon.xPos = demon.xPos - xOverlap;
						}
					}										
				}
				
				if (curTileL.collision == 1){
				if(demon.xVelocity == -1){
					if ((!sideTileBoxL.x + sideTileBoxL.w < demon.xPos) || !(sideTileBoxL.x > demon.xPos + demon.width)){
					//if(demonGridX >= x){
						//demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
						//printf(" collide ");
					}
				}
				else{
					//demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
					//printf("else ");
				}
			//}
			}
			}
			
			if(pdidCollide == true){
				//demon.xPos = demon.prevX;
				//demon.yPos = demon.prevY;
				//pdidCollide = false;
				//demon.isGrounded = true;
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					//if ((!demon.xPos + demon.width < surrounding[4].box.x) || (!demon.xPos > surrounding[4].box.x + surrounding[4].box.w)){
						//demon.xPos = gridPosX*TILE_SIZE;
					//}
				}
				else{
				//	demon.xPos = gridPosX*TILE_SIZE;
				}
				if(demon.xVelocity == -1){
					//if ((!surrounding[3].box.x + surrounding[3].box.w < demon.xPos) || (surrounding[3].box.x > demon.xPos + demon.width)){
						//surrounding[3].box.x + surrounding[3].box.w - demon.width;
						//demon.xPos = surrounding[3].box.x + surrounding[3].box.w - demon.width;
					//}
				}
				else{
				//	demon.xPos = gridPosX + TILE_SIZE - demon.width;
				}
				//if (AABBIntersect(demon.box, surrounding[6].box)){				
					if (demon.yVelocity > 0){
						//demon.yPos = (gridPosY*TILE_SIZE);
						//demon.yVelocity = 0;
						//demon.isGrounded = true;	
					}													
				//}
				
				if (demon.direction == 4 || demon.direction == 2){
					//demon.xPos = demon.prevX;
				}
				if (demon.direction == 1){
					//demon.yPos = demon.prevY + 20;
					//demon.yVelocity = .05;
					//demon.isGrounded = false;
				}
				if (demon.direction == 3){
					//demon.yPos = demon.prevY;
				}
				if (demon.yVelocity > 0){
					//demon.yPos = demon.prevY;
				//	demon.yVelocity = 0;
				//	demon.isGrounded = true;
				}
				
			}*/
			// end of new collision code
			
			/* Original collision code before redo
						//check if demon is colliding
			if (curTileR.collision == 1){
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					if ((!demon.xPos + demon.width < sideTileBoxR.x) || (!demon.xPos > sideTileBoxR.x + sideTileBoxR.w)){
						demon.xPos = tileX*TILE_SIZE;
					}
				}
				else{
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (curTileL.collision == 1){
				if(demon.xVelocity == -1){
					if ((!sideTileBoxL.x + sideTileBoxL.w < demon.xPos) || (sideTileBoxL.x > demon.xPos + demon.width)){
						demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
					}
				}
				else{
					demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
				}
			}
			if (curTile.collision == 1){	
				if (AABBIntersect(demon.box, tileBox)){				
					if (demon.yVelocity > 0){
						demon.yPos = (tileY*TILE_SIZE);
						demon.yVelocity = 0;
						demon.isGrounded = true;	
					}													
				} 						
			}
		*/
		/*
			//jump physics
			if (demon.isGrounded == true){demon.yVelocity = 0;}
			if (demon.isGrounded == true && jumpButtonPressed == true){
				demon.yVelocity = jumpVelocity;
				demon.isGrounded = false;
				//count++;
				jumpButtonPressed = false;
			}
			
			demon.yVelocity = demon.yVelocity + yGravity * physicsDeltaMs;
			demon.yPos = demon.yPos + demon.yVelocity * physicsDeltaMs;
			//printf("%f\n",demon.yVelocity);	
			*/
			
			int tileX = getTileX();
			int tileY = getTileY();
			AABB tileBox; // check tile below
			tileBox.x = tileX*TILE_SIZE;
			tileBox.y = (tileY+1)*TILE_SIZE;
			tileBox.w = TILE_SIZE;
			tileBox.h = TILE_SIZE;
			
			AABB sideTileBoxR;
			sideTileBoxR.x = (tileX+1)*TILE_SIZE;
			sideTileBoxR.y = tileY* TILE_SIZE;
			sideTileBoxR.w = TILE_SIZE;
			sideTileBoxR.h = TILE_SIZE;
			
			AABB sideTileBoxL;
			sideTileBoxL.x = (tileX)*TILE_SIZE;
			sideTileBoxL.y = tileY* TILE_SIZE;
			sideTileBoxL.w = TILE_SIZE;
			sideTileBoxL.h = TILE_SIZE;
			
			demon.box.x = demon.xPos;
			demon.box.y = demon.yPos;
			demon.box.h = demon.height;
			
			Tile curTile = tile[tileY+1][tileX];
			Tile curTileR = tile[tileY][tileX+1];
			Tile curTileL = tile[tileY][tileX];
			Tile aboveTile = tile[tileY-1][tileX];
			
			//[t1][t2][t3]
			//[t4][t9][t5]
			//[t6][t7][t8]
			t1 = tile[tileY-1][tileX-1];
			t2 = tile[tileY-1][tileX];
			t3 = tile[tileY-1][tileX+1];
			t4 = tile[tileY][tileX-1];
			t5 = tile[tileY][tileX+1];
			t6 = tile[tileY+1][tileX-1];
			t7 = tile[tileY+1][tileX];
			t8 = tile[tileY+1][tileX+1];
			t9 = tile[tileY][tileX];
			setTileBox();
			Tile surrounding[] = {t1,t2,t3,t4,t5,t6,t7,t8,t9};
			
			aboveTile.box.x = tileX*TILE_SIZE;
			aboveTile.box.y = (tileY-1)*TILE_SIZE;
			aboveTile.box.w = TILE_SIZE;
			aboveTile.box.h = TILE_SIZE;
			int xOverlap, yOverlap;
			demon.isGrounded = false;
			
			if (curTileR.collision == 3){
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					if ((!demon.xPos + demon.width < sideTileBoxR.x) || (!demon.xPos > sideTileBoxR.x + sideTileBoxR.w)){
						//printf("Collision ");
						demon.xPos = tileX*TILE_SIZE;
					//demon.yVelocity = 0;
					}
				}
				if(demon.xVelocity == -1){
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (curTileL.collision == 3){
				if(demon.xVelocity == -1){
					// if demon is not to the left or right of the tile to the left
					//if ((!demon.xPos + demon.width < sideTileBoxL.x) || (!demon.xPos > sideTileBoxL.x + sideTileBoxL.w)){
					if ((!sideTileBoxL.x + sideTileBoxL.w < demon.xPos) || (sideTileBoxL.x > demon.xPos + demon.width)){
						//printf("Collision ");
						//demon.xPos = (tileX-1)*TILE_SIZE - TILE_SIZE;
						demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
					//demon.yVelocity = 0;
					}
				}
				if(demon.xVelocity == 1){
					demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
				}
			}	

			if (curTileR.collision == 1){
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					if ((!demon.xPos + demon.width < sideTileBoxR.x) || (!demon.xPos > sideTileBoxR.x + sideTileBoxR.w)){
						//printf("Collision ");
						demon.xPos = tileX*TILE_SIZE;
					//demon.yVelocity = 0;
					}
				}
				if(demon.xVelocity == -1){
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (curTileL.collision == 1){
				if(demon.xVelocity == -1){
					// if demon is not to the left or right of the tile to the left
					//if ((!demon.xPos + demon.width < sideTileBoxL.x) || (!demon.xPos > sideTileBoxL.x + sideTileBoxL.w)){
					if ((!sideTileBoxL.x + sideTileBoxL.w < demon.xPos) || (sideTileBoxL.x > demon.xPos + demon.width)){
						//printf("Collision ");
						//demon.xPos = (tileX-1)*TILE_SIZE - TILE_SIZE;
						demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
					//demon.yVelocity = 0;
					}
				}
				if(demon.xVelocity == 1){
					demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
				}
			}	
			if (curTileL.collision == 2){
				if(demon.xVelocity == -1){
					// if demon is not to the left or right of the tile to the left
					//if ((!demon.xPos + demon.width < sideTileBoxL.x) || (!demon.xPos > sideTileBoxL.x + sideTileBoxL.w)){
					if ((!sideTileBoxL.x + sideTileBoxL.w < demon.xPos) || (sideTileBoxL.x > demon.xPos + demon.width)){
						//printf("Collision ");
						//demon.xPos = (tileX-1)*TILE_SIZE - TILE_SIZE;
						demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
					//demon.yVelocity = 0;
					}
				}
				if(demon.xVelocity == 1){
					demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
				}
			}
			if (curTileR.collision == 2){
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					if ((!demon.xPos + demon.width < sideTileBoxR.x) || (!demon.xPos > sideTileBoxR.x + sideTileBoxR.w)){
						//printf("Collision ");
						demon.xPos = tileX*TILE_SIZE;
					//demon.yVelocity = 0;
					}
				}
				if(demon.xVelocity == -1){
					demon.xPos = tileX*TILE_SIZE;
				}
			}			
			//check if demon is colliding
			if (curTile.collision == 1){
				// if demon is NOT above floor or below floor then it's colliding
				//if (!(demon.yPos + demon.height < tileY*TILE_SIZE) || !(demon.yPos > tileY*TILE_SIZE + TILE_SIZE)){
				//if (tileY + TILE_SIZE >= demon.yPos){
				//if (tileY + TILE_SIZE < demon.yPos){ //collides bottom of tile with top of demon
				//if (tileY <= demon.yPos + 91){	
				if (AABBIntersect(demon.box, tileBox)){				
					//demon.yPos = demon.prevY;// - TILE_SIZE;
					//if (!(demon.yPos + demon.height < tileBox.y) || !(demon.yPos > tileBox.y + tileBox.h) && demon.yVelocity > 0 ) {
					//printf("collision\n");
					if (demon.yVelocity > 0){
						demon.yPos = (tileY*TILE_SIZE);
						demon.yVelocity = 0;
						demon.isGrounded = true;	
					}													
				} 


					//demon.yPos = demon.prevY;
					//demon.yPos = (tileY*TILE_SIZE) - demon.height;						
			}
			if (aboveTile.collision == 1){
				if (AABBIntersect(demon.box, aboveTile.box)){
					//demon.yPos += 1;
					//yOverlap = getYOverlap(demon.box, aboveTile.box);
					//demon.yPos = demon.yPos + yOverlap;
				}
			}
			if (surrounding[5].collision == 1){
				if (AABBIntersect(demon.box, surrounding[5].box)){
					demon.yPos = (tileY*TILE_SIZE);
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (surrounding[0].collision == 1){
				if (AABBIntersect(demon.box, surrounding[0].box)){
					demon.yPos = (tileY*TILE_SIZE);
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (surrounding[7].collision == 1){
				if (AABBIntersect(demon.box, surrounding[7].box)){
					demon.yPos = (tileY*TILE_SIZE);
					demon.xPos = tileX*TILE_SIZE;
					//demon.xPos = sideTileBoxR.x + sideTileBoxR.w - demon.width;
				}
			}
			if (surrounding[2].collision == 1){
				if (AABBIntersect(demon.box, surrounding[2].box)){
					demon.yPos = (tileY*TILE_SIZE);
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			if (surrounding[9].collision == 1){
				if (AABBIntersect(demon.box, surrounding[9].box)){
					demon.yPos = (tileY*TILE_SIZE);
					demon.xPos = tileX*TILE_SIZE;
				}
			}
			
			lastPhysicsFrameMs += physicsDeltaMs;
		}while (lastPhysicsFrameMs + physicsDeltaMs < currFrameTime);
		
		//player movement
		demon.prevX = demon.xPos;
		demon.prevY = demon.yPos;
		if (demon.health > 0){
			moveCharacterPos(kbState, deltaTime); //if character isn't dead allow movement
		}
		if (demon.xVelocity == 1){
			setNewAnimation(&demon.animData, &demonIdleAnim);
		}
		if (demon.xVelocity == -1){
			setNewAnimation(&demon.animData, &demonIdleAnim2);		
		}
		if (demon.health <= 0){
			setNewAnimation(&demon.animData, &demonDyingAnim);
			//death = true;
		}
		updateEnemyPrevPos();
		
		int k;
/*		
		for (k = 0; k < ENEMYSIZE; k++){
			if (enemies[k].hurt == true && !enemies[k].evil == true){
				setAnimation(&enemies[k].animData, &catDeadAnim);
				enemies[k].hurt = false;				
			}
			if (enemies[k].evil == true){
				setAnimation(&enemies[k].animData, &evilcatIdleAnim);
			}
		if (enemies[k].evil == false){	
			if (enemies[k].currentBehavior.value == 0){
				//setAnimation(&enemies[k].animData, &catIdleAnim);
				//setNewAnimation(&enemies[k].animData, &catIdleAnim);
				// Idle
				setEnemyAnimation(k,&catIdleAnim);
				//enemies[k].animData.timeToNextFrame = 0;
				//printf("0 ");
			}
			if (enemies[k].currentBehavior.value == 1){
				//chase 
				//setNewAnimation(&enemies[k].animData, &catWalkAnim);
				if (enemies[k].xPos < demon.xPos){
					//setAnimation(&enemies[k].animData, &catWalkRAnim);
					setEnemyAnimation(k,&catWalkRAnim);
				}
				else{
					setEnemyAnimation(k,&catWalkAnim);
					//setAnimation(&enemies[k].animData, &catWalkAnim);
				}
				//printf("1 ");
			}
			if (enemies[k].currentBehavior.value == 2){
				//run away 
				if (enemies[k].xPos < demon.xPos){
					setEnemyAnimation(k,&catWalkAnim);
					//setNewAnimation(&enemies[k].animData, &catWalkAnim);
				}
				else{
					setEnemyAnimation(k,&catWalkRAnim);
					//setNewAnimation(&enemies[k].animData, &catWalkRAnim);
				}
				//printf("2 ");
			}			
			if (enemies[k].currentBehavior.value == 3){
				//setNewAnimation(&enemies[k].animData, &catAttack);
				//Attack
				if ((demon.xPos - enemies[k].xPos) <= 50 && (demon.yPos - enemies[k].yPos) <= 100){
					if (enemies[k].xPos < demon.xPos){
						setEnemyAnimation(k,&catAttackR);
					}
					else{
						setEnemyAnimation(k,&catAttack);
					}
					//printf("3 ");
				}
				else{
					if (enemies[k].xPos < demon.xPos){
						setEnemyAnimation(k,&catIdleRAnim);
					}
					else{
						setEnemyAnimation(k,&catIdleAnim);
					}
				}
			}	
		} //end of not evil
		if (enemies[k].evil == true){
			if (enemies[k].currentBehavior.value == 4){
				//setNewAnimation(&enemies[k].animData, &evilcatWalkAnim);
				//Evil Wander(!!TODO set a direction to wander in and display here based on that)
				//wander changed to chase for now
				if ((demon.xPos - enemies[k].xPos) <= 100 && (demon.yPos - enemies[k].yPos) <= 200){	
					if (enemies[k].xPos < demon.xPos){
						//setAnimation(&enemies[k].animData, &evilcatWalkRAnim);
						setEnemyAnimation(k,&evilcatWalkRAnim);
					}
					else{
						setEnemyAnimation(k,&evilcatWalkAnim);
						//setAnimation(&enemies[k].animData, &evilcatWalkAnim);
					}
					//printf("4 ");
				}
				else{
					if (enemies[k].xPos < demon.xPos){
						setEnemyAnimation(k,&evilcatIdleRAnim);
						//setAnimation(&enemies[k].animData, &evilcatIdleRAnim);
					}
					else{
						setEnemyAnimation(k,&evilcatIdleAnim);
						//setAnimation(&enemies[k].animData, &evilcatIdleAnim);
					}
				}
			}		
			if (enemies[k].currentBehavior.value == 5){
				//setAnimation(&enemies[k].animData, &evilcatIdleAnim);
				//Evil Idle(!!TODO same as evil wander)
				setEnemyAnimation(k,&evilcatIdleAnim);
				//printf("5 ");
			}		
			if (enemies[k].currentBehavior.value == 6){
				//Evil hug (!!TODO only when close to player)
				if ((demon.xPos - enemies[k].xPos) <= 50 && (demon.yPos - enemies[k].yPos) <= 100){
					if (enemies[k].xPos < demon.xPos){
						setEnemyAnimation(k,&evilcatLoveRAnim);
						//setNewAnimation(&enemies[k].animData, &evilcatLoveRAnim);
					}
					else{
						setEnemyAnimation(k,&evilcatLoveAnim);
						//setNewAnimation(&enemies[k].animData, &evilcatLoveAnim);
					}
					//printf("6 ");
				}
				else{
					if (enemies[k].xPos < demon.xPos){
						setEnemyAnimation(k,&evilcatIdleRAnim);
						//setNewAnimation(&enemies[k].animData, &evilcatIdleRAnim);
					}
					else{
						setEnemyAnimation(k,&evilcatIdleAnim);
						//setNewAnimation(&enemies[k].animData, &evilcatIdleRAnim);
					}
				}
			
			}	
		}// end of evil only
		}*/

		//if (cat3.xPos < )
		setCameraAABB(cameraAABB);
		updateCharacterAABB();
		//updateProjPos();
		for (k = 0; k < ENEMYSIZE; k++){
			iterateAnimation(&enemies[k].animData, deltaTime); 
		}
		for (k = 0; k < COINSIZE; k++){
			iterateAnimation(&coins[k].animData, deltaTime);
		}
		for (k = 0; k < POTIONSIZE; k++){
			iterateAnimation(&potions[k].animData, deltaTime);
		}
		iterateAnimation(&demon.animData, deltaTime);
		for (k = 0; k < MAGICSIZE; k++){
			iterateAnimation(&demon.spells[k].animData, deltaTime); 
		}
		iterateAnimation(&bossCat.animData, deltaTime);
		//moveCharacterPos(kbState, deltaTime);
		updateAABB();
		
		//Shoot code
		int m;
	
		if (kbState[SDL_SCANCODE_SPACE]){
			if (kbPrevState[SDL_SCANCODE_SPACE]){
				spacePressed = false;
			}
			else {
				updateProjPos();
				spacePressed = true;
				prevVel = demon.xVelocity;
				}
			/*
			for (m = 0; m < MAGICSIZE; m++){
				if (demon.magic > 0 && m == 0){
					demon.spells[m].initialized = true;
					demon.spells[m].isVisible = true;
					demon.magic -= 33;
				}
				if (demon.magic > 0 && demon.spells[m-1].initialized == false){
					demon.spells[m].initialized = true;
					demon.spells[m].isVisible = true;
					demon.magic -= 33;
				}
			}
			*/
			//if (demon.magic > 0 && spellNum > 0){
			//	demon.spells[0].isVisible = true;
			//	spellNum--;
				//demon.spells[m].xPos = demon.xPos;
				//demon.spells[m].yPos = demon.yPos;

			//	demon.magic -= 33;
				
			//}
			//updateProjPos(); //put an int in here
			
		}
		if (spacePressed == true){
			shoot();
		}
		//updateCameraPos(kbState, deltaTime);
		//updateCharacterPosInCam();
		updateMagic(deltaTime);
		updateCamera(deltaTime);
		
		//update enemies
		for (k = 0; k < ENEMYSIZE; k++){
			updateEnemy(k, deltaTime);
			if (enemies[k].health <= 0){
				enemies[k].evil = true;
			}
		}
		bossMovement(deltaTime);
		updateCoin();
		updatePotion();
		//cameraFollow(deltaTime);
		
		// Game drawing GOES HERE.
		drawBackground();
		//updateProjDraw();
		
		// draw player
		//if (death == false){
			drawCharacter(&demon.animData, (int)demon.xPos, (int)demon.yPos - 6, spriteWidth[0], spriteHeight[0]);
			
		
		//draw coins
		for(k = 0; k < COINSIZE; k++){
			if (AABBIntersect(cameraAABB, coins[k].box)){
				if (coins[k].collected == false){
					drawCharacter(&coins[k].animData, coins[k].xPos, coins[k].yPos, spriteWidth[129], spriteHeight[129]);

				}
			}
		}
		//draw potions
		for(k = 0; k < POTIONSIZE; k++){
			if (AABBIntersect(cameraAABB, potions[k].box)){
				if (potions[k].collected == false){
					drawCharacter(&potions[k].animData, potions[k].xPos, potions[k].yPos, spriteWidth[157], spriteHeight[157]);
					//printf("gets here ");
				}
			}
		}
		//}
		//draw enemies
		//STOPPED HERE !!
		for(k = 0; k < ENEMYSIZE; k++){
			if (AABBIntersect(cameraAABB, enemies[k].box)){
				//float distancebtw = distance(demon.xPos, enemies[k].xPos, demon.yPos, enemies[k].yPos);
				drawCharacter(&enemies[k].animData, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
				if (enemies[k].evil == true && (enemies[k].currentBehavior.value == 0 || enemies[k].currentBehavior.value == 1 
				|| enemies[k].currentBehavior.value == 2 || enemies[k].currentBehavior.value == 3)){
						newDrawCharacter(&enemies[k].animData, &evilcatIdleAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
				}
				if (enemies[k].evil == false){	
					if (enemies[k].currentBehavior.value == 0){
						// Idle
						newDrawCharacter(&enemies[k].animData, &catIdleAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
					}
					if (enemies[k].currentBehavior.value == 1){
						//chase 
						if (enemies[k].xPos < demon.xPos){
							newDrawCharacter(&enemies[k].animData, &catWalkRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
						}
						else{
							newDrawCharacter(&enemies[k].animData, &catWalkAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
						}
					}
					if (enemies[k].currentBehavior.value == 2){
						//run away 
						if (enemies[k].xPos < demon.xPos){
							newDrawCharacter(&enemies[k].animData, &catWalkAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
						}
						else{
							newDrawCharacter(&enemies[k].animData, &catWalkRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
						}
					}			
					if (enemies[k].currentBehavior.value == 3){
						//Attack
						if ((demon.xPos - enemies[k].xPos) <= 50 && (demon.yPos - enemies[k].yPos) <= 100){
							if (enemies[k].xPos < demon.xPos){
								newDrawCharacter(&enemies[k].animData, &catAttackR, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
							else{
								newDrawCharacter(&enemies[k].animData, &catAttack, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
						}
						else{
							if (enemies[k].xPos < demon.xPos){
								newDrawCharacter(&enemies[k].animData, &catIdleRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
							else{
								newDrawCharacter(&enemies[k].animData, &catIdleAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
						}
					}	
				} //end of not evil	

				if (enemies[k].evil == true){
					if (enemies[k].currentBehavior.value == 4){
						if ((demon.xPos - enemies[k].xPos) <= 100 && (demon.yPos - enemies[k].yPos) <= 200){	
							if (enemies[k].xPos < demon.xPos){
								newDrawCharacter(&enemies[k].animData, &evilcatWalkRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
							else{
								newDrawCharacter(&enemies[k].animData, &evilcatWalkAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
						}
						else{
							if (enemies[k].xPos < demon.xPos){
								newDrawCharacter(&enemies[k].animData, &evilcatIdleRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
							else{
								newDrawCharacter(&enemies[k].animData, &evilcatIdleAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
						}
					}		
					if (enemies[k].currentBehavior.value == 5){
						newDrawCharacter(&enemies[k].animData, &evilcatIdleAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
					}		
					if (enemies[k].currentBehavior.value == 6){
						//Evil hug (!!TODO only when close to player)
						if ((demon.xPos - enemies[k].xPos) <= 50 && (demon.yPos - enemies[k].yPos) <= 100){
							if (enemies[k].xPos < demon.xPos){
								newDrawCharacter(&enemies[k].animData, &evilcatLoveRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
							else{
								newDrawCharacter(&enemies[k].animData, &evilcatLoveAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
						}
						else{
							if (enemies[k].xPos < demon.xPos){
								newDrawCharacter(&enemies[k].animData, &evilcatIdleRAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
							else{
								newDrawCharacter(&enemies[k].animData, &evilcatIdleAnim, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
							}
						}
					
					}	
				}// end of evil only	
				
			}
		}
		if (bossCat.health > 0){ 
			if (bossCat.xPos < demon.xPos){
				drawCharacter(&bossCat.animData, bossCat.xPos+spriteWidth[160], bossCat.yPos, spriteWidth[160]*-1, spriteHeight[160]);
			}else{drawCharacter(&bossCat.animData, bossCat.xPos, bossCat.yPos, spriteWidth[160], spriteHeight[160]);}
		}
		//if(bossCat.attack == true){
		//	newDrawCharacter(&bossCat.animData, bossCat.xPos, bossCat.yPos, spriteWidth[164], spriteHeight[164]);
		//}

		//draw projectiles
		for(k = 0; k < MAGICSIZE; k++){
			if (demon.spells[k].isVisible == true){
				drawCharacter(&demon.spells[k].animData, demon.spells[k].xPos, demon.spells[k].yPos, spriteWidth[59], spriteHeight[59]);		
			}
		}
		drawHUD();
		//reset player animation
		if (!demon.animData.isMoving){
			resetAnimation(&demon.animData);
			//printf("reset animation\n");
		}
		else if (demon.animData.timeToNextFrame <= 0){resetAnimation(&demon.animData);}
		
		if (!bossCat.animData.isMoving){
			resetAnimation(&bossCat.animData);
		}
		
		for(k = 0; k < COINSIZE; k++){
			if (!coins[k].animData.isMoving){
				resetAnimation(&coins[k].animData);
			}
		}
		//reset the enemy animations
		for(k = 0; k < ENEMYSIZE; k++){
			if (!enemies[k].animData.isMoving){
				setAnimation(&enemies[k].animData, enemies[k].animData.animation );
				resetAnimation(&enemies[k].animData);
				enemies[k].animData.frameIndex = 0;
			}
			
		} 
		//reset the potion animations
		for(k = 0; k < POTIONSIZE; k++){
			if (!potions[k].animData.isMoving){
				resetAnimation(&potions[k].animData);
			}
			
		}
		
		//reset the projectiles animations
		for(k = 0; k < MAGICSIZE; k++){
			if (!demon.spells[k].animData.isMoving){
				resetAnimation(&demon.spells[k].animData);
			}
		}

		// Present the most recent frame.
        SDL_GL_SwapWindow(window);
    } //end of game loop
} //end of pause

	// Cleaning up GOES HERE.
	
    SDL_Quit();

    return 0;
}

void drawProjectile(int k){
	if (demon.spells[k].isVisible == true){
		drawCharacter(&demon.spells[k].animData, demon.spells[k].xPos, demon.spells[k].yPos, spriteWidth[59], spriteHeight[59]);		
	}
}
void moveCharacterPos(const unsigned char* kbState, float deltaTime) {
// if (demon.collision == number for collide with ground)
	//move character up
	//demon.prevX = demon.xPos;
	//demon.prevY = demon.yPos;
	if (kbState[SDL_SCANCODE_RIGHT]) {
		if (demon.isGrounded == true){
			demon.xPos += (1*deltaTime);
		}
		else{
			demon.xPos += (0.5*deltaTime);
		}
		demon.isWalking = true;
		characterBounds();
		updateCharacterAABB();
		demon.xVelocity = 1;//pos
		demon.direction = 2;
	}
	if (kbState[SDL_SCANCODE_LEFT]) {
		if (demon.isGrounded == true){
			demon.xPos -= (1*deltaTime);
		}
		else{
			demon.xPos -= (0.5*deltaTime);
		}
		demon.isWalking = true;
		characterBounds();
		updateCharacterAABB();
		demon.xVelocity = -1;//negative
		demon.direction = 4;
	}
	if (kbState[SDL_SCANCODE_UP]) {
		//demon.yPos -= (1*deltaTime);
		demon.isWalking = true;
		jumpButtonPressed = true;
		characterBounds();
		updateCharacterAABB();
		demon.direction = 1;
	}
	else{
		jumpButtonPressed = false;
	}
	if (kbState[SDL_SCANCODE_DOWN]) {
		if (demon.isGrounded == true){
		//	demon.yPos += (1*deltaTime);
		}
		demon.isWalking = true;
		characterBounds();
		updateCharacterAABB();
		//demon.yPos += (1*deltaTime);
		demon.direction = 3;

	}
	
}
int getTileX(){
	return floor(demon.xPos / TILE_SIZE);
}
int getTileY(){
	return floor(demon.yPos / TILE_SIZE);
	//return floor(demon.yPos / t);
}

int getXOverlap(const AABB box1, const AABB box2){
	int ov;
	int xmax2 = box2.x + box2.w;  //amax
	int xmax1 = box1.x + box1.w; //bmax

	
	ov = fmin(xmax2, xmax1) - fmax(box2.x, box1.x);
	if (ov != 0){return ov;}
	else{return 0;}
	
	/*
	//box1 to the right of box2
	if (box1.x > box2.x + box2.w){
		ov = (box1.x + box2.w) - box1.x;
		return ov;
	}
	//box1 to the left of box2
	if (box1.x + box1.w < box2.x){
		ov = (box1.x + box1.w) - box2.x;
		return ov;
	}
	return 0;
	*/
}
int getYOverlap(const AABB box1, const AABB box2){
	int ov;
	int ymax1 = box1.y + box1.h; //bmax
	int ymax2 = box2.y + box2.h; //might have to be subtracted from height not  plus amax
	
	ov = fmin(ymax2, ymax1) - fmax(box2.y, box1.y);
	if (ov != 0){return ov;}
	else{return 0;}
	/*
	//box1 below box2
	if (box1.y > box2.y + box2.h){
		ov = (box2.y + box2.h) - box1.y;
		return ov;
	}
	//box1 above box2
	if (box1.y + box1.h < box2.y){
		ov = (box1.y + box1.h) - box2.y;
		return ov;
	}
	return 0;
	*/
}
/*
void setBox(Tile* t, int xSet, int ySet){
	t.box.x = xSet;
	t.box.y = ySet;
	t.xPos = xSet;
	t.yPos = ySet;
	t.box.w = TILE_SIZE;
	t.box.h = t.height;
}*/
void colliding(Character c, Tile t, int x, int y){ // 2 top collision, 3 one way collision
	bool didCollide;
	if (t.collision == 1){
		//collision on all sides
		didCollide = AABBIntersect(demonAABB, t.box);
		if (didCollide){
			c.yPos = t.yPos;
		}
	}
	if (t.collision == 2){
		//collision from top only?
		if (demon.yPos + demon.height < t.yPos){
			demon.yPos = t.yPos;
		}
	}
	if (t.collision == 3){
		//one way collision?
	}
}

void updateCameraPos(const unsigned char* kbState, float deltaTime) {
	if (kbState[SDL_SCANCODE_D]) {
		camera.xPos += (1*deltaTime);
		cameraBounds();
	}
	if (kbState[SDL_SCANCODE_A]) {
		camera.xPos -= (1*deltaTime);
		cameraBounds();
	}
	if (kbState[SDL_SCANCODE_W]) {
		camera.yPos -= (1*deltaTime);
		cameraBounds();
	}
	if (kbState[SDL_SCANCODE_S]) {
		camera.yPos += (1*deltaTime);
		cameraBounds();
	}
}

void cameraBounds() {
	if (camera.xPos <= 0) {
		camera.xPos = 0;
	}
	if (camera.xPos  >= GRIDWIDTH * TILE_SIZE - WINDOW_WIDTH) {
		camera.xPos = GRIDWIDTH * TILE_SIZE - WINDOW_WIDTH;
	}
	if (camera.yPos <= 0) {
		camera.yPos = 0;
	}
	if (camera.yPos  >= GRIDHEIGHT * TILE_SIZE - WINDOW_HEIGHT) {
		camera.yPos = GRIDHEIGHT * TILE_SIZE - WINDOW_HEIGHT;
	}
}
void characterBounds() {
	if (demon.xPos <= 0 ) {
		demon.xPos = 0 ;
	}
	if (demon.xPos >= GRIDWIDTH * TILE_SIZE - demon.width) {
		demon.xPos = GRIDWIDTH * TILE_SIZE - demon.width;
	}
	if (demon.yPos <= 0) {
		demon.yPos = 0;
	}
	if (demon.yPos >= GRIDHEIGHT * TILE_SIZE - demon.height) {
		demon.yPos = GRIDHEIGHT * TILE_SIZE - demon.height;
	}
}

void enemyBounds(int e){
	if (enemies[e].xPos <= 0 ) {
		enemies[e].xPos = 0;
	}
	if (enemies[e].xPos >= GRIDWIDTH * TILE_SIZE - spriteWidth[43]) {
		enemies[e].xPos = GRIDWIDTH * TILE_SIZE - spriteWidth[43];
	}
	if (enemies[e].yPos <= 0) {
		enemies[e].yPos = 0;
	}
	if (enemies[e].yPos >= GRIDHEIGHT * TILE_SIZE - spriteHeight[43]) {
		enemies[e].yPos = GRIDHEIGHT * TILE_SIZE - spriteHeight[43];
	}
}

void updateCharacterPosInCam() {
	
	demon.xPos = demon.xPos - camera.xPos;
	demon.yPos = demon.yPos - camera.yPos;
}
void setCameraAABB(AABB box){
	box.x = camera.xPos;
	box.y = camera.yPos;
	box.w = camera.width;
	box.h = camera.height;
	
}
void updateCharacterAABB(){
	demon.box.x = demon.xPos;
	demon.box.y = demon.yPos;
	demon.box.w = demon.width;
	demon.box.h = demon.height;
}
bool AABBIntersect(const AABB box1, const AABB box2){
	//box1 to the right of box2
	if (box1.x > box2.x + box2.w){
		return false;
	}
	//box1 to the left of box2
	if (box1.x + box1.w < box2.x){
		return false;
	}
	//box1 below box2
	if (box1.y > box2.y + box2.h){
		return false;
	}
	//box1 above box2
	if (box1.y + box1.h < box2.y){
		return false;
	}
	return true;
}
void updateCamera(float deltaTime) {
	camera.xPos = demon.xPos + demon.width/2 - camera.width/2;
	camera.yPos = demon.yPos + demon.height/2 - camera.height/2;
	
	if (camera.xPos < 0){
		camera.xPos = 0;
	}
	if (camera.yPos < 0){
		camera.yPos = 0;
	}
	if (camera.xPos > GAME_WIDTH - camera.width){
		camera.xPos = GAME_WIDTH - camera.width;
	}
	if (camera.yPos > GAME_HEIGHT - camera.height){
		camera.yPos = GAME_HEIGHT - camera.height;
	}

	}


void drawBackground() {
	
	int tile_xStart = floor(camera.xPos/TILE_SIZE);
	int tile_yStart = floor(camera.yPos/TILE_SIZE);
	int tile_xEnd = floor((camera.xPos + WINDOW_WIDTH)/TILE_SIZE)+1;
	int tile_yEnd = floor((camera.yPos + WINDOW_HEIGHT)/TILE_SIZE)+1;
	
	glDrawSprite(tileTex[25], 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	int y, x;  //y,x
	//for (i=0; i<GRIDHEIGHT; i++) {
	for (x = tile_xStart; x < tile_xEnd; x++){
		//for (j=0; j<GRIDWIDTH; j++) {
		for (y = tile_yStart; y < tile_yEnd; y++){
			/*	
			if (tileTex[backTile[y][x].texIndex] != 0){
				glDrawSprite(tileTex[backTile[y][x].texIndex], 
				TILE_SIZE*x - camera.xPos, 
				TILE_SIZE*y - camera.yPos, 
				TILE_SIZE, TILE_SIZE);
			} */
			
			//setBox(tile[y][x], x, y);
			if (tile[y][x].height != TILE_SIZE){
			glDrawSprite(tileTex[tile[y][x].texIndex], 
				TILE_SIZE*x - camera.xPos, 
				TILE_SIZE*y - camera.yPos + (tile[y][x].height/2 -5), //adjust for the smaller heights
				TILE_SIZE, tile[y][x].height);	
			}
			else{
			glDrawSprite(tileTex[tile[y][x].texIndex], 
				TILE_SIZE*x - camera.xPos, 
				TILE_SIZE*y - camera.yPos, 
				TILE_SIZE, tile[y][x].height);
			}
				/*
				//if (tileTex[backTile[y][x].texIndex] != 0){
				glDrawSprite(tileTex[backTile[y][x].texIndex], 
				TILE_SIZE*x - camera.xPos, 
				TILE_SIZE*y - camera.yPos, 
				TILE_SIZE, TILE_SIZE);
				//} 
				*/
		}
	} 

	/*
	int i, j;
	for (i=0; i<GRIDHEIGHT; i++) {
		for (j=0; j<GRIDWIDTH; j++) {
			
			glDrawSprite(tileTex[tile[j][i].texIndex], 
				TILE_SIZE*j - camera.xPos, 
				TILE_SIZE*i - camera.yPos, 
				TILE_SIZE, TILE_SIZE); 
				
				if (tileTex[tile2[j][i].texIndex] != 0){
				glDrawSprite(tileTex[tile2[j][i].texIndex], 
				TILE_SIZE*j - camera.xPos, 
				TILE_SIZE*i - camera.yPos, 
				TILE_SIZE, TILE_SIZE); 
				}
		}
	} 
	*/

}

void iterateAnimation(AnimationData* animData, float delta) {
	if (!animData->isMoving) return;
	
	int totalFrames = animData->animation->totalFrames;
	animData->timeToNextFrame -= (delta/1000);//0.016;//delta;
	
	if (animData->timeToNextFrame < 0) {
		++animData->frameIndex;
		if (animData->frameIndex >= totalFrames) {
			animData->frameIndex = totalFrames - 1;
			animData->timeToNextFrame = 0;
			animData->isMoving = 0;
		}
		else {
			Frame* currFrame = &animData->animation->frame[animData->frameIndex];
			animData->timeToNextFrame += currFrame->duration;
		}
	}
}

void setAnimation(AnimationData* animData, AnimationDef* animation) {
	animData->animation = animation;
	animData->frameIndex = 0;
	animData->timeToNextFrame = animation->frame[0].duration;
	animData->isMoving = 1;
}
void setNewAnimation(AnimationData* animData, AnimationDef* animation) {
	animData->animation = animation;
	//animData->frameIndex = 0;
	//animData->timeToNextFrame = animation->frame[0].duration;
	//animData->isMoving = 1;
}

void setEnemyAnimation(int e, AnimationDef* animation){
	enemies[e].animData.animation = animation;
	enemies[e].animData.frameIndex = 0;
	//demon.animData.frameIndex = 0;
	enemies[e].animData.timeToNextFrame = animation->frame[0].duration;
	//enemies[e].animData.timeToNextFrame = 0;
	enemies[e].animData.isMoving = 1;
}

void resetAnimation(AnimationData* animData) {
	setAnimation(animData, animData->animation);
}

void drawCharacter(AnimationData* animData, int x, int y, int w, int h) {
	AnimationDef* animation = animData->animation;
	int texIndex = animation->frame[animData->frameIndex].texIndex;
	GLuint frameToDraw = spriteTex[texIndex];
	glDrawSprite(frameToDraw, x - camera.xPos, y - camera.yPos - 8, w, h);
}
void newDrawCharacter(AnimationData* animData, AnimationDef* animation,int x, int y, int w, int h){	
	animData->animation = animation;
	GLuint frameToDraw;
	//animData->frameIndex = 0;
	//animData->timeToNextFrame = animation->frame[0].duration;
	//animData->isMoving = 1;
	int texIndex = animation->frame[animData->frameIndex].texIndex;
	//if (texIndex < 300){
		frameToDraw = spriteTex[texIndex];
	//}
	//else {
	//	texIndex = animation->frame[0].texIndex;
	//	frameToDraw = spriteTex[texIndex];
	//}
	glDrawSprite(frameToDraw, x - camera.xPos, y - camera.yPos - 8, w, h);
}
void drawObject(AnimationData* animData, int x, int y, int w, int h){
	AnimationDef* animation = animData->animation;
	int texIndex = animation->frame[animData->frameIndex].texIndex;
	GLuint frameToDraw = spriteTex[texIndex];
	glDrawSprite(frameToDraw, (x*TILE_SIZE), (y*TILE_SIZE), w, h);
}

//pass in enemy index to roll for, sets a new action and timer
void rollBehavior(int e){
	float rolled = (rand()%100)+1;//1000;
	//printf("%f\n", rolled);
	if (enemies[e].evil == false){
		if(rolled < (enemies[e].actions[0].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[0];
			enemies[e].currentBehavior.value = 0;
			enemies[e].behaviorTimer = enemies[e].actions[0].behaviorMaxTime;
		}
		else if(rolled < (enemies[e].actions[1].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[1];
			enemies[e].currentBehavior.value = 1;
			enemies[e].behaviorTimer = enemies[e].actions[1].behaviorMaxTime;
		}
		else if(rolled < (enemies[e].actions[2].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[2];
			enemies[e].currentBehavior.value = 2;
			enemies[e].behaviorTimer = enemies[e].actions[2].behaviorMaxTime;
		}
		else if(rolled < (enemies[e].actions[3].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[3];
			enemies[e].currentBehavior.value = 3;
			enemies[e].behaviorTimer = enemies[e].actions[3].behaviorMaxTime;
		}
	}
	if(enemies[e].evil == true){
		if(rolled < (enemies[e].actions[4].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[4];
			enemies[e].currentBehavior.value = 4;
			enemies[e].behaviorTimer = enemies[e].actions[4].behaviorMaxTime;
		}
		else if(rolled < (enemies[e].actions[5].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[5];
			enemies[e].currentBehavior.value = 5;
			enemies[e].behaviorTimer = enemies[e].actions[5].behaviorMaxTime;
		}
		else if(rolled < (enemies[e].actions[6].weight) * 100){
			enemies[e].currentBehavior = enemies[e].actions[6];
			enemies[e].currentBehavior.value = 6;
			enemies[e].behaviorTimer = enemies[e].actions[6].behaviorMaxTime;
		}
	}
}
//pass in enemy index and deltatime
void updateAI(int e, float deltaTime){
	if (enemies[e].behaviorTimer <= 0){
		rollBehavior(e);
	}
	else{
		enemies[e].behaviorTimer -= deltaTime;
	}
}
//void makeNewDecision(){}

void executeDecision(int e, float deltaTime){
	enemyBounds(e);
	enemies[e].behaviorTimer -= deltaTime;
	float distancebtw = distance(demon.xPos, enemies[e].xPos, demon.yPos, enemies[e].yPos);
	float deltaX;
	float deltaY;
	int count = 5;
	int r;
	//printf("%f ",deltaTime);
	if (enemies[e].behaviorTimer > 0){
		switch(enemies[e].currentBehavior.value){
			//idle
			case 0: 
				//idle
				//printf("Idle: stand around\n");

				break;
			
			//chase
			case 1: 
				//run after player
				//printf("Chase: run after player\n");
				//if (distance(demon.xPos, enemies[e].xPos, demon.yPos, enemies[e].yPos) < 300){
				if ((demon.xPos - enemies[e].xPos) <= 100 && (demon.yPos - enemies[e].yPos) <= 100){
				//if (distancebtw >= 100){
					//printf("%d ", e);
					//printf("move towards player\n");
					if (enemies[e].xPos < demon.xPos){
						deltaX = ESPEED;
					}
					else{
						deltaX = -ESPEED;
					}
					if (enemies[e].yPos < demon.yPos){
						deltaY = ESPEED;
					}
					else{
						deltaY = -ESPEED;
					}					
					enemies[e].xPos += deltaX * deltaTime;
					//enemies[e].yPos += deltaY * deltaTime;
					}
				
				break;
			
			//run
			case 2: 
				//run away from player
				//printf("Run: run away from player\n");
				//animation in opp dir of player
				if ((demon.xPos - enemies[e].xPos) <= 100 && (demon.yPos - enemies[e].yPos) <= 100){
				//if (distancebtw <= 200){
					//printf("%d ", e);
					//printf("run away from player\n");
					if (enemies[e].xPos < demon.xPos){
						deltaX = -ESPEED;
					}
					else{
						deltaX = ESPEED;
					}	
					enemies[e].xPos += deltaX * deltaTime;
				}
				break;
				
			//attack
			case 3: 
				//attack player
				//printf("Attack: attack player\n");
				if ((demon.xPos - enemies[e].xPos) <= 50 && (demon.yPos - enemies[e].yPos) <= 100){	
				//	if (distancebtw <= 100){
					//printf("%d ", e);
					//printf("attack player\n");
					updateAABB();
					if(AABBIntersect(demon.box, enemies[e].box)){
						//printf("-10 health");
						demon.health -= 20;
					}
				}
				//else enemies[e].behaviorTimer = 0 if it's not close don't attack
				break;
				
			//evil roam
			case 4: 
				//follow as evil (changed from wander for now)
				//r = (rand() % 10) + 1;
				//printf("%d\n", r);
				//printf("Evil Roam: wander as evil\n");
			if ((demon.xPos - enemies[e].xPos) <= 100 && (demon.yPos - enemies[e].yPos) <= 200){	
			//if (distancebtw <= 200){
				if (enemies[e].xPos < demon.xPos){
					deltaX = ESPEED;
					//printf("");
				}
				else if (enemies[e].xPos >= demon.xPos) {
					deltaX = -ESPEED;
				}	
				if (enemies[e].yPos < demon.yPos){
					deltaY = ESPEED;
				}
				else{
					deltaY = -ESPEED;
				}
				enemies[e].xPos += deltaX * deltaTime;
				//enemies[e].yPos += deltaY * deltaTime;
				
			}
				break;
				
			//evil idle
			case 5:
				//idle with evilness
				//printf("Evil idle: idle with evilness\n");

				break;
			//evil hug
			case 6: 
				//hug player
				//printf("Evil hug: hug player with evilness\n");
				if ((demon.xPos - enemies[e].xPos) <= 50 && (demon.yPos - enemies[e].yPos) <= 100){	
				//if (distancebtw <= 100){
					//printf("%d ", e);
					//printf("evil hug player\n");
				}
				break;
		}
	}
}

void updateEnemyPrevPos(){
	cat.prevX = cat.xPos;
	cat.prevY = cat.yPos;
	
	cat2.prevX = cat2.xPos;
	cat2.prevY = cat2.yPos;

	cat3.prevX = cat3.xPos;
	cat3.prevY = cat3.yPos;

	cat4.prevX = cat4.xPos;
	cat4.prevY = cat4.yPos;
}
void updateEnemy(int e, float dt){
	//enemy ai logic
	float deltaX;
	float deltaY;
	/*
	if (enemies[i].xPos < demon.xPos){
		deltaX = ESPEED;
	}
	else{
		deltaX = -ESPEED;
	}
	
	if (enemies[i].yPos < demon.yPos){
		deltaY = ESPEED;
	}
	else{
		deltaY = -ESPEED;
	}
	
	//movement
	enemies[i].xPos += deltaX * dt;
	enemies[i].yPos += deltaY * dt;
	*/
	
	//if need a new decision
	double printstate = enemies[e].behaviorTimer;
	//	printf("%d\n", printstate);
	if (enemies[e].behaviorTimer <= 0){
		rollBehavior(e);
	}
	else{
		executeDecision(e, dt);
	}

}
void updateProjPos(){
	int a;
	for (a = 0; a < MAGICSIZE; a++){
		if (demon.spells[a].isVisible == false){
	//magicball.xPos = demon.xPos;
	//magicball.yPos = demon.yPos + 30;	
		demon.spells[a].xPos = demon.xPos;
		demon.spells[a].yPos = demon.yPos + 30;
		}
	}
}
double toDegrees(double rad){
	return rad * (180.0/M_PI);
}
void setTileBox(){
	t1.box.x = (tileX-1)*TILE_SIZE;
	t1.box.y =(tileY-1)*TILE_SIZE;
	t1.box.w =TILE_SIZE;
	t1.box.h =TILE_SIZE;
	
	t2.box.x = tileX*TILE_SIZE;
	t2.box.y = (tileY-1)*TILE_SIZE;
	t2.box.w = TILE_SIZE;
	t2.box.h = TILE_SIZE;
	
	t3.box.x = (tileX+1)*TILE_SIZE;
	t3.box.y = (tileY-1)*TILE_SIZE;
	t3.box.w = TILE_SIZE;
	t3.box.h = TILE_SIZE;
	
	t4.box.x = (tileX-1)*TILE_SIZE;
	t4.box.y = tileY*TILE_SIZE;
	t4.box.w = TILE_SIZE;
	t4.box.h = TILE_SIZE;
	
	t5.box.x = (tileX+1)*TILE_SIZE;
	t5.box.y = tileY*TILE_SIZE;
	t5.box.w = TILE_SIZE;
	t5.box.h = TILE_SIZE;
	
	t6.box.x = (tileX-1)*TILE_SIZE;
	t6.box.y = (tileY+1)*TILE_SIZE;
	t6.box.w = TILE_SIZE;
	t6.box.h = TILE_SIZE;
	
	t7.box.x = tileX*TILE_SIZE;
	t7.box.y = (tileY+1)*TILE_SIZE;
	t7.box.w = TILE_SIZE;
	t7.box.h = TILE_SIZE;
	
	t8.box.x = (tileX+1)*TILE_SIZE;
	t8.box.y = (tileY+1)*TILE_SIZE;
	t8.box.w = TILE_SIZE;
	t8.box.h = TILE_SIZE;
}
float distance(int x1, int x2, int y1, int y2){
	float distance;
	distance = sqrt((x1-y1)*(x1-y1)+(x2-y2)*(x2-y2));
	return distance;
}
void updateAABB(){
	cameraAABB.x = camera.xPos;
	cameraAABB.y = camera.yPos;
	cameraAABB.w = camera.width;
	cameraAABB.h = camera.height;
	
	int i;
	
	for(i = 0; i < ENEMYSIZE; i++){
		if(enemies[i].hurt == false){
			enemies[i].box.x = enemies[i].xPos +4;
			enemies[i].box.y = enemies[i].yPos + 20;
			enemies[i].box.w = 49-25;
			enemies[i].box.h = 49;
		}
		else{
			enemies[i].box.x = enemies[i].xPos + 4;
			enemies[i].box.y = enemies[i].yPos + 27;
			enemies[i].box.w = 49-25;
			enemies[i].box.h = 49;
		}
	}
	bossCat.box.x = bossCat.xPos;
	bossCat.box.y = bossCat.yPos;
	bossCat.box.w = 146;  
	bossCat.box.h = 176;
}
void updateProjAABB(){
	int i;
	for (i = 0; i < MAGICSIZE; i++){
		demon.spells[i].box.x = demon.spells[i].xPos;
		demon.spells[i].box.y = demon.spells[i].yPos;
		demon.spells[i].box.w = 28;
		demon.spells[i].box.h = 27;
		
	}
}
void resetGame(){
	// Setting up camera.
	// Camera dimen w=480 h=320
	// w=800=160+x+160 h=600=160+y+160
	GameLoaded = false;
	gameWon = false;
	spellNum = 0;
	magicTimer = 1000;
	coinScore = 0;
	bossCat.health = 300;
	spacePressed = false;
	camera.xPos = 0;
	camera.yPos = GRIDHEIGHT * TILE_SIZE - WINDOW_HEIGHT;
	camera.width = WINDOW_WIDTH; // 2;
	camera.height = WINDOW_HEIGHT; // 2;
	
	demon.xPos = playerStartX;
	demon.yPos = playerStartY;
	demon.prevX = demon.xPos;
	demon.prevY = demon.yPos;
	demon.xVelocity = 0;
	
	demon.isWalking = false;
	demon.width = spriteWidth[0];
	demon.height = spriteHeight[0];
	demonAABB.x = demon.xPos;
	demonAABB.y = demon.yPos;
	demonAABB.w = demon.width;
	demonAABB.h = demon.height;
	demon.health = 160;
	demon.magic = 165;
	demon.isGrounded = true;
	jumpButtonPressed = false;
	
	int k;
	for (k = 0; k < MAGICSIZE; k++){
		demon.spells[k].isVisible = false;
		
		demon.spells[k].speed = 1;
		demon.spells[k].distance = 0;
		demon.spells[k].xPos = demon.xPos;
		demon.spells[k].yPos = demon.yPos;
		demon.spells[k].initialized = false;
		
		demon.spells[k].box.x = demon.spells[k].xPos;
		demon.spells[k].box.y = demon.spells[k].yPos;
		demon.spells[k].box.w = 24;
		demon.spells[k].box.h = 23;
	}

	//set up enemy position
	/*
	cat.xPos = 200;
	cat.yPos = 100;
	cat.prevX = cat.xPos;
	cat.prevY = cat.yPos;	
	
	cat2.xPos = camera.xPos + 450;
	cat2.yPos = camera.yPos - 70;
	cat2.prevX = cat2.xPos;
	cat2.prevY = cat2.yPos;
	
	cat3.xPos = camera.xPos + 250;
	cat3.yPos = camera.yPos + 140;
	cat3.prevX = cat3.xPos;
	cat3.prevY = cat3.yPos;
	cat3.hurt = false;
	cat3.isGrounded = true;
	
	cat4.xPos = camera.xPos + 100;
	cat4.yPos = camera.yPos + 420;
	cat4.prevX = cat4.xPos;
	cat4.prevY = cat4.yPos;
	cat4.hurt = false;
	cat4.isGrounded = true;

	
	enemies[0] = cat;
	enemies[1] = cat2;
	enemies[2] = cat3;
	enemies[3] = cat4;
	
	*/
	srand((unsigned) time(&t)); //RANDOM
	//initialize enemy variables
	for (k = 0; k < ENEMYSIZE; k++){
		//index name(timer, weight)
		//0 idle(100, .15), 1 chase(200, .45), 2 run(200, .55), 
		//3 attack(100, .85), 4 evil roam(100, .50), 5 evil hug(100, .80)
		
		//Idle
		enemies[k].actions[0].timer = 5000;
		enemies[k].actions[0].weight = .15;
		enemies[k].actions[0].behaviorMaxTime = 5000;
		
		//chase
		enemies[k].actions[1].timer = 5000;
		enemies[k].actions[1].weight = .45;
		enemies[k].actions[1].behaviorMaxTime = 5000;
		
		//run
		enemies[k].actions[2].timer = 5000;
		enemies[k].actions[2].weight = .55;
		enemies[k].actions[2].behaviorMaxTime = 5000;
		
		//attack
		enemies[k].actions[3].timer = 5000;
		enemies[k].actions[3].weight = .85;
		enemies[k].actions[3].behaviorMaxTime = 5000;
		
		//evil roam
		enemies[k].actions[4].timer = 5000;
		enemies[k].actions[4].weight = .30;
		enemies[k].actions[4].behaviorMaxTime = 5000;
		
		//evil idle
		enemies[k].actions[5].timer = 5000;
		enemies[k].actions[5].weight = .60;
		enemies[k].actions[5].behaviorMaxTime = 5000;
		
		//evil hug
		enemies[k].actions[6].timer = 5000;
		enemies[k].actions[6].weight = 1.00;
		enemies[k].actions[6].behaviorMaxTime = 5000;
		
		//enemy intial values
		enemies[k].health = 100;
		enemies[k].evil = false;
		
		enemies[k].moveSpeed = 0.1;
		enemies[k].direction = rand();
		enemies[k].currentBehavior = enemies[k].actions[0];
		enemies[k].currentBehavior.value = 0;
		enemies[k].behaviorTimer = enemies[k].currentBehavior.behaviorMaxTime;
		enemies[k].patrolDistance = 0;
		enemies[k].maxPatrolDistance = 1024;
		//float printstate = enemies[k].behaviorTimer;
		//printf("%d ", printstate);
	}

		setAnimation(&demon.animData, &demonIdleAnim);

		for (k = 0; k < MAGICSIZE; k++){
			setAnimation(&demon.spells[k].animData, &projectileAnim);
		}
		
		for (k = 0; k < ENEMYSIZE; k++){
			setAnimation(&enemies[k].animData, &catIdleAnim);
		}
		for (k = 0; k < POTIONSIZE; k++){
			setAnimation(&potions[k].animData, &potionAnim);
		}

	int collisionsMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1 },
		{ 1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1 },
		{ 1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1 },
		{ 1,0,0,0,0,0,1,1,1,1,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1 },
		{ 1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0 },
		{ 1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0 },
		{ 0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};
		
	int objectsMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,2,0,0,1,3,1,0,0,2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,1,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,2,3,1,1,1,1,1,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0 },
		{ 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0 },
		{ 0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0 },
		{ 0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,1,0,1,0,1,0,0,1,2,0,1,3,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,0,0,0,0,0,2,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,2,3,1,0,0,1,0,0,1,1,0,2,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
	};		


	int grassMap[70][70] = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 19,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,22,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,2,2,2,2,2,2,23,23,23,23,23,23,23,23,23,21,0,0,0,22,20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,2,2,23,23,23,21,0,0,0,0,0,12,9,9,9,9,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,23,21,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,9,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,20,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,23,21,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,23,23,23,23,23,23,23,23,23,23,23,23,2,2,2,2,3,0,0,22,23,23,23,23,23,23,23,23,23,23 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0 },
		{ 23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,10,2,3,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0 },
		{ 0,0,0,22,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,23,23,23,23,23,23,23,23,23,23,23,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,22,2,2,2,2,2,2,2,2,23,23,23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,9,9,9,9,9,9,9,9,9,9,23,23,21,0,0,0,0 },
		{ 0,0,0,0,0,0,22,23,23,21,0,0,4,5,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,12,9,9,5,5,9,9,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,22,23,23,23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2 },
		{ 0,0,0,1,3,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,5 },
		{ 23,23,23,23,23,23,3,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,23,21,0,0,0,0,0,0,0,1,2,23,23,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2 },
		{ 0,0,0,0,0,0,19,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,0,0,0,0,0,0,0,0,1,8,16,0,0,0,0,4,5,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9 },
		{ 0,0,0,0,0,0,23,23,23,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,1,3,0,0,0,0,0,22,23,23,9,16,0,0,0,0,0,0,0,0,4,16,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,4,19,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,8,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,22,21,0,0,4,6,0,0,0,0,0,0,0,1,8,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,22,9,9,9,16,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,1,8,5,5,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,22,23,23,23,23,23,0,0,0,0,0,4,6,0,0,0,0,1,2,8,5,5,5,5,10,23,23,21,0,0,0,0,0,0,0,22,23,21,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 23,21,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,1,8,5,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,10,2,2,2,8,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,22,23,23,21,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,4,10,2,2,2,3,0,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,9,9,9,9,9,9,9,9,9,9,9,16,0,0,0,0,0,0,1,8,5,5,5,5,10,3,0,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,22,21,0,0,0,1,2,2,2,8,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,5,5,5,10,3,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 2,2,2,3,0,0,0,0,0,1,8,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,8,5,5,5,5,5,5,5,5,5,6,0,0,0,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 5,5,5,10,2,2,2,2,2,8,5,5,5,5,5,5,10,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0 }
	};	

	
	// setting up the map and collisions
	int y,x;
	int decision;
	int totalCoins = COINSIZE;
	int totalCats = ENEMYSIZE;
	int totalPots = POTIONSIZE;
	int c = 0;
	int e = 0;
	int p = 0;
	for (x=0; x<GRIDWIDTH; x++) {
		for (y=0; y<GRIDHEIGHT; y++) {
			//tile[y][x].texIndex = mapL1[y][x];  
			//tile2[y][x].texIndex = mapL2[y][x];
			tile[y][x].texIndex = grassMap[y][x]; 
			objectsTile[y][x].texIndex = objectsMap[y][x];
			collisionTile[y][x].texIndex = collisionsMap[y][x];
			backTile[y][x].texIndex = grassMap2[y][x];
			//setBox(&tile[y][x], x, y);
			//0 no collision, 1 all sides
			if (collisionTile[y][x].texIndex == 1){
				tile[y][x].collision = 1;
			}else{
				tile[y][x].collision = 0;
			}
		
			if(objectsTile[y][x].texIndex == 1){ //coin
				if (totalCoins > 0){
					tile[y][x].object = 1;
					coins[c].xPos = x*TILE_SIZE + 35; 
					coins[c].yPos = y*TILE_SIZE - 30;
					c++;
					totalCoins -=1;	
				}
			}
			if (objectsTile[y][x].texIndex == 2){
				if (totalCats > 0){
					tile[y][x].object = 2;
					enemies[e].xPos = x*TILE_SIZE + 35;
					enemies[e].yPos = y*TILE_SIZE - 40;
					e++;
					totalCats -=1;
				}
			}
			if (objectsTile[y][x].texIndex == 3){
				if (totalPots > 0){
					tile[y][x].object = 3;
					potions[p].xPos = x*TILE_SIZE;// + 35;
					potions[p].yPos = y*TILE_SIZE - 60;
				
					p++;
					totalPots -=1;
				}
			}
			if (objectsTile[y][x].texIndex == 4){
				tile[y][x].object = 4;
				bossCat.xPos = x*TILE_SIZE;
				bossCat.yPos = y*TILE_SIZE - 134;
				bossStartX = bossCat.xPos;
			}
			if(objectsTile[y][x].texIndex == 0){tile[y][x].object = 0;}
			/*
			if (tile[y][x].texIndex == 1 || tile[y][x].texIndex == 2 || tile[y][x].texIndex == 3 
			|| tile[y][x].texIndex == 4 || tile[y][x].texIndex == 13 || tile[y][x].texIndex == 14
			|| tile[y][x].texIndex == 15 || tile[y][x].texIndex == 21 || tile[y][x].texIndex == 22  
			|| tile[y][x].texIndex == 23 || tile[y][x].texIndex == 24){
				tile[y][x].collision = 1;	//all collision
				//if (tile[y][x].texIndex != 4 && tile[y][x].texIndex != 23 ){
					//decision = (rand()%10)+1;
					//printf("%i\n", decision);
					//if (decision > 0 && decision < 6 && totalCoins > 0){
					//	tile[y][x].object = 1; //put a coin here
						//printf("coin placed \n");
						//printf("%i\n", totalCoins);
					//	coins[c].xPos = x*TILE_SIZE + 35; 
					//	coins[c].yPos = y*TILE_SIZE - 30;
					//	c++;
					//	totalCoins -=1;		
					//}
				}	
			else
			{
				tile[y][x].collision = 0;
			}
			*/
				tile[y][x].height = TILE_SIZE;
		}
	}
	
	//mapL1[25] = 1;
	
	// set up coins
	//int k;
	int tx;
	int ty;
	for (k = 0; k < COINSIZE; k++){
		setAnimation(&coins[k].animData, &coinAnim);
		coins[k].collected = false;
		coins[k].value = 100;

		coins[k].box.w = spriteWidth[129];
		coins[k].box.h = spriteHeight[129];
		coins[k].box.x = coins[k].xPos;
		coins[k].box.y = coins[k].yPos;
	}
	//set up potions
	for (k = 0; k < POTIONSIZE; k++){
		potions[k].collected = false;
		potions[k].value = 50;

		potions[k].box.w = spriteWidth[157];
		potions[k].box.h = spriteHeight[157];
		potions[k].box.x = potions[k].xPos;
		potions[k].box.y = potions[k].yPos;
	}
	
	//mapL1[25] = 1;
	updateProjPos();
	// Current and previous frame time.
	int prevFrameTime;
	int currFrameTime = SDL_GetTicks();
	float deltaTime;
	int count = 0;
	float physicsDeltaMs = 10;
	float lastPhysicsFrameMs;
	
	// The previous frame's keyboard state.
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};
	// The current frame's keyboard state.
	const unsigned char* kbState = NULL;
	kbState = SDL_GetKeyboardState(NULL);
}
/*
void mousePress(SDL_MouseButtonEvent& b){
	if(b.button == SDL_BUTTON_LEFT){
		//handle a left click
	}
}*/
void saveGame(){
	SaveFile = fopen("MyGame.txt", "wb"); // opens file for writing and in binary mode
	//Players[0] = demon;
	if (SaveFile == NULL)
	{ 
	  // Error while creating file
	  printf("Error while creating file ");
	}
	/*
	const char SIGNATURE[] = "LNZ"; 
	fwrite (SIGNATURE, sizeof(const char), sizeof(SIGNATURE), SaveFile);
	fwrite(&ENEMIES, sizeof(int), 1, SaveFile);
	fwrite(&PLAYERS, sizeof(int), 1, SaveFile);
	
	fwrite(enemies, sizeof(Enemy), ENEMIES, SaveFile);
	fwrite(Players, sizeof(Character), PLAYERS, SaveFile);
	fclose(SaveFile);
	*/
	fwrite(player, sizeof(*player), 1, SaveFile);
	fwrite(enemies, sizeof(Enemy), ENEMYSIZE, SaveFile);
	fclose(SaveFile);
	
}
void loadGame(){
	LoadFile = fopen ("MyGame.txt", "rb"); // opens file for reading and in binary mode
	//int players, Enemies;
	//Enemy Enemies[10];
	
	if (LoadFile == NULL)
	{ 
	  // Error while creating file
	  printf("Error while creating file ");
	}
	/*
	fread (SIGNATURE, sizeof (char), 4, LoadFile);

	// you should check the signature to see if is equal to LSD

	fread (&players, sizeof(int), 1, SaveFile);
	fread (&Enemies, sizeof(int), 1, SaveFile);
	
	fread (Players, sizeof (Character), players, SaveFile);
	fread (enemies, sizeof (Enemy), Enemies, SaveFile);
	fclose (LoadFile);
	GameLoaded = true;
	*/
	fread(player, sizeof(*player), 1, SaveFile);
	fclose (LoadFile);
	GameLoaded = true;
}
bool checkButton(int bx, int by, int bw, int bh, int mx, int my){
		//mouse click to the right of button
	if (mx > bx + bw){
		return false;
	}
	//mouse click to the left of button
	if (mx < bx){
		return false;
	}
	//mouse click below button
	if (my > by + bh){
		return false;
	}
	//mouse click above button
	if (my < by){
		return false;
	}
	return true;
}

void mousePress(int x, int y){
	
	switch(gameState){
		case 0:
			if(checkButton(135, 300, spriteWidth[121], spriteHeight[121], x, y)){
				//play button pressed
				//printf("play button pressed ");
				gameState = 1;
				break;
			}
			if(checkButton(335, 300, spriteWidth[120], spriteHeight[120], x, y)){
				//load button pressed
				//printf("load button pressed ");
				loadGame();
				gameState = 1;
				break;
			}
			if(checkButton(535, 300, spriteWidth[122], spriteHeight[122], x, y)){
				//quit button pressed
				//printf("quit button pressed ");
				isGameLooping = 0;
				break;
			}
			break;
			
		case 1:
			break;
			
		case 2:
			if(checkButton(328, 80, spriteWidth[127], spriteHeight[127], x, y)){
				//save button pressed
				//printf("save button pressed ");
				Saved = true;
				saveGame();
				break;
			}
			if(checkButton(275, 195, spriteWidth[124], spriteHeight[124], x, y)){
				//resume button pressed
				//printf("resume button pressed ");
				gameState = 1;
				Saved = false;
				break;
			}
			if(checkButton(275, 310, spriteWidth[125], spriteHeight[125], x, y)){
				//restart button pressed
				//printf("restart button pressed ");
				resetGame();
				demon.xPos = camera.xPos;
				demon.yPos = camera.yPos + 390;
				gameState = 1;
				Saved = false;
				break;
			}
			if(checkButton(275, 425, spriteWidth[126], spriteHeight[126], x, y)){
				//exit button pressed
				//printf("exit button pressed ");
				isGameLooping = 0;
				Saved = false;
				break;
			}
			break;
			
		case 3:
			break;
			
		case 4: 
			break;
	}
}
void updateCoin(){
	int i;
	for (i = 0; i < COINSIZE; i++){
		if (AABBIntersect(demon.box, coins[i].box)){
			coins[i].collected = true;
			coinScore += coins[i].value;
			coins[i].value = 0;
		}
	}
}
void updatePotion(){
	int i;
	for (i = 0; i < POTIONSIZE; i++){
		if (AABBIntersect(demon.box, potions[i].box)){
			potions[i].collected = true;
			if(potions[i].value > 0){
				if (demon.health < 110){
					//printf("demon gets health < 110 \n");
					demon.health += potions[i].value;
				}
				if (demon.health >= 110){
					//printf("demon gets health >= 110 \n");
					demon.health = 160;
				}
				potions[i].value = 0;
			}
		}
	}
}
void drawHUD(){
	//draw HUD
	glDrawSprite(spriteTex[139], 0, 0, spriteWidth[139], spriteHeight[139]);
	
	//draw player health bar
	if (demon.health == 160){
		glDrawSprite(spriteTex[140], 63, 16, spriteWidth[140], spriteHeight[140]);
	}
	if (demon.health == 140){
		glDrawSprite(spriteTex[141], 63, 16, spriteWidth[141], spriteHeight[141]);
	}
	if (demon.health == 120){
		glDrawSprite(spriteTex[142], 63, 16, spriteWidth[142], spriteHeight[142]);
	}
	if (demon.health == 100){
		glDrawSprite(spriteTex[143], 63, 16, spriteWidth[143], spriteHeight[143]);
	}
	if (demon.health == 80){
		glDrawSprite(spriteTex[144], 63, 16, spriteWidth[144], spriteHeight[144]);
	}
	if (demon.health == 60){
		glDrawSprite(spriteTex[145], 63, 16, spriteWidth[145], spriteHeight[145]);
	}
	if (demon.health == 40){
		glDrawSprite(spriteTex[146], 63, 16, spriteWidth[146], spriteHeight[146]);
	}
	if (demon.health == 20){
		glDrawSprite(spriteTex[147], 63, 16, spriteWidth[147], spriteHeight[147]);
		glDrawSprite(spriteTex[149], 0, 0, spriteWidth[149], spriteHeight[149]);
	}
	/*
	if (demon.health == 0 || demon.health < 0){
		glDrawSprite(spriteTex[148], 63, 16, spriteWidth[148], spriteHeight[148]);
		glDrawSprite(spriteTex[156], 0, 0, spriteWidth[156], spriteHeight[156]);
	}
	*/
	//draw magic bar
	if (demon.magic == 165){
		glDrawSprite(spriteTex[150], 570, 16, spriteWidth[150], spriteHeight[150]);
	}
	if (demon.magic == 132){
		glDrawSprite(spriteTex[151], 603, 16, spriteWidth[151], spriteHeight[151]);
	}
	if (demon.magic == 99){
		glDrawSprite(spriteTex[152], 636, 16, spriteWidth[152], spriteHeight[152]);
	}
	if (demon.magic == 66){
		glDrawSprite(spriteTex[153], 669, 16, spriteWidth[153], spriteHeight[153]);
	}
	if (demon.magic == 33){
		glDrawSprite(spriteTex[154], 702, 16, spriteWidth[154], spriteHeight[154]);
	}
	if (demon.magic == 0){
		glDrawSprite(spriteTex[155], 735, 16, spriteWidth[155], spriteHeight[155]);
	}
	
	//draw coin count
		glDrawSprite(numTex[0], 654, 562, numWidth[0], numHeight[0]); 	  //     0
		if (coinScore > 0){
			glDrawSprite(numTex[1], 654, 562, numWidth[1], numHeight[1]); //    0 	
		}
			int test1 = coinScore /100;

			switch(test1){
				case 91:
				case 81:
				case 71:
				case 61:
				case 51:
				case 41:
				case 31:
				case 21:
				case 11:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,  
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 1:
					glDrawSprite(numTex[2], 654, 562, numWidth[2], numHeight[2]); 	 //   100 
					break;
				case 92:
				case 82:
				case 72:
				case 62:
				case 52:
				case 42:
				case 32:
				case 22:
				case 12:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1		
				case 2:
					glDrawSprite(numTex[3], 654, 562, numWidth[3], numHeight[3]); 	 //   200
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
					break;
				case 93:
				case 83:
				case 73:
				case 63:
				case 53:
				case 43:
				case 33:
				case 23:
				case 13:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 3:
					glDrawSprite(numTex[4], 654, 562, numWidth[4], numHeight[4]); 	 //   300 
					break;
				case 94:
				case 84:
				case 74:
				case 64:
				case 54:
				case 44:
				case 34:
				case 24:
				case 14:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 4:
					glDrawSprite(numTex[5], 654, 562, numWidth[5], numHeight[5]); 	 //   400 
					break;
				case 95:
				case 85:
				case 75:
				case 65:
				case 55:				
				case 45:
				case 35:
				case 25:
				case 15:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 5:
					glDrawSprite(numTex[6], 654, 562, numWidth[6], numHeight[6]); 	 //   500 
					break;
				case 96:
				case 86:
				case 76:
				case 66:
				case 56:				
				case 46:
				case 36:
				case 26:
				case 16:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 6:
					glDrawSprite(numTex[7], 654, 562, numWidth[7], numHeight[7]); 	 //   600 
					break;
				case 97:
				case 87:
				case 77:
				case 67:
				case 57:				
				case 47:
				case 37:
				case 27:
				case 17:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 7:
					glDrawSprite(numTex[8], 654, 562, numWidth[8], numHeight[8]); 	 //   700 
					break;
				case 98:
				case 88:
				case 78:
				case 68:
				case 58:				
				case 48:
				case 38:
				case 28:
				case 18:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 8:
					glDrawSprite(numTex[9], 654, 562, numWidth[9], numHeight[9]); 	 //   800 
					break;
				case 99:
				case 89:
				case 79:
				case 69:
				case 59:				
				case 49:
				case 39:
				case 29:
				case 19:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  ,
					//glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
				case 9:
					glDrawSprite(numTex[10], 654, 562, numWidth[10], numHeight[10]); //   900 
					break;
				case 10:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
					break;
				case 20:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[13], 654, 562, numWidth[13], numHeight[13]); // 2
					break;
				case 30:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[14], 654, 562, numWidth[14], numHeight[14]); // 3
					break;
				case 40:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[15], 654, 562, numWidth[15], numHeight[15]); // 4
					break;
				case 50:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[16], 654, 562, numWidth[16], numHeight[16]); // 5
					break;
				case 60:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[17], 654, 562, numWidth[17], numHeight[17]); // 6
					break;	
				case 70:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[18], 654, 562, numWidth[18], numHeight[18]); // 7
					break;
				case 80:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[19], 654, 562, numWidth[19], numHeight[19]); // 8
					break;	
				case 90:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[20], 654, 562, numWidth[20], numHeight[20]); // 9
					break;	
				case 100:
					glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
					glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
					glDrawSprite(numTex[21], 654, 562, numWidth[21], numHeight[21]); // 0
					glDrawSprite(numTex[22], 654, 562, numWidth[22], numHeight[22]); //1
					break;		
			}
			
			switch(test1){
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
					glDrawSprite(numTex[12], 654, 562, numWidth[12], numHeight[12]); // 1
					break;
				case 21:
				case 22:
				case 23:
				case 24:
				case 25:
				case 26:
				case 27:
				case 28:
				case 29:
					glDrawSprite(numTex[13], 654, 562, numWidth[13], numHeight[13]); // 2
					break;
				case 31:
				case 32:
				case 33:
				case 34:
				case 35:
				case 36:
				case 37:
				case 38:
				case 39:
					glDrawSprite(numTex[14], 654, 562, numWidth[14], numHeight[14]); // 3
					break;
				case 41:
				case 42:
				case 43:
				case 44:
				case 45:
				case 46:
				case 47:
				case 48:
				case 49:
					glDrawSprite(numTex[15], 654, 562, numWidth[15], numHeight[15]); // 4
					break;
				case 51:
				case 52:
				case 53:
				case 54:
				case 55:
				case 56:
				case 57:
				case 58:
				case 59:
					glDrawSprite(numTex[16], 654, 562, numWidth[16], numHeight[16]); // 5
					break;
				case 61:
				case 62:
				case 63:
				case 64:
				case 65:
				case 66:
				case 67:
				case 68:
				case 69:
					glDrawSprite(numTex[17], 654, 562, numWidth[17], numHeight[17]); // 6
					break;
				case 71:
				case 72:
				case 73:
				case 74:
				case 75:
				case 76:
				case 77:
				case 78:
				case 79:
					glDrawSprite(numTex[18], 654, 562, numWidth[18], numHeight[18]); // 7
					break;
				case 81:
				case 82:
				case 83:
				case 84:
				case 85:
				case 86:
				case 87:
				case 88:
				case 89:
					glDrawSprite(numTex[19], 654, 562, numWidth[19], numHeight[19]); // 8
					break;	
				case 91:
				case 92:
				case 93:
				case 94:
				case 95:
				case 96:
				case 97:
				case 98:
				case 99:
					glDrawSprite(numTex[20], 654, 562, numWidth[20], numHeight[20]); // 9
					break;						
			}
			if (coinScore > 10000){
				glDrawSprite(numTex[11], 654, 562, numWidth[11], numHeight[11]); //  , 
				glDrawSprite(numTex[23], 654, 562, numWidth[23], numHeight[23]); //   000
				glDrawSprite(numTex[21], 654, 562, numWidth[21], numHeight[21]); // 0
				glDrawSprite(numTex[22], 654, 562, numWidth[22], numHeight[22]); //1
			}	
	if (demon.health == 0 || demon.health < 0){
		glDrawSprite(spriteTex[148], 63, 16, spriteWidth[148], spriteHeight[148]);
		glDrawSprite(spriteTex[156], 0, 0, spriteWidth[156], spriteHeight[156]);
	}
	if (gameWon == true){
		glDrawSprite(spriteTex[159], 0, 0, spriteWidth[159], spriteHeight[159]);
	}
}
void updateMagic(float deltaTime){
	//set up a timer
	//when timer is up change to full magic
	if (demon.magic <= 0 && magicTimer <= 0){
		demon.magic = 165;
		spellNum = 0;
		magicTimer = 10000;
	}
	else{
		magicTimer -= deltaTime;
		//printf("%f\n", deltaTime);
	}
}
void shoot(){
	if (spellNum < MAGICSIZE){
		if (demon.magic > 0 ){//&& spellNum == 0){
			//demon.spells[spellNum].initialized = true;
			demon.spells[spellNum].isVisible = true;
			demon.magic -= 33;
			spellNum++;
			spacePressed = false;
		}
		//if (demon.magic > 0 && demon.spells[spellNum-1].initialized == false){
		//	demon.spells[spellNum].initialized = true;
		//	demon.spells[spellNum].isVisible = true;
		//	demon.magic -= 33;
		//}		
	}
	spacePressed = false;
	/*
			for (m = 0; m < MAGICSIZE; m++){
				if (demon.magic > 0 && m == 0){
					demon.spells[m].initialized = true;
					demon.spells[m].isVisible = true;
					demon.magic -= 33;
				}
				if (demon.magic > 0 && demon.spells[m-1].initialized == false){
					demon.spells[m].initialized = true;
					demon.spells[m].isVisible = true;
					demon.magic -= 33;
				}
			}	*/
}
void bossMovement(float dt){
	float deltaX;
	float deltaY;
	int prevCatX;
	int prevCatY;
	float distancebtw = distance(demon.xPos, bossCat.xPos, demon.yPos, bossCat.yPos);
	//if (((bossCat.xPos - demon.xPos) <= 111 )  
	//	&& ((bossCat.yPos - demon.yPos ) <= 63 && (bossCat.yPos - demon.yPos ) >= 113)){
	if (distancebtw < 400){
		if (bossCat.xPos < demon.xPos){
			deltaX = ESPEED;
			prevCatX = bossCat.xPos;
			prevCatX += deltaX*dt;
			//printf("boss cat moved ");
			bossCat.xPos += 2;
		}
		else{
			deltaX = -ESPEED;
			prevCatX = bossCat.xPos;
			prevCatX += deltaX*dt;
			bossCat.xPos -= 2;
			//printf("boss cat moved ");
		}
		
		//if (bossCat.yPos < demon.yPos){
		//	deltaY = ESPEED;
		//}
		//else{
		//	deltaY = -ESPEED;
		//}
	}
	//movement
	//if (prevCatX <= bossStartX + 10){
		//bossCat.xPos += deltaX * dt;
		//bossCat.yPos += deltaY * dt;
	//}
	updateAABB();
	
	if ((demon.xPos - bossCat.xPos) <= 50 && (demon.yPos - bossCat.yPos) <= 100){	
		if(AABBIntersect(demon.box, bossCat.box)){
			demon.health -= 20;
			//printf("boss cat attack ");
		}
	}
}
