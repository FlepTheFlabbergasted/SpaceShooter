
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#define FPS 60
#define FRAME_TIME 1000 / FPS
uint32_t currentTime = 0, prevTime = 0, asteroidTime = 0, lastAsteroidTime = 0;
time_t t;

//Screen dimension constants
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

#define true 1
#define false 0

struct entity{
	float x,y,width,height;
	float dx, dy;
	SDL_Texture* texture;
	float angle;
	int laserExist;
	float speed, pos;
	int dead;
};

struct entity ship;
struct entity laser;
struct entity asteroidPos[8];

SDL_Texture* asteroids = NULL;
SDL_Texture* exhaustFlame = NULL;
SDL_Texture* spaceShipLaser = NULL;
SDL_Texture* beams = NULL;
SDL_Texture* SpaceShip = NULL;
SDL_Texture* YOUDIED = NULL;

Mix_Music *backgroundMusic = NULL;
Mix_Chunk *YOUDIEDSOUND = NULL;
Mix_Chunk *PEWPEW = NULL;
Mix_Chunk *TENPOINTS = NULL, *TWENTYPOINTS = NULL, *FORTYPOINTS = NULL;
Mix_Chunk *SIXTYPOINTS = NULL;
Mix_Chunk *EIGHTYPOINTS = NULL;

TTF_Font *points = NULL;
SDL_Texture *pointsTexture = NULL;
SDL_Surface *pointsSurface = NULL;

// Directions of the rectangles
int up = false, down = false, left = false, right = false, spaceBar = false;
int rotLeft = 0, rotRight = 0;

void movAng(void);
float degToRad(float degrees);
float radToDeg(float rad);

bool init();
bool loadMedia();
void closeSDL();

//SDL_Surface* loadSurface(char* filename);
SDL_Texture* loadTexture( char* filename);
//SDL_Texture* loadTexture2( char* filename);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;
//The window renderer
SDL_Renderer* gRenderer = NULL;
//Current displayed texture
SDL_Texture* gTexture = NULL;

//Main loop flag
bool quit = false;

int WinMain( int argc, char* args[] ){

	// Show/hide hitboxes
	int hitbox = false;
	int gamePoints = 0;

	srand( (unsigned) time(&t) );

	ship.x = SCREEN_WIDTH/2 -25;
	ship.y = SCREEN_HEIGHT/2 - 30;
	ship.width = 173/4;
	ship.height = 291/4;
	ship.dx = 0;
	ship.dy = 0;
	ship.angle = -90;
	ship.texture = SpaceShip;

	laser.x = SCREEN_WIDTH/2 -25;
	laser.y = SCREEN_HEIGHT/2 - 30;
	laser.width = 74/2;
	laser.height = 105/2;
	laser.dx = 0;
	laser.dy = 0;
	laser.angle = -90;
	laser.texture = spaceShipLaser;
	laser.laserExist = 0;

	// Determine how often to animate exhaust
	int currentFrame = 0;
	int currentExhaustClip = 0;
	const int exhaustFrames = 8;
	int exhaustAcc = 15;
	SDL_Rect exhaustClips[exhaustFrames];
	for( int i = 0; i < 8; i++){
		exhaustClips[ i ].x =  i * 34,
		exhaustClips[ i ].y =  0,
		exhaustClips[ i ].w = 30, // eller 32
		exhaustClips[ i ].h = 64; // 64
	}

	int currentAsteroid = 0;
	int ASTEROIDS_ROW3 = 256;
	const int ASTEROIDS_MAX = 8;
	int ASTEROID_WIDTH = 128, ASTEROID_HEIGHT = 128;
	SDL_Rect asteroidClips[ASTEROIDS_MAX];
	for(int i = 0; i < ASTEROIDS_MAX; i++){
		asteroidClips[i].x = i * ASTEROID_WIDTH;
		asteroidClips[i].y = ASTEROIDS_ROW3;
		asteroidClips[i].w = ASTEROID_WIDTH;
		asteroidClips[i].h = ASTEROID_HEIGHT;
	}

	int spawnTime = 500, fasterSpeed = 700;
	for(currentAsteroid = 0; currentAsteroid < ASTEROIDS_MAX; currentAsteroid++){
		asteroidPos[currentAsteroid].dead = true;
	}

	//Event handler
	SDL_Event e;

	//Start up SDL and create window
	if( !init() ){
		printf( "Failed to initialize!\n" );
	}
	else{
		//Load media
		if( !loadMedia() ){
			printf( "Failed to load media!\n" );
		}
		else{
			//Play background music
			Mix_PlayMusic(backgroundMusic, -1);

			//While application is running
			while( !quit ){
				currentTime = SDL_GetTicks();
				if(currentTime - prevTime >= FRAME_TIME){
					currentFrame++;
					//printf("%d\n", FRAME_TIME);
					//Handle events on queue
					while( SDL_PollEvent( &e ) != 0 ){
						//User requests quit
						if( e.type == SDL_QUIT )
						{
							quit = true;
						}
						else if(e.type == SDL_KEYDOWN && e.key.repeat == 0){
							switch(e.key.keysym.sym){
							case SDLK_UP:
								up = true;
								break;
							case SDLK_LEFT:
								rotLeft = true;
								//left = true;
								break;
							case SDLK_RIGHT:
								rotRight = true;
								//right = true;
								break;
							case SDLK_SPACE:
								if (!laser.laserExist){
									laser.laserExist = true;
									laser.x = ship.x + 4;
									laser.y = ship.y - 4;
									laser.angle = ship.angle;
									laser.dx = 20 * cos(degToRad(laser.angle));
									laser.dy = 20 * sin(degToRad(laser.angle));

									//Play PEWPEW sound
									Mix_PlayChannel( -1, PEWPEW, 0 ); // (Nearest channel, sound, repeats)
								}
								break;
							case SDLK_h:
								hitbox = true;
								break;
							}
						}
						else if (e.type == SDL_KEYUP && e.key.repeat == 0){
							switch( e.key.keysym.sym ){
							case SDLK_UP:
								up = false;
								break;
							case SDLK_DOWN:
								down = false;
								break;
							case SDLK_LEFT:
								rotLeft = false;
								//left = false;
								break;
							case SDLK_RIGHT:
								rotRight = false;
								//right = false;
								break;
							case SDLK_SPACE:
								spaceBar = false;
								break;
							case SDLK_h:
								hitbox = false;
								break;
							}
						}
					}
							// Movement and stuff
							movAng();

							asteroidTime = SDL_GetTicks();
							for(currentAsteroid = 0; currentAsteroid < ASTEROIDS_MAX; currentAsteroid++){
								if( asteroidPos[currentAsteroid].dead && ((asteroidTime - lastAsteroidTime) >= spawnTime)){
									int Pos = (rand() % 2);
									if(Pos == 0){					// Spawn kl 12 ..
										asteroidPos[currentAsteroid].x = (rand() % SCREEN_WIDTH );
										asteroidPos[currentAsteroid].y = 30 - ASTEROID_HEIGHT;
										asteroidPos[currentAsteroid].angle = ((atan2( (ship.y - asteroidPos[currentAsteroid].y), (ship.x - asteroidPos[currentAsteroid].x)) ) * 180.0f ) / M_PI;
										asteroidPos[currentAsteroid].speed = (400 + (rand() % fasterSpeed)) / 800.0f;
										asteroidPos[currentAsteroid].dx = cos(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dy = sin(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dead = false;
									}
									else if(Pos == 1){
										asteroidPos[currentAsteroid].x = 30 - ASTEROID_WIDTH; // Spawn kl 9 ...
										asteroidPos[currentAsteroid].y =  rand() % SCREEN_HEIGHT;
										asteroidPos[currentAsteroid].angle = ((atan2( (ship.y - asteroidPos[currentAsteroid].y), (ship.x - asteroidPos[currentAsteroid].x)) ) * 180.0f ) / M_PI;
										asteroidPos[currentAsteroid].speed = (400 + (rand() % fasterSpeed)) / 800.0f;
										asteroidPos[currentAsteroid].dx = cos(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dy = sin(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dead = false;
									}
									lastAsteroidTime = SDL_GetTicks();
									if(spawnTime > 0){
										spawnTime = spawnTime - 50;
									}
									fasterSpeed = fasterSpeed + 50;
									if(fasterSpeed == 2500){ // När du träffat 30 st så resettas spawntiden
										spawnTime = 1250;
									}

								}
							}

							for(currentAsteroid = 0; currentAsteroid < ASTEROIDS_MAX; currentAsteroid++){
								if(asteroidPos[currentAsteroid].dead == false){
									asteroidPos[currentAsteroid].x += asteroidPos[currentAsteroid].dx;//cos(degToRad(asteroidPos[currentAsteroid].angle) * asteroidPos[currentAsteroid].speed);
									asteroidPos[currentAsteroid].y += asteroidPos[currentAsteroid].dy;//sin(degToRad(asteroidPos[currentAsteroid].angle) * asteroidPos[currentAsteroid].speed);

									// Respawn asteroids who wander off screen
									if(asteroidPos[currentAsteroid].x > SCREEN_WIDTH){
										asteroidPos[currentAsteroid].x = - ASTEROID_WIDTH;
										asteroidPos[currentAsteroid].angle = ((atan2( (ship.y - asteroidPos[currentAsteroid].y), (ship.x - asteroidPos[currentAsteroid].x)) ) * 180.0f ) / M_PI;
										asteroidPos[currentAsteroid].dx = cos(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dy = sin(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
									}
									else if((asteroidPos[currentAsteroid].x + ASTEROID_WIDTH) < 0){
										asteroidPos[currentAsteroid].x = SCREEN_WIDTH;
										asteroidPos[currentAsteroid].angle = ((atan2( (ship.y - asteroidPos[currentAsteroid].y), (ship.x - asteroidPos[currentAsteroid].x)) ) * 180.0f ) / M_PI;
										asteroidPos[currentAsteroid].dx = cos(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dy = sin(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
									}

									if(asteroidPos[currentAsteroid].y > SCREEN_HEIGHT){
										asteroidPos[currentAsteroid].y = - ASTEROID_HEIGHT;
										asteroidPos[currentAsteroid].angle = ((atan2( (ship.y - asteroidPos[currentAsteroid].y), (ship.x - asteroidPos[currentAsteroid].x)) ) * 180.0f ) / M_PI;
										asteroidPos[currentAsteroid].dx = cos(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dy = sin(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
									}
									else if((asteroidPos[currentAsteroid].y  + ASTEROID_HEIGHT) < 0) {
										asteroidPos[currentAsteroid].y = SCREEN_HEIGHT;
										asteroidPos[currentAsteroid].angle = ((atan2( (ship.y - asteroidPos[currentAsteroid].y), (ship.x - asteroidPos[currentAsteroid].x)) ) * 180.0f ) / M_PI;
										asteroidPos[currentAsteroid].dx = cos(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
										asteroidPos[currentAsteroid].dy = sin(degToRad(asteroidPos[currentAsteroid].angle)) * asteroidPos[currentAsteroid].speed;
									}
								}
							}

							//Clear screen
							SDL_RenderClear( gRenderer );
							//Render texture to screen
							SDL_RenderCopy( gRenderer, gTexture, NULL, NULL );

							if(laser.laserExist){
								SDL_Rect laserPointer = {206, 308, 60, 92}; // x,y,width,height för röda lasern i bilden beams
								SDL_Rect spaceShipLaserRec = { laser.x, laser.y, laser.width, laser.height }; // Vars de spawnar/hur stor
								SDL_Point laserCenter = { laser.width/2.0f - 1, laser.height/2.0f + 14 };
								SDL_RenderCopyEx(gRenderer, beams, &laserPointer, &spaceShipLaserRec, laser.angle + 90, &laserCenter, SDL_FLIP_NONE);
							}

							SDL_Rect spaceShipRec = { ship.x, ship.y, ship.width, ship.height };
							SDL_RenderCopyEx(gRenderer, SpaceShip, NULL, &spaceShipRec, ship.angle + 90, NULL, SDL_FLIP_NONE);


							if(up){
								SDL_Rect exhaustRec = { (ship.x + ship.width/2) - 16, (ship.y + ship.height) - 13, 30, exhaustAcc };
								SDL_Point exhaustCenter = { 15, -22};
								SDL_RenderCopyEx(gRenderer, exhaustFlame, &exhaustClips[currentExhaustClip], &exhaustRec, ship.angle+90, &exhaustCenter, SDL_FLIP_NONE);

								if(exhaustAcc < 65 ){
									exhaustAcc += 5;
								}

								if(currentFrame > 3){
									currentExhaustClip++;
									if(currentExhaustClip >= exhaustFrames){
										currentExhaustClip = 0;
									}
									currentFrame = 0;
								}
							}
							else{
								for(int i = 0; i < 8; i++){
									exhaustAcc = 15;
								}
							}

							for( currentAsteroid = 0; currentAsteroid < ASTEROIDS_MAX; currentAsteroid++ ){
								if(asteroidPos[currentAsteroid].dead == false){
									SDL_Rect asteroidRec = { asteroidPos[currentAsteroid].x, asteroidPos[currentAsteroid].y, ASTEROID_WIDTH, ASTEROID_HEIGHT };
									SDL_RenderCopy(gRenderer, asteroids, &asteroidClips[currentAsteroid], &asteroidRec);

									// Hitbox for asteroids
									SDL_Rect asteroidHitbox = { asteroidPos[currentAsteroid].x + 25,asteroidPos[currentAsteroid].y + 25, ASTEROID_WIDTH - 50, ASTEROID_HEIGHT - 50};
									SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x00, 0x00 );

									// Hitbox for spaceship
									SDL_Rect spaceShipHitbox = { ship.x, ship.y + 19, ship.width, ship.height/2 };
									SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x00, 0x00 );

									if(hitbox){
										SDL_RenderDrawRect( gRenderer, &spaceShipHitbox );
										SDL_RenderDrawRect( gRenderer, &asteroidHitbox );
									}

									SDL_Point laserPoint1 = {laser.x + 15 - (37/2)*cos(degToRad(laser.angle)), laser.y + 32 - (30/2)*sin(degToRad(laser.angle))};
									SDL_Point laserPoint2 = {laser.x + 15 - (37/2)*cos(degToRad(180+laser.angle)), laser.y + 32 - (30/2)*sin(degToRad(180+laser.angle))};

									if(laser.laserExist){
										if( SDL_IntersectRectAndLine(&asteroidHitbox, &laserPoint1.x, &laserPoint1.y, &laserPoint2.x, &laserPoint2.y) ) {
											//printf("HIT ASTEROID NR: %d\n", currentAsteroid);
											laser.laserExist = 0;
											asteroidPos[currentAsteroid].dead = true;
											gamePoints++;

											if(gamePoints == 10){
												Mix_PlayChannel(-1, TENPOINTS, 0);
											}
											else if(gamePoints == 20){
												Mix_PlayChannel(-1, TWENTYPOINTS, 0);
											}
											else if(gamePoints == 40){
												Mix_PlayChannel(-1, FORTYPOINTS, 0);
											}
											else if(gamePoints == 60){
												Mix_PlayChannel(-1, SIXTYPOINTS, 0);
											}
											else if(gamePoints == 80){
												Mix_PlayChannel(-1, EIGHTYPOINTS, 0);
											}
											/*SDL_Rect asteroidMiniClip = {128, 256, 128, 128}; // Ta asteroiden från bilden
											SDL_Rect asteroidMiniSpawnRec = { (asteroidPos[currentAsteroid].x + ASTEROID_WIDTH/2) , (asteroidPos[currentAsteroid].y + ASTEROID_HEIGHT/2), ASTEROID_WIDTH/4, ASTEROID_HEIGHT/4 }; // Vars den spawnar och hur stor
											SDL_RenderCopyEx( gRenderer, asteroids, &asteroidMiniClip, &asteroidMiniSpawnRec, laser.angle + 90, NULL, 0);
											SDL_RenderCopyEx( gRenderer, asteroids, &asteroidMiniClip, &asteroidMiniSpawnRec, laser.angle - 90, NULL, 0); */
										}
									}

									if( SDL_HasIntersection(&asteroidHitbox, &spaceShipHitbox) ){

										Mix_PauseMusic(); // Paus backgroundMusic
										//Play YOUDIEDSOUND
										Mix_PlayChannel(-1, YOUDIEDSOUND, 0);  // (Nearest channel, sound, repeats)

										SDL_Rect YOUDIEDRECCUT = { 0, 0, 1487, 170 };
										SDL_Rect YOUDIEDREC = { 0, SCREEN_HEIGHT/3, SCREEN_WIDTH, SCREEN_HEIGHT/3 };
										SDL_SetTextureBlendMode(YOUDIED, SDL_BLENDMODE_BLEND);

										currentTime = SDL_GetTicks();
										int alpha = 0, quitGame = 0;
										while( !quitGame ){
											alpha = ( ((SDL_GetTicks() - currentTime)/100) % 255 );
											if(SDL_GetTicks() - currentTime > 1800){
												alpha = 255;
												quitGame = 1;
											}
											SDL_SetTextureAlphaMod(YOUDIED, alpha);
											SDL_RenderCopy( gRenderer, YOUDIED, &YOUDIEDRECCUT, &YOUDIEDREC );

											char buffer[64] = {0};
											sprintf(buffer, "%d points", gamePoints);
											SDL_Color foregroundColor = {255, 255, 0};
											pointsSurface = TTF_RenderText_Solid(points, buffer, foregroundColor);
											pointsTexture = SDL_CreateTextureFromSurface( gRenderer, pointsSurface );
											SDL_Rect fpsTextDst = {10, 10, 80, 17};
											SDL_RenderCopy(gRenderer, pointsTexture, NULL, &fpsTextDst);

											SDL_RenderPresent( gRenderer );
										}

										while( !((SDL_PollEvent(&e) &&  e.key.keysym.sym == SDLK_ESCAPE) || e.type == SDL_QUIT) );
										closeSDL();
									}

									// Draw laser hitbox
									//SDL_RenderDrawLine( gRenderer, laserPoint1.x, laserPoint1.y, laserPoint2.x, laserPoint2.y);
								}
							}

							char buffer[64] = {0};
							sprintf(buffer, "Points: %d", gamePoints);
							SDL_Color foregroundColor = {255, 255, 0};
							pointsSurface = TTF_RenderText_Solid(points, buffer, foregroundColor);
							pointsTexture = SDL_CreateTextureFromSurface( gRenderer, pointsSurface );
							SDL_Rect fpsTextDst = {10, 10, 80, 17};
							SDL_RenderCopy(gRenderer, pointsTexture, NULL, &fpsTextDst);

							//Update screen
							SDL_RenderPresent( gRenderer );

							prevTime = SDL_GetTicks();
				}
				else{
					SDL_Delay(FRAME_TIME - (currentTime-prevTime));
				}

			}
		}
	}

	//Free resources and close SDL
	closeSDL();

	return 0;
}

void movAng(){

	// Keep the ship within the screen
	if(ship.x > SCREEN_WIDTH - ship.width/2){
		ship.x = 0 - ship.width/2;
	}
	else if(ship.x + ship.width / 2 < 0) {
		ship.x = SCREEN_WIDTH - ship.width / 2;
	}

	if(ship.y > SCREEN_HEIGHT - ship.height / 2)
		ship.y = 0 - ship.height / 2;
	else if(ship.y  + ship.height / 2 < 0) {
		ship.y = SCREEN_HEIGHT - ship.height / 2;
	}

	// Movement and rotation
	if(rotLeft){
		ship.angle -=  3;
	}else if (rotRight){
		ship.angle += 3;
	}

	if(up){
		ship.dx += cos(degToRad(ship.angle)) / 5.0f;
		ship.dy += sin(degToRad(ship.angle)) / 5.0f;
	}else{
		ship.dx *= 0.950;
		ship.dy *= 0.950;
	}

	if(laser.x > SCREEN_WIDTH - laser.width){
		laser.laserExist = 0;
		laser.dx = 0;
		laser.dy = 0;
	}
	else if(laser.x + laser.width < 0) {
		laser.laserExist = 0;
		laser.dx = 0;
		laser.dy = 0;
	}

	if(laser.y > SCREEN_HEIGHT - laser.height){
		laser.laserExist = 0;
		laser.dx = 0;
		laser.dy = 0;
	}
	else if(laser.y + laser.height < 0) {
		laser.laserExist = 0;
		laser.dx = 0;
		laser.dy = 0;
	}

	laser.x += laser.dx;
	laser.y += laser.dy;

	ship.x += ship.dx;
	ship.y += ship.dy;
}

float degToRad(float degrees){

	float rad = degrees * (M_PI/180.0f);

	return rad;
}

float radToDeg(float rad){

	float degree = rad * (180.0f/M_PI);

	return degree;
}

bool init(){

	    //Initialization flag
	    bool success = true;

	    //Initialize SDL
	    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	    {
	        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	        success = false;
	    }
	    else{

	        //Create window
	        gWindow = SDL_CreateWindow( "SDL is a bitch", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
	        if( gWindow == NULL ){
	        	printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
	            success = false;
	        }
	        else{
	        	//Create renderer for window
				gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
				if( gRenderer == NULL )
				{
					printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
					success = false;
				}
				else{
					//Initialize renderer color
					SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

					//Initialize PNG loading
				   int imgFlags = IMG_INIT_PNG;
				   if( !( IMG_Init( imgFlags ) & imgFlags ) )
				   {
					   printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					   success = false;
				   }

				   //Initialize SDL_mixer
				  if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) // (Frekvens, sample format, chunk size , 2048 bytes -> 2kilobytes)
				  {
					  printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
					  success = false;
				  }

				  if(TTF_Init() < 0) {
						  printf("TTF Init failed: %s\n", TTF_GetError());
						  success = false;
					  } else {
						  points = TTF_OpenFont("arialbd.ttf", 15);
					  if(points == NULL) {
						  printf("Error loading font. %s\n", TTF_GetError());
						  success = false;
					  }
				  }

				   //Get window surface
				   gScreenSurface = SDL_GetWindowSurface( gWindow );

				}
	        }
	    }

	    return success;
}


bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Load splash image
    gTexture = loadTexture( "space.jpg" );
    if( gTexture == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", "interesting", SDL_GetError() );
        success = false;
    }

    SpaceShip = loadTexture( "SpaceShip.png" );
	if( SpaceShip == NULL )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "SpaceShip", SDL_GetError() );
		success = false;
	}

	beams = loadTexture( "beams.png" );
	if( beams == NULL )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "spaceShipLaser", SDL_GetError() );
		success = false;
	}

	exhaustFlame = loadTexture( "exhaust.png" );
	if( exhaustFlame == NULL  )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "exhaustFlame", SDL_GetError() );
		success = false;
	}

	asteroids = loadTexture( "asteroids.png" );
	if( asteroids == NULL  )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "asteroids", SDL_GetError() );
		success = false;
	}

	YOUDIED = loadTexture( "YOUDIED.PNG" );
	if( YOUDIED == NULL  )
	{
		printf( "Unable to load image %s! SDL Error: %s\n", "YOUDIED", SDL_GetError() );
		success = false;
	}

	//Load music
	backgroundMusic = Mix_LoadMUS( "background.mp3" );
	if( backgroundMusic == NULL )
	{
		printf( "Failed to load music! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	//Load sound effects
	YOUDIEDSOUND = Mix_LoadWAV( "YOUDIEDSOUND.wav" );
	if( YOUDIEDSOUND == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	PEWPEW = Mix_LoadWAV( "PEWPEW.wav" );
	if( PEWPEW == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	TENPOINTS = Mix_LoadWAV( "TENPOINTS.wav" );
	if( TENPOINTS == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	TWENTYPOINTS = Mix_LoadWAV( "TWENTYPOINTS.wav" );
	if( TWENTYPOINTS == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	FORTYPOINTS = Mix_LoadWAV( "FORTYPOINTS.wav" );
	if( FORTYPOINTS  == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	SIXTYPOINTS = Mix_LoadWAV( "SIXTYPOINTS.wav" );
	if( SIXTYPOINTS  == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

	EIGHTYPOINTS = Mix_LoadWAV( "EIGHTYPOINTS.wav" );
	if( EIGHTYPOINTS  == NULL )
	{
		printf( "Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
		success = false;
	}

    return success;
}

/*SDL_Surface* loadSurface(char* filename) {

	//The final optimized image
	SDL_Surface* optimizedSurface = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( filename);

	if( loadedSurface == NULL ) {
		printf( "Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError() );
	} else {
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);

		if( optimizedSurface == NULL ) {
			printf( "Unable to optimize image %s! SDL Error: %s\n", filename, SDL_GetError() );
		}
		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	return optimizedSurface;
}*/

SDL_Texture* loadTexture( char* filename){

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( filename );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError() );
    }
    else
    {

      //  SDL_SetColorKey(loadedSurface, true, SDL_MapRGB(loadedSurface->format,255,255,255));

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", filename, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

/*
SDL_Texture* loadTexture2( char* filename){

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( filename );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", filename, IMG_GetError() );
    }
    else
    {

        SDL_SetColorKey(loadedSurface, true, SDL_MapRGB(loadedSurface->format,0,0,0));

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", filename, SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}
*/

void closeSDL(){

	//Free loaded image
	SDL_DestroyTexture( gTexture );
	SDL_DestroyTexture( SpaceShip );
	SDL_DestroyTexture( spaceShipLaser );
	SDL_DestroyTexture( exhaustFlame );
	SDL_DestroyTexture( beams );
	SDL_DestroyTexture( asteroids );
	SDL_DestroyTexture( YOUDIED );
	gTexture = NULL;
	SpaceShip = NULL;
	spaceShipLaser = NULL;
	exhaustFlame = NULL;
	beams = NULL;
	asteroids = NULL;
	YOUDIED = NULL;

	//Free the sound effects
	Mix_FreeChunk( YOUDIEDSOUND );
	Mix_FreeChunk( PEWPEW );
	Mix_FreeChunk( TENPOINTS );
	Mix_FreeChunk( TWENTYPOINTS );
	YOUDIEDSOUND = NULL;
	PEWPEW = NULL;
	TENPOINTS = NULL;
	TWENTYPOINTS = NULL;

	//Free the music
	Mix_FreeMusic( backgroundMusic );
	backgroundMusic = NULL;

	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	quit = true;
}

