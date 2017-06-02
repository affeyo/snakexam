/*
 * @file: GameManager.cpp
 * @author: Alf Henrik Langaas, Ole Petter Smith
 * @version: 2.1.2 <02.06.2017>
 *
 * @description: This class controls a game of Snake.
 */

#include "SDL/SDLBmp.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Timer.h"
#include <string>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

/* Initializes SDL, creates the game window and fires off the timer. It also initialize the audiofiles and the font. */
GameManager::GameManager()
{
	TTF_Init();
	Sans = TTF_OpenFont("Assets/font/Sans.ttf", 40);
	SDLManager::Instance().init();

	m_window = SDLManager::Instance().createWindow("Snake", 640,480);
	
	rend = SDLManager::Instance().getRenderer(*(SDLManager::Instance().getMainWindow()));
	Timer::Instance().init();
	
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("Error: %s", Mix_GetError());
	}
	eat = Mix_LoadWAV("Assets/sfx/olu.wav");
	death = Mix_LoadWAV("Assets/sfx/faen.wav");

}


/* Main function */
void GameManager::play()
{
	// Initializes variables
	bool gameOver = false; // Changed from notGameOver to gameOver as it gives more sense
	bool eaten = false; 
	float last_movement = 0.0f; 
	float movement_timer = 0.10f; 
	int score = 0; 
	float lastposx = 0.0f;
	float lastposy = 0.0f;
	float appleCounter = 0.0f;

	// Struct to contain the tail positions
	struct bodyparts
	{
		float x = 0.0f;
		float y = 0.0f;
	};
	// Contains the tail parts. 768 is the maximum amount of tail pieces possible.
	bodyparts parts[768];

	// enum with the directions.
	enum directions
	{
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT
	};
	int dir = DIR_RIGHT;

	// Struct to keep control of the player position
	struct playerCharacter {
		float x = 0.0f;
		float y = 0.0f;
	};

	playerCharacter player;

	// Load bitmaps
	SDLBmp backround("Assets/gfx/sdl2.bmp");
	SDLBmp apple("Assets/gfx/eple20x20.bmp");
	apple.x = (rand() % 32) * 20;
	apple.y = (rand() % 24) * 20;
	SDLBmp playerRight("Assets/gfx/head20x20.bmp");
	SDLBmp playerLeft("Assets/gfx/head20x20left.bmp");
	SDLBmp playerUp("Assets/gfx/head20x20Up.bmp");
	SDLBmp playerDown("Assets/gfx/head20x20down.bmp");
	SDLBmp body("Assets/gfx/body20x20.bmp");
	SDLBmp badapple("Assets/gfx/badeple20x20.bmp");

	// Calculate render frames per second (second / frames) (60)
	float render_fps = 1.f / 60.f;
	m_lastRender = render_fps; // set it to render immediately

	// Gameloop
	while (!gameOver)
	{
		// Update input and deltatime
		InputManager::Instance().Update();
		Timer::Instance().update();
		
		// Calculate displacement based on deltatime
		float displacement = 20.0f;

		/* Input Management */

		// Left key
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_LEFT) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_LEFT))
		{
			if (dir != DIR_RIGHT) dir = DIR_LEFT; // Cant go opposite direction
		}
		
		// Right key
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_RIGHT) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_RIGHT))
		{
			if (dir != DIR_LEFT) dir = DIR_RIGHT; // Cant go opposite direction
		}

		// Key up
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_UP) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_UP))
		{
			if (dir != DIR_DOWN) dir = DIR_UP; // Cant go opposite direction
		}

		// Key down
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_DOWN) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_DOWN))
		{
			if (dir != DIR_UP) dir = DIR_DOWN; // Cant go opposite direction
		}

		// Exit on [Esc], or window close (user X-ed out the window)
		if (InputManager::Instance().hasExit() || InputManager::Instance().KeyDown(SDL_SCANCODE_ESCAPE))
		{
			gameOver = true;
		}
		
		// Controls the tail pieces.
		last_movement += Timer::Instance().deltaTime();
		if (last_movement >= movement_timer) {
			for (int i = score; i >= 0; i--) {
				if (i > 0) {
					parts[i].x = parts[i - 1].x;
					parts[i].y = parts[i - 1].y;
					
					// Checks collision with tail.
					if (player.x == parts[i].x && player.y == parts[i].y) {
						SDL_Log("Du tapte!");
						gameOver = true;
					}
				}

				else if (i == 0) {
					parts[i].x = lastposx;
					parts[i].y = lastposy;
				}

			}

			// Controls the movement direction.
			switch (dir)
			{
			case DIR_UP:
				player.y -= displacement;
				break;
			case DIR_DOWN:
				player.y += displacement;
				break;
			case DIR_LEFT:
				player.x -= displacement;
				break;
			case DIR_RIGHT:
				player.x += displacement;
				break;
			default:
				break;
			}
			last_movement = 0.0f;
			//if ((rand() % 4) == 3) score++; // Used for testing. Increses the tail length rapidly.
		}

		// Update time since last render
		m_lastRender += Timer::Instance().deltaTime();

		// Check if it's time to render
		if (m_lastRender >= render_fps)
		{
			// Add bitmaps to renderer
			backround.draw();
			
			// Draws the bodyparts
			for (int i = 0; i < score; i++) {
				body.x = parts[i].x;
				body.y = parts[i].y;
				body.draw();
			}

			// Draws the correct face related to what direction the snake is moving
			switch (dir)
			{
			case DIR_RIGHT:
				playerRight.x = player.x;
				playerRight.y = player.y;
				playerRight.draw();
				break;
			case DIR_LEFT:
				playerLeft.x = player.x;
				playerLeft.y = player.y;
				playerLeft.draw();
				break;
			case DIR_UP:
				playerUp.x = player.x;
				playerUp.y = player.y;
				playerUp.draw();
				break;
			case DIR_DOWN:
				playerDown.x = player.x;
				playerDown.y = player.y;
				playerDown.draw();
				break;
			default:
				playerRight.draw();
				break;
			}

			// Draws apple
			apple.draw();

			if (score >= 10) {
				badapple.draw();
			}

			// Controls what happens when an apple is eaten
			if (eaten == true) {

				// Places a new apple on a random position
				apple.x = (rand() % 32) * 20;
				apple.y = (rand() % 24) * 20;
				score++;

				// Places the badapple on a random spot everytime an apple is eaten.
				badapple.x = (rand() % 32) * 20;
				badapple.y = (rand() % 24) * 20;

				// Plays a sound when an apple is eaten
				Mix_PlayChannel(-1, eat, 0);
				SDL_Log("score: %i", score);
				eaten = false;
			}
			// Draws the scorepoint on the screen
			writeText(1, 1, "Score: " + std::to_string(score)); 
			// Render window
			SDLManager::Instance().renderWindow(m_window);
			m_lastRender = 0.f;

		}
		// Checks collision with the apple
		if (player.x == apple.x && player.y == apple.y) {
			eaten = true;
		}

		// Checks collision with the bad apple.
		if (player.x == badapple.x && player.y == badapple.y && score >= 10) {
			SDL_Log("Du tapte!");
			gameOver = true;
		}

		// Checks collision with a wall
		if (player.x < 0 || player.x >= 640 || player.y < 0 || player.y >= 480) {
			SDL_Log("Du tapte!");
			gameOver = true;
		}

		// Sleep to prevent CPU exthaustion (1ms == 1000 frames per second)
		lastposx = player.x;
		lastposy = player.y;
		SDL_Delay(1);
	}
	// Plays death sound and delays 1 secound if the game is over.
	Mix_PlayChannel(-1, death, 0);
	SDL_Delay(1000);

	
}

// Function that controls the text to be written on the screen, and where.
void GameManager::writeText(int x, int y, std::string message) {
	// Checks the font.
	if (!Sans) {
		printf("Error: font: %s\n", TTF_GetError());
	}

	SDL_Color White = { 255, 255, 255 };
	SDL_Color Black = { 0, 0, 0 };

	SDL_Surface* sMessage = TTF_RenderText_Solid(Sans,message.c_str(), Black);
	SDL_Texture* Message = SDL_CreateTextureFromSurface(rend, sMessage);
	SDL_FreeSurface(sMessage);
	SDL_Rect mRect;
	mRect.x = (640/2)-50;
	mRect.y = 0;
	mRect.w = 100;
	mRect.h = 50;

	SDL_RenderCopy(rend, Message, NULL, &mRect);
	SDL_DestroyTexture(Message);

}