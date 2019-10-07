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
	bool collided;
	int prevX;
	int prevY;
	bool evil;
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
GLuint spriteTex[200];
int spriteWidth[200];
int spriteHeight[200];
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
double jumpVelocity = 0;//-.045;
double yGravity = 0;//.000008;
bool jumpButtonPressed;
bool noCollision = true;
bool hasBeenHurt3 = false;
bool hasBeenHurt4= false;
int tileX;
int tileY;	
Tile t1,t2,t3,t4,t5,t6,t7,t8;

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
void updateEnemyPrevPos();
void updateAABB();
void updateProjPos();
void drawProjectile();
void updateProjDraw();

int getTileX();
int getTileY();
void setTileBox();

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
    //tileTex[10] = glTexImageTGAFile("art/background/bottomleftcorner.tga", NULL, NULL);
	tileTex[10] = glTexImageTGAFile("art/Platformer/tiles/Tiles/10.tga", NULL, NULL);
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
	tileTex[18] = glTexImageTGAFile("art/Platformer/tiles/Tiles/18.tga", NULL, NULL);
	
	tileTex[19] = glTexImageTGAFile("art/Platformer/tiles/Tiles/19.tga", NULL, NULL);
	tileTex[20] = glTexImageTGAFile("art/Platformer/tiles/Tiles/20.tga", NULL, NULL);
	tileTex[21] = glTexImageTGAFile("art/Platformer/tiles/Tiles/21.tga", NULL, NULL);
	tileTex[22] = glTexImageTGAFile("art/Platformer/tiles/Tiles/22.tga", NULL, NULL);
	tileTex[23] = glTexImageTGAFile("art/Platformer/tiles/Tiles/23.tga", NULL, NULL);
	tileTex[24] = glTexImageTGAFile("art/Platformer/tiles/Object/Bush3.tga", NULL, NULL);
	tileTex[25] = glTexImageTGAFile("art/Platformer/tiles/BG/BG.tga", NULL, NULL);
	
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
	catIdleAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
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
	
	// cat walking
	AnimationDef catWalkAnim;
	catWalkAnim.totalFrames = 3;
	catWalkAnim.frame[0].texIndex = 56;
	catWalkAnim.frame[0].duration = .4;
	catWalkAnim.frame[1].texIndex = 57;
	catWalkAnim.frame[1].duration = .4;
	catWalkAnim.frame[2].texIndex = 58;
	catWalkAnim.frame[2].duration = .4;
	
	//cat walking reverse
	AnimationDef catWalkRAnim;
	catWalkRAnim.totalFrames = 3;
	catWalkRAnim.frame[0].texIndex = 79;
	catWalkRAnim.frame[0].duration = .4;
	catWalkRAnim.frame[1].texIndex = 80;
	catWalkRAnim.frame[1].duration = .4;
	catWalkRAnim.frame[2].texIndex = 81;
	catWalkRAnim.frame[2].duration = .4;
	
	//reverse idle
	AnimationDef catIdleRAnim;
	catIdleRAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		catIdleRAnim.frame[i].texIndex = i + 69;
		catIdleRAnim.frame[i].duration = 0.5;
	}

	//evil cat walking reverse
	AnimationDef evilcatWalkRAnim;
	evilcatWalkRAnim.totalFrames = 3;
	evilcatWalkRAnim.frame[0].texIndex = 92;
	evilcatWalkRAnim.frame[0].duration = .4;
	evilcatWalkRAnim.frame[1].texIndex = 93;
	evilcatWalkRAnim.frame[1].duration = .4;
	evilcatWalkRAnim.frame[2].texIndex = 94;
	evilcatWalkRAnim.frame[2].duration = .4;
	
	//evil cat reverse idle
	AnimationDef evilcatIdleRAnim;
	evilcatIdleRAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		evilcatIdleRAnim.frame[i].texIndex = i + 82;
		evilcatIdleRAnim.frame[i].duration = 0.5;
	}
	
	//evil cat walking 
	AnimationDef evilcatWalkAnim;
	evilcatWalkAnim.totalFrames = 3;
	evilcatWalkAnim.frame[0].texIndex = 95;
	evilcatWalkAnim.frame[0].duration = .4;
	evilcatWalkAnim.frame[1].texIndex = 96;
	evilcatWalkAnim.frame[1].duration = .4;
	evilcatWalkAnim.frame[2].texIndex = 97;
	evilcatWalkAnim.frame[2].duration = .4;
	
	//evil cat reverse idle
	AnimationDef evilcatIdleAnim;
	evilcatIdleAnim.totalFrames = 10;
	for (i = 0; i < 10; i++){
		evilcatIdleAnim.frame[i].texIndex = i + 98;
		evilcatIdleAnim.frame[i].duration = 0.5;
	}
	
	//evil cat love
	AnimationDef evilcatLoveAnim;
	evilcatLoveAnim.totalFrames = 5;
	for (i = 0; i < 5; i++){
		evilcatLoveAnim.frame[i].texIndex = i + 108;
		evilcatLoveAnim.frame[i].duration = 0.5;
	}
	
	//evil cat love reverse
	AnimationDef evilcatLoveRAnim;
	evilcatLoveRAnim.totalFrames = 5;
	for (i = 0; i < 5; i++){
		evilcatLoveRAnim.frame[i].texIndex = i + 113;
		evilcatLoveRAnim.frame[i].duration = 0.5;
	}
	
	//set up enemy position
	cat.xPos = 200;
	cat.yPos = 100;
	cat.prevX = cat.xPos;
	cat.prevY = cat.yPos;
	cat.health = 100;
	cat.evil = false;
	
	
	cat2.xPos = camera.xPos + 450;
	cat2.yPos = camera.yPos - 70;
	cat2.prevX = cat2.xPos;
	cat2.prevY = cat2.yPos;
	cat2.health = 100;
	cat2.evil = false;
	
	cat3.xPos = camera.xPos + 250;
	cat3.yPos = camera.yPos + 140;
	cat3.prevX = cat3.xPos;
	cat3.prevY = cat3.yPos;
	cat3.health = 100;
	cat3.hurt = false;
	cat3.isGrounded = true;
	cat3.evil = false;
	
	cat4.xPos = camera.xPos + 100;
	cat4.yPos = camera.yPos + 420;
	cat4.prevX = cat4.xPos;
	cat4.prevY = cat4.yPos;
	cat4.health = 100;
	cat4.hurt = false;
	cat4.isGrounded = true;
	cat4.evil = false;
	
	enemies[0] = cat;
	enemies[1] = cat2;
	enemies[2] = cat3;
	enemies[3] = cat4;
	
	//cat.xPos = MAPWIDTH - 100;
	//cat.yPos = MAPHEIGHT - 100;
	demon.xPos = camera.xPos;
	demon.yPos = camera.yPos + 390;
	demon.prevX = demon.xPos;
	demon.prevY = demon.yPos;
	//if (demon.isWalking){
			
		setAnimation(&demon.animData, &demonIdleAnim);
		/*
		setAnimation(&cat.animData, &catWalkAnim); //commentout
		setAnimation(&cat2.animData, &catWalkAnim);
		setAnimation(&cat3.animData, &catWalkAnim);
		setAnimation(&cat4.animData, &catIdleAnim);
		*/
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
			if (tile[y][x].texIndex == 1 || tile[y][x].texIndex == 2 || tile[y][x].texIndex == 3 
			|| tile[y][x].texIndex == 4 || tile[y][x].texIndex == 13 || tile[y][x].texIndex == 14
			|| tile[y][x].texIndex == 15 || tile[y][x].texIndex == 21 || tile[y][x].texIndex == 22  
			|| tile[y][x].texIndex == 23 || tile[y][x].texIndex == 24){
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
			//demon.prevX = demon.xPos;
			//demon.prevY = demon.yPos;
		//physics
		do{
			demon.box.x = demon.xPos;
			demon.box.y = demon.yPos;
			demon.box.h = 91;
			demon.box.w = 60;
			updateAABB();
			int b;



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
				
				//enemy and projectile checking
				for (b = 0; b < ENEMYSIZE; b++){
					if (AABBIntersect(projectileAABB, enemies[b].box) && enemies[b].health > 0){
					enemies[b].health -=50;
					magicball.isVisible = false;
					updateProjPos();
					}
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

		

			
			// player and enemy collision checking and resolution
			for(b = 0; b < ENEMYSIZE; b++){
				if (AABBIntersect(demon.box, enemies[b].box) && enemies[b].health > 0){
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
				
			//get grid positions
			tileX = getTileX();
			tileY = getTileY();	
			int gridPosX = tileX;
			int gridPosY = tileY;
			
			/*
			AABB tileBox; // check tile below
			tileBox.x = tileX*TILE_SIZE;
			tileBox.y = (tileY+1)*TILE_SIZE;
			tileBox.w = TILE_SIZE;
			tileBox.h = TILE_SIZE;
			
			AABB sideTileBoxR; //check tile to the right
			sideTileBoxR.x = (tileX+1)*TILE_SIZE;
			sideTileBoxR.y = tileY* TILE_SIZE;
			sideTileBoxR.w = TILE_SIZE;
			sideTileBoxR.h = TILE_SIZE;
			
			AABB sideTileBoxL; //check tile to the left
			sideTileBoxL.x = (tileX)*TILE_SIZE;
			sideTileBoxL.y = tileY* TILE_SIZE;
			sideTileBoxL.w = TILE_SIZE;
			sideTileBoxL.h = TILE_SIZE;
			
			demon.box.x = demon.xPos;
			demon.box.y = demon.yPos;
			demon.box.h = demon.height;
			
			Tile curTile = tile[tileY+1][tileX]; //tile below
			Tile curTileR = tile[tileY][tileX+1];//tile to the right
			Tile curTileL = tile[tileY][tileX]; //tile to the left
			*/
			
			//[t1][t2][t3]
			//[t4][x ][t5]
			//[t6][t7][t8]
			t1 = tile[tileY-1][tileX-1];
			t2 = tile[tileY-1][tileX];
			t3 = tile[tileY-1][tileX+1];
			t4 = tile[tileY][tileX-1];
			t5 = tile[tileY][tileX+1];
			t6 = tile[tileY+1][tileX-1];
			t7 = tile[tileY+1][tileX];
			t8 = tile[tileY+1][tileX+1];
			setTileBox();
			Tile surrounding[] = {t1,t2,t3,t4,t5,t6,t7,t8};
			demon.isGrounded = false;
			
			bool pdidCollide = false;

			//player collision detection
			for (b = 0; b < 8; b++){
				if (surrounding[b].collision == 1){
					if (AABBIntersect(demon.box, surrounding[b].box)){
						pdidCollide = true;
						//printf("collision ");
					}
				}
			}
			for(b = 0; b < ENEMYSIZE; b++){
				tileX = floor(enemies[b].xPos / TILE_SIZE);
				tileY = floor(enemies[b].yPos / TILE_SIZE);
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
				//demon.xPos = demon.prevX;
				//demon.yPos = demon.prevY;
				//pdidCollide = false;
				//demon.isGrounded = true;
				if(demon.xVelocity == 1){
					// if demon is not to the left or right of the tile to the right
					//if ((!demon.xPos + demon.width < surrounding[4].box.x) || (!demon.xPos > surrounding[4].box.x + surrounding[4].box.w)){
						demon.xPos = gridPosX*TILE_SIZE;
					//}
				}
				else{
				//	demon.xPos = gridPosX*TILE_SIZE;
				}
				if(demon.xVelocity == -1){
					//if ((!surrounding[3].box.x + surrounding[3].box.w < demon.xPos) || (surrounding[3].box.x > demon.xPos + demon.width)){
						//surrounding[3].box.x + surrounding[3].box.w - demon.width;
						demon.xPos = surrounding[3].box.x + surrounding[3].box.w - demon.width;
					//}
				}
				else{
				//	demon.xPos = gridPosX + TILE_SIZE - demon.width;
				}
				//if (AABBIntersect(demon.box, surrounding[6].box)){				
					if (demon.yVelocity > 0){
						demon.yPos = (gridPosY*TILE_SIZE);
						demon.yVelocity = 0;
						demon.isGrounded = true;	
					}													
				//}
				/*
				if (demon.direction == 4 || demon.direction == 2){
					demon.xPos = demon.prevX;
				}
				if (demon.direction == 1){
					//demon.yPos = demon.prevY + 20;
					//demon.yVelocity = .05;
					//demon.isGrounded = false;
				}
				if (demon.direction == 3){
					demon.yPos = demon.prevY;
				}
				if (demon.yVelocity > 0){
					demon.yPos = demon.prevY;
				//	demon.yVelocity = 0;
				//	demon.isGrounded = true;
				}
				*/
			}
			
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
				
			
			
			lastPhysicsFrameMs += physicsDeltaMs;
		}while (lastPhysicsFrameMs + physicsDeltaMs < currFrameTime);
		
		//player movement
		demon.prevX = demon.xPos;
		demon.prevY = demon.yPos;
		moveCharacterPos(kbState, deltaTime);
		if (demon.xVelocity == 1){
			setNewAnimation(&demon.animData, &demonIdleAnim);
		}
		if (demon.xVelocity == -1){
			setNewAnimation(&demon.animData, &demonIdleAnim2);		
		}
		updateEnemyPrevPos();
		
		int k;
		for (k = 0; k < ENEMYSIZE; k++){
			if (enemies[k].hurt == true){
				setAnimation(&cat3.animData, &catDeadAnim);
				enemies[k].hurt = false;
				//don't have a hasbeenhurt that might cause problems
				
			}
		}
		/*
		if (cat3.hurt == true){//commentout
			setAnimation(&cat3.animData, &catDeadAnim);
			cat3.hurt = false;
			hasBeenHurt3 = true;
		}
		if (cat4.hurt == true){
			setAnimation(&cat4.animData, &catDeadAnim);
			cat4.hurt = false;
			hasBeenHurt4 = true;
		}
		*/
		//if (cat3.xPos < )
		setCameraAABB(cameraAABB);
		updateCharacterAABB();
		for (k = 0; k < ENEMYSIZE; k++){
			iterateAnimation(&enemies[k].animData, deltaTime); 
		}
		iterateAnimation(&demon.animData, deltaTime);
		iterateAnimation(&magicball.animData, deltaTime); 
		
		//moveCharacterPos(kbState, deltaTime);
		updateAABB();
		
		//Shoot code
		if (kbState[SDL_SCANCODE_SPACE]){
			magicball.isVisible = true;
			magicball.xPos = demon.xPos;
			magicball.yPos = demon.yPos;
			updateProjPos();
			
		}
		
		//updateCameraPos(kbState, deltaTime);
		//updateCharacterPosInCam();
		updateCamera(deltaTime);
		//cameraFollow(deltaTime);
		
		// Game drawing GOES HERE.
		drawBackground();
		//updateProjDraw();
		
		// draw player
		drawCharacter(&demon.animData, (int)demon.xPos, (int)demon.yPos - 6, spriteWidth[0], spriteHeight[0]);

		//draw enemies
		for(k = 0; k < ENEMYSIZE; k++){
			if (AABBIntersect(cameraAABB, enemies[k].box)){
				if (enemies[k].health > 0 ){
					drawCharacter(&enemies[k].animData, enemies[k].xPos, enemies[k].yPos, spriteWidth[43], spriteHeight[43]);
				}
			}
		}
		//draw projectiles
		drawProjectile();
		
		//reset player animation
		if (!demon.animData.isMoving){
			resetAnimation(&demon.animData);
		}
		
		//reset the enemy animations
		for(k = 0; k < ENEMYSIZE; k++){
			if (!enemies[k].animData.isMoving){
				resetAnimation(&enemies[k].animData);
			}
		} //left off here
		
		//reset the projectiles animations
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
		demon.yPos -= (1*deltaTime);
		demon.isWalking = true;
		//jumpButtonPressed = true;
		characterBounds();
		updateCharacterAABB();
		demon.direction = 1;
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
		demon.yPos += (1*deltaTime);
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

	}


void drawBackground() {
	
	int tile_xStart = floor(camera.xPos/TILE_SIZE);
	int tile_yStart = floor(camera.yPos/TILE_SIZE);
	int tile_xEnd = floor((camera.xPos + WINDOW_WIDTH)/TILE_SIZE)+1;
	int tile_yEnd = floor((camera.yPos + WINDOW_HEIGHT)/TILE_SIZE)+1;
	
	glDrawSprite(tileTex[25], 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

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
void updateProjPos(){
	magicball.xPos = demon.xPos;
	magicball.yPos = demon.yPos + 30;
	
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

}
