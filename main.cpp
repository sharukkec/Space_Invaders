#include "lib/leetlib.h"
#include <math.h>

//int x[50];
//int y[50];
struct enemy {
	int x, y, size;
	bool exists;
	enemy(): x(0), y(0), size(0), exists(true) {}
};
enemy enemies[50];
int time=0;

struct bullet
{
	float BX,BY,BA;
	bool active;
	bullet(): BX(0), BY(0), BA(0), active(false) {}
};

bullet bullets[10];

void Game()
{
	bool gameover = false;
	bool win = false;
	unsigned int score = 0;
	
	void *Text[]=
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
	int UX=400, UY=550;
	void *Enemy = LoadSprite("gfx/Little Invader.png");
	void* EnemySniper = LoadSprite("gfx/Invader Sniper.png");
	void *U = LoadSprite("gfx/Big Invader.png");
	void *bull = LoadSprite("gfx/bullet.png");

	for(int n=0;n<50;++n)
	{
		enemies[n].x=(n%10)*60+120;
		enemies[n].y =(n/10)*60+70;
		enemies[n].size = 10 + (n % 17);
	}

	//Game cycle
	while (true) {
		
		++time;
		if (WantQuit()) return;
		if (IsKeyDown(VK_ESCAPE)) return;

		if (gameover) {
			StartTextBatch(50);
			DrawSomeText(400, 300, 50, 0xffffffff, true, "Game Over");
			DrawSomeText(400, 380, 30, 0xffffffff, true, "Your score: %d", score);
			DrawSomeText(400, 450, 15, 0xffffffff, true, "Best score: %d", 228);
			EndTextBatch();
			Flip();
			continue; // skip the rest of the loop
		}

		Hitbox playerHitbox = getHitbox(UX, UY, 50, 50); // player hitbox

		int enemyCounter = 0;	
		//enemy rendering
		for (int n = 0; n < 50; ++n)
		{
			if (!enemies[n].exists) continue; // skip if enemy does not exist
			int xo = 0, yo = 0;
			int n1 = time + n * n + n * n * n;
			int n2 = time + n + n * n + n * n * n * 3;
			if (((n1 >> 6) & 0x7) == 0x7)
				xo += (1 - cos((n1 & 0x7f) / 64.0f * 2.f * 3.141592)) * (20 + ((n * n) % 9));
			if (((n1 >> 6) & 0x7) == 0x7)
				yo += (sin((n1 & 0x7f) / 64.0f * 2.f * 3.141592)) * (20 + ((n * n) % 9));
			if (((n2 >> 8) & 0xf) == 0xf)
				yo += (1 - cos((n2 & 0xff) / 256.0f * 2.f * 3.141592)) * (150 + ((n * n) % 9));

			Hitbox enemyHitbox = getHitbox(enemies[n].x + xo, enemies[n].y + yo, enemies[n].size, enemies[n].size); // enemy hitbox
			if (CheckCollision(enemyHitbox, playerHitbox))
				gameover = true;
			for (int i = 0; i < 10; ++i)
			{
				if (!bullets[i].active) continue; // skip if bullet is not active
				Hitbox bulletHitbox = getHitbox(bullets[i].BX, bullets[i].BY, 10, 10); // bullet hitbox
				if (CheckCollision(enemyHitbox, bulletHitbox))
				{
					enemies[n].exists = false; // enemy is hit
					bullets[i].active = false; // bullet is used
					score += 100; // increase score
				}
			}

			if (!enemies[n].exists) continue;
			if (n < 10)
				DrawSprite(EnemySniper, enemies[n].x + xo, enemies[n].y + yo, enemies[n].size, enemies[n].size, 0, 0xffffffff);
			else
				DrawSprite(Enemy, enemies[n].x + xo, enemies[n].y + yo, enemies[n].size, enemies[n].size, 0, 0xffffffff);
		}

		//player rendering
		DrawSprite(U, UX += IsKeyDown(VK_LEFT) ? -7 : IsKeyDown(VK_RIGHT) ? 7 : 0, UY, 50, 50, 3.141592 + sin(time * 0.1) * 0.1, 0xffffffff);
		
		//Text rendering
		StartTextBatch(20);
		//DrawSomeText(100, 550, 20, 0xffffffff, false, "Lives: %d", 4);
		DrawSomeText(70, 550, 24, 0xffffffff, false, "Score: %d", score);
		EndTextBatch();
		

		// FIRE
		static int b = 0;
		static int count = 0;
		if (count) --count;
		if (!IsKeyDown(VK_SPACE)) count = 0;
		if (IsKeyDown(VK_SPACE) && count == 0) { bullets[b].BX = UX; bullets[b].BY = UY; bullets[b].active = true; b = (b + 1) % 10; count = 15; }

		for (int n = 0; n < 10; ++n)
		{
			if (!bullets[n].active) continue; // skip if bullet is not active
			DrawSprite(bull, bullets[n].BX, bullets[n].BY -= 4, 10, 10, bullets[n].BA += 0.1f, 0xffffffff);
			if (bullets[n].BY < 0) { bullets[n].active = false; continue; } // bullet is out of screen
		}

		for (int n = 0; n < strlen("space invaders"); ++n) if (n != 5)DrawSprite(Text[n], n * 40 + 150, 30, 20, 20, sin(time * 0.1) * n * 0.01);

		Flip();
	}
	
}

void OldGame()
{
	void *sprite = LoadSprite("sprite.png");
	float size=10;
	float angle=0;
	while (!WantQuit() && !IsKeyDown(VK_ESCAPE))
	{
		angle+=0.01f;
		float mx,my;
		GetMousePos(mx,my);
		DWORD col=0xffffffff; // white
		if (IsKeyDown(VK_LBUTTON)) col=0xffff0000; // solid red
		if (IsKeyDown(VK_RBUTTON)) col=0x8000ff00; // 50% transparent green
		if (IsKeyDown(VK_UP)) size++;
		if (IsKeyDown(VK_DOWN)) size--;
		
		DrawSprite(sprite,400,300,100,100, angle);
		DrawSprite(sprite, mx,my, size,size, 0, col);	
		Flip();
	}
}
