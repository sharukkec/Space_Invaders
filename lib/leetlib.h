#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>


typedef unsigned long       DWORD;

#define PI ((float)3.1415926535)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 'system' - screen is always 800 x 600
// simply call StartFrame and Flip alternately to run the game; StartFrame returns false if the user is trying to quit

bool WantQuit(DWORD clearcolor=0);
void Flip(); // flips the screen, frame locked to 60 hz
void Game(); // you write this :)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// input
void GetMousePos(float &x, float &y); // 0,0 is top left; 800,600 is bottom right
bool IsKeyDown(int key); // use windows VK_ codes for special keys, eg VK_LEFT; use capital chars for letter keys eg 'A', '0'. use VK_LBUTTON and VK_RBUTTON for mouse buttons. 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// output
// 'sprite output' 
void *LoadSprite(const char *fname);
void DrawSprite(void *sprite, float xcentre, float ycentre, float xsize, float ysize, float rotate_angle_radians=0, DWORD tint_col_argb = 0xffffffff);

// 'Text output'

void StartTextBatch(int size = 0); // starts a text batch, you can call DrawSomeText() many times, then EndTextBatch() to draw it all at once
int DrawSomeText(int x, int y, int size, int col, bool centered, const char* pFormat, ...);
void EndTextBatch(); // ends a text batch, you can call DrawSomeText() many times, then EndTextBatch() to draw it all at once
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sound
int PlayMusic(const char *fname, float volume=1); // returns a handle which you only need if you want to change volume later with ChangeVolume
void StopMusic();

// sorry for this being 'snd' but otherwise it clashes with a windows #define (PlaySound) grr
void *LoadSnd(const char *fname, bool loop=false); // if you set loop the sample will go on looping forever, until you call StopSound
int PlaySnd(void *sound, float volume=1); // returns a handle which you only need if you are going to call StopSound or ChangeVolume()
void StopSnd(int handle);
void ChangeVolume(int handle, float newvolume=1);


struct Bullet
{
	float BX, BY, BA;
	bool active;
	Bullet() : BX(0), BY(0), BA(0), active(false) {}
};

//enemy functions
struct Enemy {
	int x, y, size, health;
	bool exists;
	Enemy() : x(0), y(0), size(0), exists(true), health(1){}
	virtual ~Enemy() = default;

	void respawn(int health) {
		exists = true;
		this->health = health; // respawn the enemy with the given health
	};
};

struct EnemySniper : public Enemy {
	Bullet bullet; // sniper bullet
	EnemySniper() = default;
	//void shootbullet()
};


std::vector<std::shared_ptr<Enemy>> spawnEnemies();

void respawnEnemies(std::vector<std::shared_ptr<Enemy>>& enemies, int health);

//-----------------------------------------------------------------------------
//Reset functions

inline  void resetPosition(int& x, int& y){
	x = 400; y = 550; // reset player x,y positions
}

inline void resetScore(unsigned int& score) {
	score = 0; // reset score
}

inline void resetDiff(int& diff) {
	diff = 1; // reset score
}

// reset bullets
void resetBullets(struct Bullet* bullets);

void resetGame(unsigned int& score, int& diff, int& UX, int& UY, std::vector<std::shared_ptr<Enemy>>& enemies, Bullet* bullets);

//-----------------------------------------------------------------------------
//Special screens Rendering
void renderGameOver(unsigned int score, unsigned int highScore);

void renderNextLevel(unsigned int score, int diff);

//-----------------------------------------------------------------------------
//Collision detection & hitboxes
struct Hitbox
{
	// A hitbox is an invisible rectangle, defined by its top left and bottom right corners
	Hitbox(float x1, float y1, float x2, float y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
	float x1, y1, x2, y2; // x1,y1 is top left, x2,y2 is bottom right
};

Hitbox getHitbox(float xcentre, float ycentre, float xsize, float ysize); // returns a rectangle with the hitbox of the sprite, for collision detection
bool CheckCollision(Hitbox & e, Hitbox & p);


//High score loading and saving
int loadHighScore(const std::string & filename = "High Score.txt");
void saveHighScore(int newScore, const std::string & filename = "High Score.txt");