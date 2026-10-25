#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define MAX_MOB_COUNT 100
#define SPAWN_RADIUS 750
#define MOB_SPAWN_TIMER 1
#define PLAYER_MAX_HEALTH 10
#define SPELL_COOLDOWN 0.5
#define DEATH_TIMER 1

struct Mob{
  int id;
  int posX;
  int posY;
  Vector2 speed;
  bool hovered;
  bool enchanted;
  float deathTimer;
} typedef Mob;

struct GameState{
  bool paused;
  Texture2D wabbit;
  Texture2D character;
  Texture2D cat;
  Texture2D particle;
  Texture2D tile;
  Texture2D explosion;
  char *lastKeyPressed;
  Vector2 lastMousePos;
  Mob mobs[MAX_MOB_COUNT];
  int mobCount;
  int lastMobId;
  float mobSpawnTimer;
  float spellCoolDownTimer;
  float playerHealth;
};

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
      printf("deleted %d\n", state->mobs[i].id);
      state->mobCount--;
      break;
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
    state->mobs[mobIndex].posX += toCenterUnit.x*3;
    state->mobs[mobIndex].posY += toCenterUnit.y*3;
    if(Vector2Length(Vector2Subtract(center,mobPos)) < 50.0 ){
      deleteMob(state, state->mobs[mobIndex].id);
      state->playerHealth -= 1;
    }
  }

}

void player(struct GameState *state){
  if(state->spellCoolDownTimer > 0.0){
    printf("FOOBAR: %0.2f\n", state->spellCoolDownTimer);
    state->spellCoolDownTimer = fmax(0.0, state->spellCoolDownTimer - GetFrameTime());
    printf("FOOBARCARRY: %0.2f\n", state->spellCoolDownTimer);
  }
  else{
    state->spellCoolDownTimer = 0.0;
  }
}

void init(struct GameState *state){
  state->paused = false;
  Mob nilStruct = {-1, 0, 0, {0,0}};
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    state->mobs[i] = nilStruct; 
  }
  state->lastMobId = -1;
  state->mobCount = 0;
  state->mobSpawnTimer = MOB_SPAWN_TIMER;
  state->playerHealth = PLAYER_MAX_HEALTH;
  state->spellCoolDownTimer = 0;
  createMob(state, 100,100);
}

void Input(struct GameState *state){
  if(IsKeyPressed(KEY_P)){
    state->paused = !state->paused;
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
	deleteMob(state,state->mobs[i].id);
	state->spellCoolDownTimer = SPELL_COOLDOWN;
	printf("FOO: %0.2f\n", state->spellCoolDownTimer);
      }
    }
  }
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
  float playerHealthRatio = state.playerHealth/PLAYER_MAX_HEALTH;
  printf("%0.2f\n", state.spellCoolDownTimer);
  float spellCoolDownRatio = state.spellCoolDownTimer/SPELL_COOLDOWN;
  DrawRectangle(50, 720-75, (1280-100) * playerHealthRatio , 25, RED);
  DrawRectangle(50, 720-100, (1280-100) * spellCoolDownRatio , 25, PURPLE);
  DrawText("Hello Raylib", 200,200,20, WHITE);
  DrawText(state.lastKeyPressed, 200,400,20, WHITE);
  char buffer[100];
  int cx;
  cx = snprintf(buffer, 100, "Mouse Pos: %f, %f", state.lastMousePos.x, state.lastMousePos.y);
  DrawText(buffer, 200,400,20, WHITE);
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
  EndDrawing();
}

void startGame(struct GameState *state){
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  // Create the window and OpenGL context
  InitWindow(1280, 720, "Hello Raylib");
  // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
  SearchAndSetResourceDir("resources");
  // Exit if user closes window or presses escape.
  state->wabbit = LoadTexture("wabbit_alpha.png");
  state->character = LoadTexture("Caster.png");
  state->cat = LoadTexture("Cat.png");
  state->particle = LoadTexture("particle.png");
  state->tile = LoadTexture("Tile.png");
  state->explosion = LoadTexture("Explosion.png");
  state->lastKeyPressed = "";
  while (!WindowShouldClose())
    {
      Input(state);
      if(!state->paused){
	mobs(state);
	player(state);
      }
      Draw(*state);
  }
  UnloadTexture(state->wabbit);
  CloseWindow();
}

int main ()
{
  struct GameState state;
  init(&state);
  startGame(&state);
  return 0;
}
