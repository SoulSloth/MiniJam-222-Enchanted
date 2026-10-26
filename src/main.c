#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define MAX_MOB_COUNT 100
#define MAX_EXP_COUNT 100
#define SPAWN_RADIUS 750
#define MOB_SPAWN_TIMER 1
#define PLAYER_MAX_HEALTH 10
#define SPELL_COOLDOWN 0.5
#define DEATH_TIMER 0.5
#define EXP_LIFETIME 1.0

struct Mob{
  int id;
  int posX;
  int posY;
  Vector2 speed;
  bool hovered;
  bool enchanted;
  float deathTimer;
} typedef Mob;

struct Explosion{
  int id;
  int posX;
  int posY;
  float timer;
} typedef Explosion;

struct GameState{
  bool paused;
  bool gameWon;
  bool gameOver;
  Texture2D wabbit;
  Texture2D character;
  Texture2D cat;
  Texture2D particle;
  Texture2D tile;
  Texture2D explosion;
  Sound CatMeowSFX;
  Sound MagicSFX;
  Sound ExpSFX;
  Sound HurtSFX;
  char *lastKeyPressed;
  Vector2 lastMousePos;
  Mob mobs[MAX_MOB_COUNT];
  int mobCount;
  int lastMobId;
  float mobSpawnTimer;
  float spellCoolDownTimer;
  float playerHealth;
  Explosion exp[MAX_EXP_COUNT];
  int lastExpId;
  int expCount;
  int catsKilled;
};

void createExp(struct GameState *state, int posX, int posY){
  if(state->expCount + 1 > MAX_EXP_COUNT){
    printf("Tried to exceed max exp count: %d\n", state->expCount);
    exit(1);
  }
  state->expCount++;
  state->lastExpId++;
  for(int i = 0; i < MAX_EXP_COUNT; i++){
    if(state->exp[i].id == -1){
      Explosion newExp = { state->lastExpId, posX, posY, EXP_LIFETIME};
      state->exp[i] = newExp;
      PlaySound(state->ExpSFX);
      break;
    }
  }
}

void deleteExp(struct GameState *state, int expId){
  for(int i = 0; i < MAX_EXP_COUNT; i++){
    if(state->exp[i].id == expId){
      state->exp[i].id = -1;
      state->expCount--;
      break;
    }
  }
}

void createMob(struct GameState *state, int posX, int posY){
  if(state->mobCount + 1 > MAX_MOB_COUNT){
    printf("Tried to exceed max mob count: %d\n", state->mobCount);
    exit(1);
  }
  state->mobCount++;
  state->lastMobId++;
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    if(state->mobs[i].id == -1){
      Mob newMob = {state->lastMobId, posX, posY, {0,0}, false, false, DEATH_TIMER};
      state->mobs[i] = newMob;
      break;
    }
  }
}

void deleteMob(struct GameState *state, int mobId){
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    if(state->mobs[i].id == mobId){
      state->mobs[i].id = -1;
      state->mobCount--;
      break;
    }
  }
}

void enchantMob(struct GameState *state, int mobId){
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    if(state->mobs[i].id == mobId){
      state->mobs[i].enchanted = true;
      PlaySound(state->MagicSFX);
      break;
    }
  }
  
}

void explosions(struct GameState *state){
  for(int i = 0; i < MAX_EXP_COUNT; i++){
    if(state->exp[i].id == -1){
      continue;
    }
    state->exp[i].timer -= GetFrameTime();
    if(state->exp[i].timer < 0.0){
      deleteExp(state, state->exp[i].id);
    }
  }
}

void mobs(struct GameState *state){

  state->mobSpawnTimer -= GetFrameTime();
  if(state->mobSpawnTimer < 0){
    state->mobSpawnTimer = MOB_SPAWN_TIMER;
    float randDegree = (float)rand()/(float)(RAND_MAX/(PI*2));
    Vector2 center = {1280/2, 720/2};
    Vector2 radius = {SPAWN_RADIUS, 0};
    float angle = randDegree;
    Vector2 rotRadius = Vector2Rotate(radius, angle);
    Vector2 finalPos = Vector2Add(rotRadius, center);
    createMob(state,(int) finalPos.x, (int) finalPos.y);
    state->mobSpawnTimer = MOB_SPAWN_TIMER;
  }

  for(int mobIndex = 0; mobIndex < MAX_MOB_COUNT; mobIndex++){
    if(state->mobs[mobIndex].id == -1){
      continue;
    }
    Vector2 center = {GetScreenWidth()/2, GetScreenHeight()/2};
    Vector2 mobPos = {
      state->mobs[mobIndex].posX,
      state->mobs[mobIndex].posY};
    Vector2 toCenter = Vector2Subtract(center, mobPos);
    float toCenterL = Vector2Length(toCenter);
    Vector2 toCenterUnit = {toCenter.x/toCenterL, toCenter.y/toCenterL};
    // This fails but appears to be unit?
    //if(Vector2Length(toCenterUnit) != 1){
    //  printf("not unit vector: %0.2f %0.2f %0.2f %b\n", toCenterUnit.x, toCenterUnit.y, Vector2Length(toCenterUnit), Vector2Length(toCenterUnit) == 1.00 );
    //  exit(1);
    //}

    // Check if mob can damage player
    state->mobs[mobIndex].posX += toCenterUnit.x*3;
    state->mobs[mobIndex].posY += toCenterUnit.y*3;
    if(Vector2Length(Vector2Subtract(center,mobPos)) < 50.0 ){
      deleteMob(state, state->mobs[mobIndex].id);
      state->playerHealth -= 1;
      PlaySound(state->HurtSFX);
      if(state->playerHealth == 0){
	state->gameOver = true;
	PlaySound(state->CatMeowSFX);
      }
    }

    // Enchantment
    if(state->mobs[mobIndex].enchanted){
      state->mobs[mobIndex].deathTimer -= GetFrameTime();
      if(state->mobs[mobIndex].deathTimer < 0.0){
	state->catsKilled++;
	if(state->catsKilled == 25){
	  state->gameWon = true;
	}
	createExp(state,state->mobs[mobIndex].posX, state->mobs[mobIndex].posY);
	deleteMob(state,state->mobs[mobIndex].id);
      }
    }
  }
}

void player(struct GameState *state){
  if(state->spellCoolDownTimer > 0.0){
    state->spellCoolDownTimer = fmax(0.0, state->spellCoolDownTimer - GetFrameTime());
  }
  else{
    state->spellCoolDownTimer = 0.0;
  }
}

void init(struct GameState *state){
  state->gameWon = false;
  state->gameOver = false;
  state->paused = true;
  state->catsKilled = 0;
  Mob nilStruct = {-1, 0, 0, {0,0}};
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    state->mobs[i] = nilStruct; 
  }
  Explosion nilExp = {-1,0,0,0.0};
  for(int i = 0; i < MAX_EXP_COUNT; i++){
    state->exp[i] = nilExp; 
  }
  state->lastMobId = -1;
  state->mobCount = 0;
  state->mobSpawnTimer = MOB_SPAWN_TIMER;
  state->playerHealth = PLAYER_MAX_HEALTH;
  state->spellCoolDownTimer = 0;
  state->expCount = 0;
  state->lastExpId = -1;
}

void Input(struct GameState *state){
  if(IsKeyPressed(KEY_P)){
    state->paused = !state->paused;
  }

  if(IsKeyPressed(KEY_R) && (state->gameWon || state->gameOver)){
    init(state);
  }
  state->lastMousePos = GetMousePosition();
  int mousePosX = GetMouseX();
  int mousePosY = GetMouseY();
  int catRectWidth = state->cat.width/8;
  int catRectHeight = state->cat.height/8;
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    if(state->mobs[i].id == -1){
      continue;
    }
    int catTopLeftX = state->mobs[i].posX - state->cat.width/8;
    int catTopLeftY = state->mobs[i].posY - state->cat.height/8;
    if(mousePosX > catTopLeftX && mousePosX < catTopLeftX + state->cat.width/4 &&
       mousePosY > catTopLeftY && mousePosY < catTopLeftY + state->cat.height/4){
      state->mobs[i].hovered = true;
    }
    else{
      state->mobs[i].hovered = false;
    }
  }
  if(IsMouseButtonPressed(0)){
    for(int i = 0; i < MAX_MOB_COUNT; i++){
      if(state->mobs[i].id == -1){
	continue;
      }
      if(state->mobs[i].hovered &&
	 state->spellCoolDownTimer == 0.0){
	enchantMob(state,state->mobs[i].id);
	state->spellCoolDownTimer = SPELL_COOLDOWN;
      }
    }
  }
}

void drawPauseScreen(struct GameState state){
  DrawRectangle(50,50, 1280-100, 720-100, BLUE);
  //    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
  char introText[] = "OF COURSE IT WOULD COME TO THIS\nThose stupid felines couldn't respect the treaty.\nCouldn't leave our dwindling nests well enough alone,\nCouldn't hold back their ravonous urges!\nWell you know what? We've had enough.\nHere's an echantment that makes cats explode.\nTime to cut down their numbers.";
  DrawText(introText, 100, 100, 32, BLACK);   
  DrawText(introText, 102, 102, 32, WHITE);

  char toUnpause[] = "Click on cats to make them explode.\nPress `p` to begin the slaughter";
  DrawText(toUnpause, 100, 500, 32, BLACK);   
  DrawText(toUnpause, 102, 502, 32, WHITE);
}

void drawGameOverScreen(struct GameState state){
  DrawRectangle(50,50, 1280-100, 720-100, RED);
  //    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
  char introText[] = "Congrats, you've failed, we're all doomed.\nThe nests are empty and soon, so will the sky.";
  DrawText(introText, 100, 100, 32, BLACK);   
  DrawText(introText, 102, 102, 32, WHITE);

  char toUnpause[] = "Press 'r' to redeem yourself.";
  DrawText(toUnpause, 100, 500, 32, BLACK);   
  DrawText(toUnpause, 102, 502, 32, WHITE);
}

void drawGameWonScreen(struct GameState state){
  DrawRectangle(50,50, 1280-100, 720-100, DARKBLUE);
  //    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);
  char introText[] = "They'll call you a monster, but you did what needed to be done.";
  DrawText(introText, 100, 100, 32, BLACK);   
  DrawText(introText, 102, 102, 32, WHITE);

  char toUnpause[] = "Press 'r' to relive your glory.";
  DrawText(toUnpause, 100, 500, 32, BLACK);   
  DrawText(toUnpause, 102, 502, 32, WHITE);
}

void drawTiles(struct GameState state){
  for(int i = 0; i < 13; i++){
    for(int d = 0; d < 8; d++){
      DrawTexture(state.tile,
		  i*100,
		  d*100,
		  WHITE);
    }
  }
}

void Draw(struct GameState state){
  BeginDrawing();
  ClearBackground(BLACK);
  drawTiles(state);
  Vector2 characterPos = {GetScreenWidth()/2 - state.character.width/4,
			  GetScreenHeight()/2 - state.character.height/4};
		
  DrawTextureEx(state.character,
		characterPos,
		0,
		0.5,
		WHITE);
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    if(state.mobs[i].id == -1){
      continue;
    }
    Vector2 catPos = {
		state.mobs[i].posX - state.cat.width/8,
		state.mobs[i].posY - state.cat.height/8,
    };
    if(state.mobs[i].hovered){
      DrawTextureEx(state.cat,
		    catPos,
		    0,
		    0.25,
		    GRAY);
    }
    else{
      DrawTextureEx(state.cat,
		    catPos,
		    0,
		    0.25,
		    WHITE);
    }
  }
  for(int i = 0; i < MAX_EXP_COUNT; i++){
    if(state.exp[i].id == -1){
      continue;
    }
    Vector2 expPos = {
      state.exp[i].posX,
      state.exp[i].posY
    };
    Vector2 expCorrect = {
      state.explosion.width/8,
      state.explosion.height/8
    };
    DrawTextureEx(state.explosion,
		  Vector2Subtract(expPos,expCorrect),
		  0,
		  0.25,
		  WHITE);
  }
  float playerHealthRatio = state.playerHealth/PLAYER_MAX_HEALTH;
  float spellCoolDownRatio = state.spellCoolDownTimer/SPELL_COOLDOWN;
  DrawRectangle(50, 720-75, (1280-100) * playerHealthRatio , 25, RED);
  DrawRectangle(50, 720-100, (1280-100) * spellCoolDownRatio , 25, PURPLE);
  char buffer[50];
  sprintf(buffer, "Nests Avenged: %d", state.catsKilled);
  DrawText(buffer, 10, 10, 48, WHITE);
  if(state.paused){
    drawPauseScreen(state);
  }

  if(state.gameWon){
    drawGameWonScreen(state);
  }
  if(state.gameOver){
    drawGameOverScreen(state);
  }
  EndDrawing();
}

void startGame(struct GameState *state){
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  // Create the window and OpenGL context
  InitWindow(1280, 720, "Hello Raylib");
  InitAudioDevice(); 
  // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
  SearchAndSetResourceDir("resources");
  // Exit if user closes window or presses escape.
  state->wabbit = LoadTexture("wabbit_alpha.png");
  state->character = LoadTexture("Caster.png");
  state->cat = LoadTexture("Cat.png");
  state->particle = LoadTexture("particle.png");
  state->tile = LoadTexture("Tile.png");
  state->explosion = LoadTexture("Explosion.png");
  state->CatMeowSFX = LoadSound("CatMeow.ogg");
  state->MagicSFX = LoadSound("Misc Popup Magic 006.wav");
  state->ExpSFX = LoadSound("Pot Explosion.wav");
  state->HurtSFX = LoadSound("Vein Spurt C.wav");
  state->lastKeyPressed = "";
  while (!WindowShouldClose())
    {
      Input(state);
      if(!state->paused && !state->gameOver && !state->gameWon){
	mobs(state);
	player(state);
	explosions(state);
      }
      Draw(*state);
  }
  UnloadTexture(state->wabbit);
  UnloadTexture(state->character);
  UnloadTexture(state->cat);
  UnloadTexture(state->particle);
  UnloadTexture(state->explosion);
  UnloadSound(state->CatMeowSFX);
  UnloadSound(state->MagicSFX);
  UnloadSound(state->ExpSFX);
  UnloadSound(state->HurtSFX);
  CloseWindow();
}

int main ()
{
  struct GameState state;
  init(&state);
  startGame(&state);
  return 0;
}
