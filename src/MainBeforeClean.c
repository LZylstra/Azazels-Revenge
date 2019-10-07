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
#define MAPWIDTH 70
#define MAPHEIGHT 70
#define TILE_SIZE 70
#define GAME_WIDTH 70 * 70
#define GAME_HEIGHT 70 * 70
#define ENEMYSIZE 10
 
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
	Frame frame[100];
	int totalFrames;
} AnimationDef;

// Stores the run-time state of an animation.
typedef struct AnimationData {
	AnimationDef* animation;
	int frameIndex;
	float timeToNextFrame;
	char isMoving;
} AnimationData;

typedef struct Character {
	AnimationData animData;
	int health;
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
	//Projectile[] magic;
	//for platformer
	bool isGrounded;
	double yVelocity;
	double xVelocity;
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
} Tile;

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
}Enemy;
typedef struct Projectile{
	AnimationData animData;
	//int damage;
	int xPos;
	int yPos;
	int xPosCam;
	int yPosCam;
	bool isVisible;
	int speed; 
	int distance;

}Projectile;

// Set this to 0 (false) to make the game loop exit.
char isGameLooping = 1;
char isLtr = 1;
// New.
Tile tile[MAPHEIGHT][MAPWIDTH];
Tile behindTile[MAPHEIGHT][MAPWIDTH];
Tile tile2[MAPHEIGHT][MAPWIDTH];
Tile backTile[MAPHEIGHT][MAPWIDTH];
GLuint tileTex[100];
GLuint spriteTex[100];
int spriteWidth[100];
int spriteHeight[100];
Character demon;
Enemy cat;
Enemy cat2;
Enemy cat3;
Enemy cat4;
Camera camera;
Enemy enemies[ENEMYSIZE]; //int size = sizeof(a) / sizeof(a[0]);
AABB catAABB;
AABB cat2AABB;
AABB cat3AABB;
AABB cat4AABB;
AABB cameraAABB;
AABB demonAABB;
AABB tileAABB;
Projectile magicball;
AABB projectileAABB;
double jumpVelocity = -.045;
double yGravity = 0.000008;
bool jumpButtonPressed;
bool noCollision = true;
bool hasBeenHurt3 = false;
bool hasBeenHurt4= false;

// Functions.
void setAnimation(AnimationData*, AnimationDef*);
void setNewAnimation(AnimationData*, AnimationDef*);
void resetAnimation(AnimationData*);
void iterateAnimation(AnimationData*, float);
void drawCharacter(AnimationData*, int, int, int, int);
void drawBackground();

void moveCharacterPos(const unsigned char*, float);
void characterBounds();
void updateCameraPos(const unsigned char*, int);
void cameraBounds();
void updateCharacterPosInCam();

void updateCamera(int);
bool AABBIntersect(const AABB, const AABB);
void setCameraAABB(AABB);
void updateCharacterAABB();
void updateEnemyPos();
void updateAABB();
void updateProjPos();
void drawProjectile();
void updateProjDraw();

int getTileX();
int getTileY();

void colliding(Character, Tile, int, int);
//void setBox(Tile*, int, int);
int getOverlap(const AABB, const AABB);

int main(void) {
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
	tileTex[9] = glTexImageTGAFile("art/background/toprightcorner.tga", NULL, NULL);
	tileTex[10] = glTexImageTGAFile("art/background/bottomleftcorner.tga", NULL, NULL);
	tileTex[11] = glTexImageTGAFile("art/background/bottomrightcorner.tga", NULL, NULL);
	tileTex[12] = glTexImageTGAFile("art/background/tree1.tga", NULL, NULL);
	//tileTex[13] = glTexImageTGAFile("art/background/tree2.tga", NULL, NULL);
	tileTex[13] = glTexImageTGAFile("art/Platformer/tiles/Tiles/13.tga", NULL, NULL);
	//tileTex[14] = glTexImageTGAFile("art/background/tree3.tga", NULL, NULL);
	tileTex[14] = glTexImageTGAFile("art/Platformer/tiles/Tiles/14.tga", NULL, NULL);
	//tileTex[15] = glTexImageTGAFile("art/background/tree4.tga", NULL, NULL);
	tileTex[15] = glTexImageTGAFile("art/Platformer/tiles/Tiles/15.tga", NULL, NULL);
	tileTex[16] = glTexImageTGAFile("art/background/tree5.tga", NULL, NULL);
	//tileTex[17] = glTexImageTGAFile("art/background/devilsymbol.tga", NULL, NULL);
	tileTex[17] = glTexImageTGAFile("art/Platformer/tiles/Tiles/17.tga", NULL, NULL);
	tileTex[18] = glTexImageTGAFile("art/Platformer/tiles/BG/BG.tga", NULL, NULL);
	
	tileTex[19] = glTexImageTGAFile("art/Platformer/tiles/Object/Bush3.tga", NULL, NULL);
	tileTex[20] = glTexImageTGAFile("art/Platformer/tiles/Tiles/19.tga", NULL, NULL);
	tileTex[21] = glTexImageTGAFile("art/Platformer/tiles/Tiles/20.tga", NULL, NULL);
	
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
	spriteTex[27] = glTexImageTGAFile("art/Demon/die/ddeath0.tga", &spriteWidth[27], &spriteHeight[27]);
	spriteTex[28] = glTexImageTGAFile("art/Demon/die/ddeath1.tga", &spriteWidth[28], &spriteHeight[28]);
	spriteTex[29] = glTexImageTGAFile("art/Demon/die/ddeath2.tga", &spriteWidth[29], &spriteHeight[29]);
	spriteTex[30] = glTexImageTGAFile("art/Demon/die/ddeath3.tga", &spriteWidth[30], &spriteHeight[30]);
	spriteTex[31] = glTexImageTGAFile("art/Demon/die/ddeath4.tga", &spriteWidth[31], &spriteHeight[31]);
	spriteTex[32] = glTexImageTGAFile("art/Demon/die/ddeath5.tga", &spriteWidth[32], &spriteHeight[32]);
	spriteTex[33] = glTexImageTGAFile("art/Demon/die/ddeath6.tga", &spriteWidth[33], &spriteHeight[33]);
	spriteTex[34] = glTexImageTGAFile("art/Demon/die/ddeath7.tga", &spriteWidth[34], &spriteHeight[34]);
	spriteTex[35] = glTexImageTGAFile("art/Demon/die/ddeath8.tga", &spriteWidth[35], &spriteHeight[35]);
	spriteTex[36] = glTexImageTGAFile("art/Demon/die/ddeath9.tga", &spriteWidth[36], &spriteHeight[36]);
	spriteTex[37] = glTexImageTGAFile("art/Demon/die/ddeath10.tga", &spriteWidth[37], &spriteHeight[37]);
	spriteTex[38] = glTexImageTGAFile("art/Demon/die/ddeath11.tga", &spriteWidth[38], &spriteHeight[38]);
	spriteTex[39] = glTexImageTGAFile("art/Demon/die/ddeath12.tga", &spriteWidth[39], &spriteHeight[39]);
	spriteTex[40] = glTexImageTGAFile("art/Demon/die/ddeath13.tga", &spriteWidth[40], &spriteHeight[40]);
	spriteTex[41] = glTexImageTGAFile("art/Demon/die/ddeath14.tga", &spriteWidth[41], &spriteHeight[41]);
	spriteTex[42] = glTexImageTGAFile("art/Demon/die/ddeath15.tga", &spriteWidth[42], &spriteHeight[42]);
	
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
	
	// Setting up camera.
	// Camera dimen w=480 h=320
	// w=800=160+x+160 h=600=160+y+160
	camera.xPos = 0;
	camera.yPos = MAPHEIGHT * TILE_SIZE - WINDOW_HEIGHT;
	camera.width = WINDOW_WIDTH; // 2;
	camera.height = WINDOW_HEIGHT; // 2;

	demon.isWalking = false;
	demon.width = spriteWidth[0];
	demon.height = spriteHeight[0];
	demonAABB.x = demon.xPos;
	demonAABB.y = demon.yPos;
	demonAABB.w = demon.width;
	demonAABB.h = demon.height;
	demon.health = 100;
	demon.isGrounded = true;
	jumpButtonPressed = false;
	//updateProjPos();
	magicball.xPos = demon.xPos;
	magicball.yPos = demon.yPos;
	projectileAABB.x = magicball.xPos;
	projectileAABB.y = magicball.yPos;
	projectileAABB.w = 24;
	projectileAABB.h = 23;
	magicball.isVisible = false;
	magicball.speed = 1;
	magicball.distance = 0;



	int i, j;
	// Setting up main character and its animation.
	//demon idle animation
	AnimationDef demonIdleAnim;
	demonIdleAnim.totalFrames = 8;
	for (i = 0; i < 8; i++){
		demonIdleAnim.frame[i].texIndex = i;
		demonIdleAnim.frame[i].duration = .4;  
	}	

	AnimationDef demonIdleAnim2;
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
	AnimationDef projectileAnim;
	projectileAnim.totalFrames = 2;
	projectileAnim.frame[0].texIndex = 59;
	projectileAnim.frame[0].duration = 0.3;
	projectileAnim.frame[1].texIndex = 60;
	projectileAnim.frame[1].duration = 0.3;
	
	//demon move animation
	AnimationDef demonMoveAnim;
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
		demonDyingAnim.frame[i].duration = .5;
	}
	
	// setting up enemies and their animations
	AnimationDef catIdleAnim;
	catIdleAnim.totalFrames = 9;
	for (i = 0; i < 9; i++){
		catIdleAnim.frame[i].texIndex = i + 43;
		catIdleAnim.frame[i].duration = 0.5;
	}
	// cat dead
	AnimationDef catDeadAnim;
	catDeadAnim.totalFrames = 2;
	catDeadAnim.frame[0].texIndex = 54;
	catDeadAnim.frame[0].duration = .5;
	catDeadAnim.frame[1].texIndex = 53;
	catDeadAnim.frame[1].duration = 5;
	
	// cat hurt
	
	// cat attack
	
	// cat walking
	AnimationDef catWalkAnim;
	catWalkAnim.totalFrames = 3;
	catWalkAnim.frame[0].texIndex = 56;
	catWalkAnim.frame[0].duration = .4;
	catWalkAnim.frame[1].texIndex = 57;
	catWalkAnim.frame[1].duration = .4;
	catWalkAnim.frame[2].texIndex = 58;
	catWalkAnim.frame[2].duration = .4;
	
	//set up enemy position
	
	cat.xPos = 200;
	cat.yPos = 100;
	cat.health = 100;

	
	
	cat2.xPos = 1100;
	cat2.yPos = 500;
	cat2.health = 100;
	
	cat3.xPos = camera.xPos + 250;
	cat3.yPos = camera.yPos + 210;
	cat3.health = 100;
	cat3.hurt = false;
	cat3.isGrounded = true;
	
	cat4.xPos = camera.xPos + 100 ;
	cat4.yPos = camera.yPos + 420;
	cat4.health = 100;
	cat4.hurt = false;
	cat4.isGrounded = true;
	
	enemies[0] = cat;
	enemies[1] = cat2;
	enemies[2] = cat3;
	enemies[3] = cat4;
	
	//cat.xPos = MAPWIDTH - 100;
	//cat.yPos = MAPHEIGHT - 100;
	demon.xPos = camera.xPos;
	demon.yPos = camera.yPos + 400;
	demon.prevX = demon.xPos;
	demon.prevY = demon.yPos;
	//if (demon.isWalking){
			
		setAnimation(&demon.animData, &demonIdleAnim);
		setAnimation(&cat.animData, &catWalkAnim); //commentout
		setAnimation(&cat2.animData, &catWalkAnim);
		setAnimation(&cat3.animData, &catWalkAnim);
		setAnimation(&cat4.animData, &catIdleAnim);
		setAnimation(&magicball.animData, &projectileAnim);
		int k;
		
		for (k = 0; k < ENEMYSIZE; k++){
			setAnimation(&enemies[k].animData, &catIdleAnim);
		}
		
	//}
	//else{
	//	setAnimation(&demon.animData, &idleAnim);
	//}
	
	// Mapping.
	int mapL1[MAPHEIGHT][MAPWIDTH] = {
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
	int mapL2[MAPHEIGHT][MAPWIDTH] = {
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

	int grassMap[MAPHEIGHT][MAPWIDTH] = {
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
		0,0,0,13,14,15,0,0,0,0, 0,0,0,0,19,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0, 0,0,0,0,1,2,2,3,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,3,0, 0,0,0,0,4,5,5,6,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		2,2,2,3,0,0,1,2,2,2, 3,0,0,0,4,5,5,6,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
		5,5,5,20,2,2,21,5,5,5, 20,2,2,2,21,5,5,20,2,2, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
	};

	int grassMap2[MAPHEIGHT][MAPWIDTH] = {
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
	
	// setting up the map and collisions
	int y,x;
	for (x=0; x<MAPWIDTH; x++) {
		for (y=0; y<MAPHEIGHT; y++) {
			//tile[y][x].texIndex = mapL1[y][x];  
			//tile2[y][x].texIndex = mapL2[y][x];
			tile[y][x].texIndex = grassMap[y][x]; 
			backTile[y][x].texIndex = grassMap2[y][x];
			//setBox(&tile[y][x], x, y);
			//0 no collision, 1 all sides
			if (tile[y][x].texIndex == 2 || tile[y][x].texIndex == 3 || tile[y][x].texIndex == 1 
			|| tile[y][x].texIndex == 4 || tile[y][x].texIndex == 13 || tile[y][x].texIndex == 14
			|| tile[y][x].texIndex == 15 || tile[y][x].texIndex == 6 || tile[y][x].texIndex == 19  ){
				tile[y][x].collision = 1;	//all collision
			}
			else
			{
				tile[y][x].collision = 0;
			}
			
			/*
			if (tile[y][x].texIndex == 15 || tile[y][x].texIndex == 14 ||
				tile[y][x].texIndex == 13 || tile[y][x].texIndex == 19 ){
				tile[y][x].height = TILE_SIZE - 20;
			}

			else { */
				tile[y][x].height = TILE_SIZE;
			//}
		}
	}
	
	
	//mapL1[25] = 1;
	
	// Current and previous frame time.
	int prevFrameTime;
	int currFrameTime = SDL_GetTicks();
	float deltaTime;
	int count = 0;
	int physicsDeltaMs = 10;
	int lastPhysicsFrameMs;
	
	// The previous frame's keyboard state.
	unsigned char kbPrevState[SDL_NUM_SCANCODES] = {0};
	// The current frame's keyboard state.
	const unsigned char* kbState = NULL;
	kbState = SDL_GetKeyboardState(NULL);
	// The game loop.
	while (isGameLooping) {
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
            isGameLooping = 0;
        }
			demon.prevX = demon.xPos;
			demon.prevY = demon.yPos;
		//physics
		do{
			demon.box.x = demon.xPos;
			demon.box.y = demon.yPos;
			demon.box.h = 91;
			demon.box.w = 60;
			updateAABB();
			int b;

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
			*/
			//demon.isGrounded = true;
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
			if (magicball.isVisible){
			//for (i = 0; i < 5; i++){
				if (demon.xVelocity == 1){
				magicball.xPos += magicball.speed *deltaTime;
				}
				else{magicball.xPos -= magicball.speed *deltaTime;}
				//magicball.distance += magicball.speed*deltaTime;
				projectileAABB.x = magicball.xPos;
				projectileAABB.y = magicball.yPos;
				projectileAABB.w = 28;
				projectileAABB.h = 27;
				//int stop = magicball.xPosCam + 5;
				
				for (b = 0; b < ENEMYSIZE; b++){
					if (AABBIntersect(projectileAABB, enemies[b].box) && enemies[b].health > 0){
					enemies[b].health -=50;
					magicball.isVisible = false;
					updateProjPos();
					}
				}
				if (AABBIntersect(projectileAABB, catAABB) && cat.health > 0){
				//commentout
					cat.health -=50;
					magicball.isVisible = false;
					updateProjPos();
				}	
				if (AABBIntersect(projectileAABB, cat2AABB)&& cat2.health > 0){
					cat2.health -= 50;
					magicball.isVisible = false;
					updateProjPos();
				}
				if (AABBIntersect(projectileAABB, cat3AABB)&& cat3.health > 0){
					cat3.health -= 50;
					magicball.isVisible = false;
					updateProjPos();
				}
				if (AABBIntersect(projectileAABB, cat4AABB)&& cat4.health > 0){
					cat4.health -= 50;
					magicball.isVisible = false;
					updateProjPos();
				}

				if (!AABBIntersect(projectileAABB, cameraAABB)){
					magicball.isVisible = false;
					updateProjPos();
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
			//}
			}
		//}

		
			//collision with cats and demon
			/*
			int catTileX = floor(cat.xPos / TILE_SIZE);
			int catTileY = floor(cat.yPos/TILE_SIZE);
			AABB catTileBox;
			catTileBox.x = catTileX*TILE_SIZE;
			catTileBox.y = (catTileY+1)*TILE_SIZE;
			catTileBox.w = TILE_SIZE;
			catTileBox.h = TILE_SIZE;
			Tile catCurTile = tile[catTileY+1][catTileX];
			

			if (catCurTile.collision == 1){
				if (AABBIntersect(cat3AABB, catTileBox)){
					if (cat3.yVelocity > 0){
						cat3.yPos = (catTileY*TILE_SIZE);
						cat3.yVelocity = 0;
						cat3.isGrounded = true;
					}								
				}
				else{
					cat3.isGrounded = false;
				}
			}*/
			
			for(b = 0; b < ENEMYSIZE; b++){
				if (AABBIntersect(demon.box, enemies[b].box)){
					//going right					
					if (demon.xVelocity == 1){
						demon.xPos = demon.xPos -2;
						//cat4.xPos = cat4.xPos +1; //if want cat to bounce
					}
					//going left
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + 2;
						//cat4.xPos = cat4.xPos -1;						
					}
					//from above
					if (demon.yVelocity > 0 && demon.isGrounded == false){
						demon.yPos = demon.yPos - 5;
						//demon.xPos = demon.xPos +2;
						enemies[b].hurt = true;
						//demon.isGrounded = true;
						//demon.yVelocity = 0;
					}
					
				}
			}
				if (AABBIntersect(demon.box, catAABB)){//commentout
					if (demon.xVelocity == 1){
						demon.xPos = demon.xPos - 1;
					}
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + demon.width;	
					}
				}	
				if (AABBIntersect(demon.box, cat2AABB)){
					
					if (demon.xVelocity == 1){
						demon.xPos = demon.xPos - 1;
					}
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + demon.width;	
					}
				}
				if (AABBIntersect(demon.box, cat3AABB)){
					
					//going right					
					if (demon.xVelocity == 1){
						demon.xPos = demon.xPos -2;
						//cat3.xPos = cat3.xPos +1;
					}
					//going left
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + 2;
						//cat3.xPos = cat3.xPos -1;						
					}
					if (demon.yVelocity > 0 && demon.isGrounded == false){
						demon.yPos = demon.yPos - 5;
						cat3.hurt = true;
						//demon.isGrounded = true;
						//demon.yVelocity = 0;
					}
				}
				if (AABBIntersect(demon.box, cat4AABB)){
					//going right					
					if (demon.xVelocity == 1){
						demon.xPos = demon.xPos -2;
						//cat4.xPos = cat4.xPos +1;
					}
					//going left
					if (demon.xVelocity == -1){
						demon.xPos = demon.xPos + 2;
						//cat4.xPos = cat4.xPos -1;						
					}
					//from above
					if (demon.yVelocity > 0 && demon.isGrounded == false){
						demon.yPos = demon.yPos - 5;
						//demon.xPos = demon.xPos +2;
						cat4.hurt = true;
						//demon.isGrounded = true;
						//demon.yVelocity = 0;
					}
					
				}
			
				
			
			//physics collision detection
			int tileX = getTileX();
			int tileY = getTileY();

			/*	
			int tile_xStart = floor(camera.xPos/TILE_SIZE);
			int tile_yStart = floor(camera.yPos/TILE_SIZE);
			int tile_xEnd = floor((camera.xPos + WINDOW_WIDTH)/TILE_SIZE)+1;
			int tile_yEnd = floor((camera.yPos + WINDOW_HEIGHT)/TILE_SIZE)+1;
	*/
			
			/*
			//while (noCollision == true){
			int yt, xt;  
			do {
			for (xt = tile_xStart; xt < tile_xEnd; xt++){
				for (yt = tile_yStart; yt < tile_yEnd; yt++){
					if (tile[yt][xt].collision == 1){
					AABB tileBox;
					tileBox.x = xt;
					tileBox.y = yt;
					tileBox.w = TILE_SIZE;
					tileBox.h = TILE_SIZE;
					demon.box.x = xt;
					demon.box.y = yt;
					demon.box.h = 181;

					
					
						
						//if it intersects above
						//if (demon.yPos + demon.height < y && demon.yVelocity >0){
						if (AABBIntersect(demon.box, tileBox) == true){
						//if(yt + TILE_SIZE < demon.yPos + demon.height){
							//printf("collision ");
							noCollision = false;
							demon.yPos = (yt*TILE_SIZE) - demon.height + 6;
						//fix position
							//if(demon.yVelocity > 0){
								//demon.yPos = y*TILE_SIZE - demon.height;
							//demon.xPos = demon.prevY;
							//set grounded to true
							//demon.isGrounded = true;
							//}
						}	
				    }
				}
				
			}
			} //end while noCollision = true
			while (noCollision == true);
*/

			
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
				

			
			

			
			
			//physics collision resolution
			
			
			lastPhysicsFrameMs += physicsDeltaMs;
		}while (lastPhysicsFrameMs + physicsDeltaMs < currFrameTime);
		
		moveCharacterPos(kbState, deltaTime);
		if (demon.xVelocity == 1){
			setNewAnimation(&demon.animData, &demonIdleAnim);
		}
		if (demon.xVelocity == -1){
			setNewAnimation(&demon.animData, &demonIdleAnim2);		
		}
		
		if (cat3.hurt == true){
			setAnimation(&cat3.animData, &catDeadAnim);
			cat3.hurt = false;
			hasBeenHurt3 = true;
		}
		if (cat4.hurt == true){
			setAnimation(&cat4.animData, &catDeadAnim);
			cat4.hurt = false;
			hasBeenHurt4 = true;
		}

		//if (cat3.xPos < )
		setCameraAABB(cameraAABB);
		updateCharacterAABB();
		iterateAnimation(&demon.animData, deltaTime);
		iterateAnimation(&cat.animData, deltaTime);  
		iterateAnimation(&cat2.animData, deltaTime); 
		iterateAnimation(&cat3.animData, deltaTime); 
		iterateAnimation(&cat4.animData, deltaTime); 
		iterateAnimation(&magicball.animData, deltaTime); 
		
		//moveCharacterPos(kbState, deltaTime);
		updateAABB();
		
		//Shoot code
		if (kbState[SDL_SCANCODE_SPACE]){
			//magicball.isVisible = true;
			//magicball.xPos = demon.xPos;
			//magicball.yPos = demon.yPos;
			//updateProjPos();
			
		}
		
		//updateCameraPos(kbState, deltaTime);
		//updateCharacterPosInCam();
		//updateEnemyPos();
		updateCamera(deltaTime);
		//cameraFollow(deltaTime);
		// Game drawing GOES HERE.
		drawBackground();
		//updateProjDraw();
		
		// !!fix height!!
		drawCharacter(&demon.animData, (int)demon.xPos, (int)demon.yPos - 6, spriteWidth[0], spriteHeight[0]);
		if (AABBIntersect(cameraAABB, catAABB)){
			if (cat.health > 0 ){
				drawCharacter(&cat.animData, cat.xPos, cat.yPos, spriteWidth[43], spriteHeight[43]);
			}
		}
		if (AABBIntersect(cameraAABB, cat2AABB)){ 
			if (cat2.health > 0 ){
				drawCharacter(&cat2.animData, cat2.xPos, cat2.yPos, spriteWidth[43], spriteHeight[43]);
			}
		}
		if (AABBIntersect(cameraAABB, cat3AABB)){ 
			if (cat3.health > 0 ){
				drawCharacter(&cat3.animData, cat3.xPos, cat3.yPos, spriteWidth[43], spriteHeight[43]);
			}
		}
		if (AABBIntersect(cameraAABB, cat4AABB)){ 
			if (cat4.health > 0 ){
				drawCharacter(&cat4.animData, cat4.xPos, cat4.yPos, spriteWidth[43], spriteHeight[43]);
			}
		}
		drawProjectile();
		
		if (!demon.animData.isMoving){
			resetAnimation(&demon.animData);
		}
		if (!cat.animData.isMoving){
			resetAnimation(&cat.animData);
		}
		if (!cat2.animData.isMoving){
			resetAnimation(&cat2.animData);
		}
		if (!cat3.animData.isMoving){
			resetAnimation(&cat3.animData);
		}
		if (!cat4.animData.isMoving){
			resetAnimation(&cat4.animData);
		}
		if (!magicball.animData.isMoving){
			resetAnimation(&magicball.animData);
		}

		// Present the most recent frame.
        SDL_GL_SwapWindow(window);
    }

	// Cleaning up GOES HERE.
	
    SDL_Quit();

    return 0;
}

void drawProjectile(){
	if (magicball.isVisible == true){
		drawCharacter(&magicball.animData, magicball.xPos, magicball.yPos, spriteWidth[59], spriteHeight[59]);		
	}
}
void moveCharacterPos(const unsigned char* kbState, float deltaTime) {
// if (demon.collision == number for collide with ground)
	//move character up
	demon.prevX = demon.xPos;
	demon.prevY = demon.yPos;
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
	}
	if (kbState[SDL_SCANCODE_UP]) {
		//demon.yPos -= (1*deltaTime);
		demon.isWalking = true;
		jumpButtonPressed = true;
		characterBounds();
		updateCharacterAABB();
	}
	else{
		jumpButtonPressed = false;
	}
	if (kbState[SDL_SCANCODE_DOWN]) {
		if (demon.isGrounded == true){
			demon.yPos += (1*deltaTime);
		}
		demon.isWalking = true;
		characterBounds();
		updateCharacterAABB();

	}
}
int getTileX(){
	return floor(demon.xPos / TILE_SIZE);
}
int getTileY(){
	return floor(demon.yPos / TILE_SIZE);
	//return floor(demon.yPos / t);
}

int getOverlap(const AABB box1, const AABB box2){
	int ov;
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

void updateCameraPos(const unsigned char* kbState, int deltaTime) {
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
	if (camera.xPos  >= MAPWIDTH * TILE_SIZE - WINDOW_WIDTH) {
		camera.xPos = MAPWIDTH * TILE_SIZE - WINDOW_WIDTH;
	}
	if (camera.yPos <= 0) {
		camera.yPos = 0;
	}
	if (camera.yPos  >= MAPHEIGHT * TILE_SIZE - WINDOW_HEIGHT) {
		camera.yPos = MAPHEIGHT * TILE_SIZE - WINDOW_HEIGHT;
	}
}
void characterBounds() {
	if (demon.xPos <= 0 ) {
		demon.xPos = 0 ;
	}
	if (demon.xPos >= MAPWIDTH * TILE_SIZE - demon.width) {
		demon.xPos = MAPWIDTH * TILE_SIZE - demon.width;
	}
	if (demon.yPos <= 0) {
		demon.yPos = 0;
	}
	if (demon.yPos >= MAPHEIGHT * TILE_SIZE - demon.height) {
		demon.yPos = MAPHEIGHT * TILE_SIZE - demon.height;
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
void updateCamera(int deltaTime) {
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
/*
	if (demon.xPos + demon.width >= camera.xPos + camera.width){// + 300) {
		if (camera.xPos + camera.width < MAPWIDTH * TILE_SIZE) {
			camera.xPos += (1*deltaTime);
			
		}
	}
	else if (demon.xPos - demon.width <= camera.xPos) {
		if (camera.xPos > 0) {
			camera.xPos -= (1*deltaTime);
			
		}
	}
	if (demon.yPos - demon.height <= camera.yPos) {
		if (camera.yPos > 0) {
			camera.yPos -= (1*deltaTime);
		}
	}
	else if (demon.yPos - demon.height  >= camera.yPos + camera.height){// + 100) {
		if (camera.yPos + camera.height < MAPHEIGHT * TILE_SIZE) {
			camera.yPos += (1*deltaTime);
		}
	}
	cameraBounds();
*/
	}


void drawBackground() {
	
	int tile_xStart = floor(camera.xPos/TILE_SIZE);
	int tile_yStart = floor(camera.yPos/TILE_SIZE);
	int tile_xEnd = floor((camera.xPos + WINDOW_WIDTH)/TILE_SIZE)+1;
	int tile_yEnd = floor((camera.yPos + WINDOW_HEIGHT)/TILE_SIZE)+1;
	
	glDrawSprite(tileTex[18], 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	int y, x;  //y,x
	//for (i=0; i<MAPHEIGHT; i++) {
	for (x = tile_xStart; x < tile_xEnd; x++){
		//for (j=0; j<MAPWIDTH; j++) {
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
	for (i=0; i<MAPHEIGHT; i++) {
		for (j=0; j<MAPWIDTH; j++) {
			
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

/*
void drawEnemies(AnimationData* animData, Enemy e, const Tile[] tile){
	int i, j;
	for (i=0; i<MAPWIDTH; i++) {
		for (j=0; j<MAPHEIGHT; j++) {
			if (tile[i][j]){}
			drawCharacter();
		}
		}
	}
}
*/

void updateEnemyPos(){
	//int i;
	//for (i = 0; i < 5; i++){
	//enemies[i].xPosCam = enemies[i].xPos - camera.xPos;
	//enemies[i].yPosCam = enemies[i].yPos - camera.yPos;
	//}
	cat.xPos = cat.xPos - camera.xPos;
	cat.yPos = cat.yPos - camera.yPos;
	
	cat2.xPos = cat2.xPos - camera.xPos;
	cat2.yPos = cat2.yPos - camera.yPos;
	
	cat3.xPos = cat3.xPos - camera.xPos;
	cat3.yPos = cat3.yPos - camera.yPos;
	
	cat4.xPos = cat4.xPos - camera.xPos;
	cat4.yPos = cat4.yPos - camera.yPos;
}
void updateProjPos(){
	magicball.xPos = demon.xPos;
	magicball.yPos = demon.yPos + 30;
	
	//magicball.xPos = magicball.xPos - camera.xPos;
	//magicball.yPos = magicball.yPos - camera.yPos;
}
/*
void updateProjDraw(){
	magicball.xPos = magicball.xPos - camera.xPos;
	magicball.yPos = magicball.yPos - camera.yPos;
}*/
void updateAABB(){
	cameraAABB.x = camera.xPos;
	cameraAABB.y = camera.yPos;
	cameraAABB.w = camera.width;
	cameraAABB.h = camera.height;
	
	int i;
	
	for(i = 0; i < ENEMYSIZE; i++){
		if(enemies[i].hurt == false){
			enemies[i].box.x = enemies[i].xPos + 4;
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
	
	catAABB.x = cat.xPos; //commentout
	catAABB.y = cat.yPos;
	catAABB.w = spriteWidth[43];
	catAABB.h = spriteHeight[43];
	
	cat2AABB.x = cat2.xPos;
	cat2AABB.y = cat2.yPos;
	cat2AABB.w = spriteWidth[43];
	cat2AABB.h = spriteHeight[43];
	if (hasBeenHurt3 == false){
		cat3AABB.x = cat3.xPos + 4;//add offset for collision with left
		cat3AABB.y = cat3.yPos + 20; //add offset for collision with top
		cat3AABB.w = 49 - 25;
		cat3AABB.h = 49;
	}
	else{
		cat3AABB.x = cat3.xPos+4;
		cat3AABB.y = cat3.yPos + 27;
		cat3AABB.w = 49 - 25;
		cat3AABB.h = 49;
	}
	if(hasBeenHurt4 == false){
	cat4AABB.x = cat4.xPos+4;
	cat4AABB.y = cat4.yPos + 20;
	cat4AABB.w = (49 - 25);
	cat4AABB.h = 49;
	}
	else{
		cat4AABB.x = cat4.xPos+4;
		cat4AABB.y = cat4.yPos + 27;
		cat4AABB.w = 49 - 25;
		cat4AABB.h = 49;
	}
}
