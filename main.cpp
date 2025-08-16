#include "lib/gamelib.h"
#include <math.h>
#include <vector>
#include <memory>

int time=0;

// Desc: Main game loop function.
// This function initializes game state, handles game updates, rendering,
// input processing, and maintains the loop until the game ends

void Game()
{
	int startTimer = 0 ;

	// Text setup
	void *textSprites[]=
	{
		LoadSprite("gfx/slet.png"),
		LoadSprite("gfx/plet.png"),
		LoadSprite("gfx/alet.png"),
		LoadSprite("gfx/clet.png"),
		LoadSprite("gfx/elet.png"),
		0,
		LoadSprite("gfx/ilet.png"),
		LoadSprite("gfx/nlet.png"),
		LoadSprite("gfx/vlet.png"),
		LoadSprite("gfx/alet.png"),
		LoadSprite("gfx/dlet.png"),
		LoadSprite("gfx/elet.png"),
		LoadSprite("gfx/rlet.png"),
		LoadSprite("gfx/slet.png")
	};

	// SETUP
	const int MAX_BULLETS = 10; // maximum number of bullets on screen at once
	const int FIRE_COOLDOWN = 15; // cooldown for firing bullets
	const int RESTART_DELAY = 60; // delay before restarting (in frames) the game after game over or next level
	int UX = 400, UY = 550; // player x,y positions
	void* enemySprite = LoadSprite("gfx/Little Invader.png"); // enemy sprite
	void* uSprite = LoadSprite("gfx/Big Invader.png"); // player sprite
	void* bullSprite = LoadSprite("gfx/bullet.png"); // bullet sprite
	bool gameover = false; // game over flag
	bool nextLevel = true; // next level flag
	unsigned int score = 0; // current score
	unsigned int highScore = loadHighScore(); // load high score from file
	int diff = 1; // difficulty level changes the enemy health 
	std::vector<Bullet> bullets(10); // vector of bullets


	//sound setup
	void* shootSound = LoadSnd("sounds/shoot.wav", false);

	//enemy setup
	std::vector<Enemy> enemies = spawnEnemies();

	//Game cycle
	while (true) {
		
		++time;
		// Quit logic
		if (WantQuit()) {  saveHighScore(highScore); return; }
		if (IsKeyDown(VK_ESCAPE)) { saveHighScore(highScore); return; }

		// check for game over
		if (gameover) {
			StopMusic(); // stop music on game over
			if (score > highScore)
				highScore = score;
			if ( time - startTimer > RESTART_DELAY && IsKeyDown(VK_SPACE)) {
				resetGame(score, diff, UX, UY, enemies, bullets); // reset game state
				gameover = false; // reset gameover flag
				nextLevel = true; // reset next level flag
				time = 0; // reset time
				startTimer = 60; // reset start timer
			} 
			renderGameOver(score, highScore);
			continue; // skip the rest of the loop
		}

		// check for next level
		if (nextLevel) {
			StopMusic(); // stop music on next level, let the player have a moment to breathe
			if ((startTimer == 0 || (time - startTimer > RESTART_DELAY)) && IsKeyDown(VK_SPACE)) {
				nextLevel = false; // wait for space to continue
				time = 0; // reset time
				startTimer = 0; // reset start timer
				PlayMusic("sounds/main theme.wav", 0.1f); // play main theme music
			}
			renderNextLevel(score, diff);
			continue; // skip the rest of the loop
		}

		// player hitbox
		const Hitbox playerHitbox = getHitbox(UX, UY, 50, 50);

		
		// enemy rendering and collision detection
		if (!renderEnemies(enemies, bullets, enemySprite, playerHitbox, time, startTimer, gameover, score)) {
			startTimer = time;
			diff++;
			respawnEnemies(enemies, diff); // respawn enemies with increased health
			resetBullets(bullets); // reset bullets
			resetPosition(UX, UY); // reset player position
			nextLevel = true; // set next level flag
		}

		//player rendering
		DrawSprite(uSprite, UX += IsKeyDown(VK_LEFT) ? -7 : IsKeyDown(VK_RIGHT) ? 7 : 0, UY, 50, 50,  3.141592f + sinf(time * 0.1f) * 0.1f, 0xffffffff);
		
		//Text rendering
		renderScore(score);
		
		// FIRE logic
		static int b = 0;
		static int count = 0;
		if (count) --count;
		if (!IsKeyDown(VK_SPACE)) count = 0;
		if (IsKeyDown(VK_SPACE) && count == 0) { 
			PlaySnd(shootSound, 0.1f); // play shoot sound
			bullets[b].BX = UX;
			bullets[b].BY = UY;
			bullets[b].active = true;
			b = (b + 1) % MAX_BULLETS;
			count = FIRE_COOLDOWN;
		}

		// BULLET rendering and movement
		for (int n = 0; n < MAX_BULLETS; ++n)
		{
			if (!bullets[n].active) continue; // skip if bullet is not active
			DrawSprite(bullSprite, bullets[n].BX, bullets[n].BY -= 4, 10, 10, bullets[n].BA += 0.1f, 0xffffffff);
			if (bullets[n].BY < 0) { bullets[n].active = false; continue; } // bullet is out of screen
		}

		// Draw text sprites for "space invaders"
		for (int n = 0; n < strlen("space invaders"); ++n) if (n != 5)DrawSprite(textSprites[n], n * 40 + 150, 30, 20, 20, sin(time * 0.1) * n * 0.01);

		// Flip the screen to render everything
		Flip();
	}
}

