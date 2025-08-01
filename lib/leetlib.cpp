#pragma warning(disable:4995)
#include "leetlib.h"
#include <d3d9.h>
#include <d3dx9.h>		
#include <strsafe.h>
#include <math.h>
#include <map>
#include <direct.h>
#include <malloc.h>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include "fmod/api/inc/fmod.h"
#pragma comment(lib,"lib/fmod/api/lib/fmodvc.lib")
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")


#pragma warning(disable:4244)

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
LPDIRECT3D9             g_pD3D       = NULL; // Used to create the D3DDevice
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; // Our rendering device
LPDIRECT3DVERTEXBUFFER9 g_pVB        = NULL; // Buffer to hold tea2
bool fullscreen;

// A structure for our custom vertex type
struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
	float u,v;
};



// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Text rendering

std::map<int, LPD3DXFONT> fonts;
int lasttextwidth,lasttextheight;
int intextbatch=0;
LPD3DXSPRITE fontsprite;
#define MA_RELEASE(x) {int c=0;if (x) c=(x)->Release();x=NULL;}
void ReleaseFonts();
typedef unsigned int u32;

void StartTextBatch(int size);
//int DrawText(int x, int y, int size, u32 col, bool cenetered, const char *pFormat, ...);


void EndTextBatch();

// Find a font of the given size, or create it if it doesn't exist
LPD3DXFONT FindFont(int size) {
	auto fit = fonts.find(size);
	if (fit == fonts.end()) {
		LPD3DXFONT m_pFont = NULL;
		if (FAILED(D3DXCreateFont(g_pd3dDevice, size, 0, FW_NORMAL, 0, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			TEXT("Lucida Console"), &m_pFont)))
			
			return NULL;
		fit = fonts.insert({ size, m_pFont }).first;
	}
	return (fit != fonts.end()) ? fit->second : NULL;
}

// Release all fonts and sprites
void ReleaseFonts()
{
	if (intextbatch) EndTextBatch();
	for (std::map<int, LPD3DXFONT>::iterator it=fonts.begin();it!=fonts.end();++it)
	{
		MA_RELEASE(it->second);
	}
	fonts.clear();
	MA_RELEASE(fontsprite);
	intextbatch=0;
}

// Start a text batch, allowing multiple DrawSomeText calls to be made before rendering
void StartTextBatch(int size)
{
	if (intextbatch) EndTextBatch();
	intextbatch=1;
	if (size) FindFont(size);
	if (!fontsprite) 
	{
		if (FAILED(D3DXCreateSprite( g_pd3dDevice, &fontsprite)))
			return;
	}
	if (fontsprite)
	{
		fontsprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );
	}
	//for (std::map<int, LPD3DXFONT>::iterator fit=fonts.begin();fit!=fonts.end();++fit) fit->second->Begin();


}

// End the text batch, drawing all the text that was queued up
void EndTextBatch()
{
	if (intextbatch)
	{
		intextbatch=0;
		if (fontsprite)
		{			
			fontsprite->End();
		}
		//for (std::map<int, LPD3DXFONT>::iterator fit=fonts.begin();fit!=fonts.end();++fit) fit->second->End();		
	}
}


// Draw some text at the specified position, with the specified size and color.
int DrawSomeText(int x, int y, int size, int col, bool centered, const char *pFormat, ...)
{
	char debugText[8192];
	va_list	parameter;
	va_start(parameter,pFormat);
	vsnprintf(debugText, sizeof(debugText), pFormat, parameter);

	LPD3DXFONT m_pFont=FindFont(size);
	if (!m_pFont) return 0;
	RECT r={x,y,x+100,y+100};	
	
	m_pFont->DrawText(intextbatch?fontsprite:NULL,debugText,-1,&r,DT_CALCRECT,0);
	int wid=r.right-r.left;
	if (centered) r.left-=wid/2;
	if (centered) r.right-=wid/2;
	m_pFont->DrawText(intextbatch?fontsprite:NULL,debugText,-1,&r,DT_TOP|DT_LEFT,col);
	lasttextheight=r.bottom-r.top;
	lasttextwidth=r.right-r.left;
	return lasttextheight;


}

// End of Text rendering

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//-----------------------------------------------------------------------------
// Name: InitD3D()
// Desc: Initializes Direct3D
//-----------------------------------------------------------------------------
HRESULT InitD3D( HWND hWnd )
{
    // Create the D3D object.
    if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    d3dpp.Windowed = !fullscreen;
	d3dpp.SwapEffect = fullscreen? D3DSWAPEFFECT_FLIP : D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE ;
	d3dpp.BackBufferFormat = fullscreen ? D3DFMT_A8R8G8B8 : D3DFMT_UNKNOWN;
	d3dpp.BackBufferWidth=800;
	d3dpp.BackBufferHeight=600;
	d3dpp.FullScreen_RefreshRateInHz = fullscreen? 60 : 0;

    // Create the D3DDevice
    if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice ) ) )
    {
        return E_FAIL;
    }

    // Device state would normally be set here

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitVB()
// Desc: Creates a vertex buffer and fills it with our tea2. The vertex
//       buffer is basically just a chuck of memory that holds tea2. After
//       creating it, we must Lock()/Unlock() it to fill it. For indices, D3D
//       also uses index buffers. The special thing about vertex and index
//       buffers is that they can be created in device memory, allowing some
//       cards to process them in hardware, resulting in a dramatic
//       performance gain.
//-----------------------------------------------------------------------------
HRESULT InitVB()
{
  
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),
                                                  D3DUSAGE_DYNAMIC, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    {
        return E_FAIL;
    }

   

	//D3DXCreateTextureFromFile(g_pd3dDevice,"bg.jpg",&g_bgtex);
	//D3DXCreateTextureFromFile(g_pd3dDevice,"arrow.png",&g_arrow);


    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
VOID Cleanup()
{
    if( g_pVB != NULL )        
        g_pVB->Release();

    if( g_pd3dDevice != NULL ) 
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )       
        g_pD3D->Release();

	ReleaseFonts();
	FSOUND_Close();
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
bool g_keydown[256];
int g_keyhit[256];
int g_mb;
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		g_mb|=1;
		g_keydown[VK_LBUTTON]=true;
		g_keyhit[VK_LBUTTON]++;
		break;
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		g_keydown[VK_RBUTTON]=true;
		g_keyhit[VK_RBUTTON]++;
		g_mb|=2;
		break;
	case WM_MBUTTONDOWN:
		SetCapture(hWnd);
		g_mb|=4;
		g_keydown[VK_MBUTTON]=true;
		g_keyhit[VK_MBUTTON]++;
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		g_mb&=~1;
		g_keydown[VK_LBUTTON]=false;
		break;
	case WM_RBUTTONUP:
		ReleaseCapture();
		g_mb&=~2;
		g_keydown[VK_RBUTTON]=false;
		break;
	case WM_MBUTTONUP:
		ReleaseCapture();
		g_mb&=~4;
		g_keydown[VK_MBUTTON]=false;
		break;
		
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		g_keydown[wParam&255]=true;
		g_keyhit[wParam&255]++;
		return 0;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		g_keydown[wParam&127]=false;
		break;

    case WM_DESTROY:
        Cleanup();
        PostQuitMessage( 0 );
        return 0;
	case WM_ACTIVATEAPP:
		if (!wParam)
		{
			memset(g_keydown,0,sizeof(g_keydown));
		}
		break;

	case WM_ACTIVATE:
		if( WA_INACTIVE != wParam )
		{
			// Make sure the device is acquired, if we are gaining focus.
			
		}
		break;
    }


    return DefWindowProc( hWnd, msg, wParam, lParam );
}

LARGE_INTEGER starttime;
LARGE_INTEGER freq;
extern HWND hWnd;
HWND hWnd;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR cmd, INT )
{	
	// FULLSCREEN?
	int id = MessageBox(NULL,"fullscreen?","answer me!",MB_YESNOCANCEL);
	if (id==IDCANCEL) return 0;
	fullscreen=(id==IDYES);

    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      "Game", NULL };
    RegisterClassEx( &wc );

	RECT r={0,0,WINDOW_WIDTH,WINDOW_HEIGHT };
	int style = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	style|=WS_VISIBLE;

	AdjustWindowRect(&r,style,false);

    // Create the application's window
     hWnd = CreateWindow( "Game", "Space invaders",
                              style, 0,0,r.right-r.left,r.bottom-r.top,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL );

	FSOUND_Init(44100, 42, 0);
	
	QueryPerformanceCounter(&starttime);
	QueryPerformanceFrequency(&freq);
    // Initialize Direct3D
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Create the vertex buffer
        if( SUCCEEDED( InitVB() ) )
        {
			//SetWindowPos(hWnd,NULL,0,0,1024,768,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE|SWP_ASYNCWINDOWPOS);
			SetCursor(LoadCursor(NULL,IDC_ARROW));

            // Show the window
            ShowWindow( hWnd, SW_SHOWDEFAULT );
            UpdateWindow( hWnd );

			//InitDirectInput( hWnd );			
			
			Game();
			//DestroyWindow(hWnd);
        }
    }
	
	// Cleanup everything used 
	Cleanup();
	// Release all fonts and sprites
	// Cleanup and close the application
    UnregisterClass( "Game", wc.hInstance );
    return 0;
}

//------------------------------------------------------------------------------
///Handles Windows messages, checks for quit events, and prepares the Direct3D device
/// for a new frame by setting rendering states and clearing the screen.
bool WantQuit(DWORD clearcol)
{
	// Enter the message loop
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	while ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		if (msg.message==WM_QUIT) return true;
	}	

	// Clear the backbuffer to a solid color
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, clearcol, 1.0f, 0 );

	// Set the render states
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,true);
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE,false);

	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);

	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);

	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,false);
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	D3DVIEWPORT9 vp={ 0,0, 800, 600, 0, 1};

	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

	// Begin the scene
	g_pd3dDevice->BeginScene() ;

	return false;
}

// Get the time in milliseconds since the start of the program
int	GetTimeInMS() // ...since start of program
{
	LARGE_INTEGER arse;
	QueryPerformanceCounter(&arse);
	return ((arse.QuadPart-starttime.QuadPart)*1000) / freq.QuadPart;
}

// Flip the backbuffer to the front, presenting the rendered scene to the display
void Flip()
{
	
	static int lastflip=0;
	if (lastflip==0) lastflip = GetTimeInMS();
	g_pd3dDevice->EndScene();

	// Present the backbuffer contents to the display
	
	//while (GetTimeInMS()<lastflip+1000/60) ;// Sleep(0); // clamp to max of 60hz
	lastflip=GetTimeInMS();

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	Sleep(0);
	memset(g_keyhit,0,sizeof(g_keyhit));
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	
}

// Get the current mouse position relative to the window
void GetMousePos(float &x, float &y) // 0,0 is top left; 800,600 is bottom right
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hWnd, &p);
	x=p.x;
	y=p.y;	
}

// Checks whether a specific key is currently being held down.
bool IsKeyDown(int key) // use windows VK_ codes for special keys, eg VK_LEFT; use capital chars for letter keys eg 'A', '0'
{
	return g_keydown[key&255];
}

// 'sprite output' 
// Loads a sprite from a file and returns a pointer to the texture
void * LoadSprite(const char *fname)
{
	IDirect3DTexture9 *tex = NULL;
	D3DXCreateTextureFromFile(g_pd3dDevice,fname,&tex);
	return tex;
}

// sets the current texture for rendering
void SetCurrentTexture(void *tex )
{
	IDirect3DTexture9 *t = (IDirect3DTexture9 *)tex;
	g_pd3dDevice->SetTexture(0,t);
}

// 'sprite output' - draw a sprite at xcentre,ycentre, with size xsize,ysize, rotated by angle, with color col
void DrawSprite(void *sprite, int xcentre, int ycentre, int xsize, int ysize, float angle, DWORD col )
{
	SetCurrentTexture(sprite);
	float c=cosf(angle);
	float s=sinf(angle);
#define ROTATE(xx,yy) xcentre+(xx)*c+(yy)*s,ycentre+(yy)*c-(xx)*s 
	CUSTOMVERTEX tea2[] =
	{

		///{ xcentre+xsize*c+ysize*s,ycentre+ysize*c-xsize*s , 0.5f, 1.0f, col, 0,0, }, // x, y, z, rhw, color
		
		{ ROTATE(-xsize,-ysize), 0.5f, 1.0f, col, 0,0, }, // x, y, z, rhw, color
		{ ROTATE( xsize,-ysize), 0.5f, 1.0f, col, 1,0, },
		{ ROTATE(-xsize, ysize), 0.5f, 1.0f, col, 0,1, },
		{ ROTATE( xsize, ysize), 0.5f, 1.0f, col, 1,1, },
	};
	g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, tea2, sizeof(CUSTOMVERTEX));
}

//Function to get a hitbox for future collision detection
Hitbox getHitbox(int xcentre, int ycentre, int xsize, int ysize) {
	return Hitbox(xcentre - xsize, ycentre - ysize, xcentre + xsize, ycentre + ysize);
}

// Check if two hitboxes collide
bool CheckCollision(const Hitbox& e, const Hitbox& p) {
	return ((p.x1 > e.x1 && p.x1 < e.x2 || p.x2 > e.x1 && p.x2 < e.x2) && (p.y1 > e.y1 && p.y1 < e.y2 || p.y2 > e.y1 && p.y2 < e.y2));
	//return true;
}

//enemy spawn
std::vector<Enemy> spawnEnemies() {
	std::vector<Enemy> enemies(50);
	enemies.reserve(50);
	for (int n = 0; n < 50; ++n)
	{
		enemies[n].x = (n % 10) * 60 + 120;
		enemies[n].y = (n / 10) * 60 + 70;
		enemies[n].size = 10 + (n % 17);
		
	}
	return enemies;
}

// Respawn enemies with a given health value
void respawnEnemies(std::vector<Enemy>& enemies, int health) {
	for (int n = 0; n < 50; ++n)
	{
		enemies[n].respawn(health);
	}
}

//------------------------------------------------------------------------------
//High score functions

// Load high score from a file
int loadHighScore(const std::string& filename) {
	std::ifstream file(filename);
	int highScore = 0;
	if (file >> highScore) {
		return highScore;
	}
	return 0; // default if file missing or unreadable
}

// Save a new high score to a file
void saveHighScore(int newScore, const std::string& filename) {
	std::ofstream file(filename);
	if (file) {
		file << newScore;
	}
}

//------------------------------------------------------------------------------
// Reset functions

// Reset bullets state
void resetBullets(std::vector<Bullet>& bullets) {
	for (int i = 0; i < 10; ++i) {
		bullets[i].active = false; // reset bullets
	}
}

// Reset Game state
void resetGame(unsigned int& score, int& diff, int& UX, int& UY, std::vector<Enemy>& enemies, std::vector<Bullet>& bullets) {
	resetPosition(UX, UY); // reset player position
	resetScore(score); // reset score
	resetDiff(diff); // reset difficulty
	respawnEnemies(enemies, diff); // respawn enemies with initial health
	resetBullets(bullets); // reset bullets
}

//------------------------------------------------------------------------------
//Render special screens

// Render the Game Over screen with score and high score
void renderGameOver(unsigned int score, unsigned int highScore){
	StartTextBatch(50);
	DrawSomeText(400, 300, 50, 0xffffffff, true, "Game Over.");
	DrawSomeText(400, 380, 30, 0xffffffff, true, "Your score: %d", score);
	DrawSomeText(400, 450, 15, 0xffffffff, true, "Best score: %d", highScore);
	DrawSomeText(400, 480, 15, 0xffffffff, true, "Press Space to restart");
	DrawSomeText(400, 500, 15, 0xffffffff, true, "Press  Esc to quit");
	EndTextBatch();
	Flip();
}

// Render the Next Level screen with score
void renderNextLevel(unsigned int score, int diff){
	StartTextBatch(50);
	DrawSomeText(400, 300, 50, 0xffffffff, true, "Level %d", diff);
	DrawSomeText(400, 380, 20, 0xffffffff, true, "Press space to Continue.");
	EndTextBatch();
	Flip();
}

// Render the current score on the screen
void renderScore(unsigned int score) {
	StartTextBatch(20);
	DrawSomeText(70, 550, 24, 0xffffffff, false, "Score: %d", score);
	EndTextBatch();
}

//------------------------------------------------------------------------------
// Render enemies and handle collisions with player and bullets
bool renderEnemies(std::vector<Enemy> & enemies, std::vector<Bullet> & bullets, void* enemySprite, const Hitbox & playerHitbox,int time, int & startTimer, bool & gameover, unsigned int & score) {
	int enemyCounter = 0;
	//enemy rendering
	for (int n = 0; n < 50; ++n)
	{
		Enemy& enemy = enemies[n];
		if (!enemy.exists) continue; // skip if enemy does not exist
		enemyCounter++;
		int xo = 0, yo = 0;
		int n1 = time + n * n + n * n * n;
		int n2 = time + n + n * n + n * n * n * 3;
		if (((n1 >> 6) & 0x7) == 0x7)
			xo += (1 - cos((n1 & 0x7f) / 64.0f * 2.f * PI)) * (20 + ((n * n) % 9));
		if (((n1 >> 6) & 0x7) == 0x7)
			yo += (sin((n1 & 0x7f) / 64.0f * 2.f * PI)) * (20 + ((n * n) % 9));
		if (((n2 >> 8) & 0xf) == 0xf)
			yo += (1 - cos((n2 & 0xff) / 256.0f * 2.f * PI)) * (150 + ((n * n) % 9));

		int enemyX = enemy.x + xo; // calculate enemy x position
		int enemyY = enemy.y + yo; // calculate enemy y position
		Hitbox enemyHitbox = getHitbox(enemyX, enemyY, enemy.size, enemy.size); // enemy hitbox

		// check for collision with player
		if (CheckCollision(enemyHitbox, playerHitbox)) {
			gameover = true;
			startTimer = time;
		}

		// check for collision with bullets
		for (Bullet& bullet : bullets) {
			if (!bullet.active) continue;

			Hitbox bulletHitbox = getHitbox(bullet.BX, bullet.BY, 10, 10);
			if (CheckCollision(enemyHitbox, bulletHitbox)) {
				enemy.health--;
				bullet.active = false;

				if (enemy.health <= 0) {
					enemy.exists = false;
					score += 100;
					break;
				}
			}
		}

		if (enemy.exists) 
			DrawSprite(enemySprite, enemyX, enemyY, enemy.size, enemy.size, 0, 0xffffffff);
	}
	return enemyCounter > 0; // return true if there are enemies left
}


//------------------------------------------------------------------------------
//Music and sound functions
FSOUND_STREAM *music;

// Play music from a file with a given volume (0.0 to 1.0)
int PlayMusic(const char *fname, float volume)
{
	if (music) StopMusic();
	music = FSOUND_Stream_Open(fname,FSOUND_LOOP_NORMAL,0,0);
	int chan = FSOUND_Stream_Play(FSOUND_FREE,music);
	if (volume<=0) volume=0;
	if (volume>1) volume=1;
	FSOUND_SetVolume(chan, (int)(volume*255));
	return chan;
}

// Stop the currently playing music
void StopMusic()
{
	if (music)
	{
		FSOUND_Stream_Close(music);
	}
	music=NULL;
}

void *LoadSnd(const char *fname, bool looped)
{
	int flags=0;
	if (looped) flags|=FSOUND_LOOP_NORMAL;
	return FSOUND_Sample_Load(FSOUND_FREE,fname,flags,0,0);
}

// Play a sound sample with a given volume (0.0 to 1.0)
int PlaySnd(void *sound, float volume)
{
	if (!sound) return -1;
	if (volume<=0) volume=0;
	if (volume>1) volume=1;
	int chan = FSOUND_PlaySound(FSOUND_FREE,(FSOUND_SAMPLE*)sound);
	FSOUND_SetVolume(chan, (int)(volume*255));
	return chan;
}

// Stop a sound sample by its handle
void StopSnd(int handle)
{
	if (handle<=0) return ;
	FSOUND_StopSound(handle);
}

// Change the volume of a sound sample by its handle (0.0 to 1.0)
void ChangeVolume(int handle, float volume)
{
	if (handle<=0) return ;
	if (volume<=0) volume=0;
	if (volume>1) volume=1;
	FSOUND_SetVolume(handle, (int)(volume*255));
}
