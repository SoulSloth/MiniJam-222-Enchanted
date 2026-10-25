#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define MAX_MOB_COUNT 100
#define SPAWN_RADIUS 200

struct Mob{
  int id;
  int posX;
  int posY;
  Vector2 speed;
} typedef Mob;

struct GameState{
  Texture2D wabbit;
  char *lastKeyPressed;
  Vector2 lastMousePos;
  Mob mobs[MAX_MOB_COUNT];
  int mobCount;
  int lastMobId;
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
      Mob newMob = {state->lastMobId, posX, posY, {10,10}};
      state->mobs[i] = newMob;
      break;
    }
  }
}

void deleteMob(struct GameState *state, int mobId){
}

void mobs(struct GameState *state){
  for(int mobIndex = 0; mobIndex < MAX_MOB_COUNT; mobIndex++){
    if(state->mobs[mobIndex].id == -1){
      continue;
    }

    state->mobs[mobIndex].posX += state->mobs[mobIndex].speed.x;
    state->mobs[mobIndex].posY += state->mobs[mobIndex].speed.y;
  }
}

void init(struct GameState *state){
  Mob nilStruct = {-1, 0, 0, {0,0}};
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    state->mobs[i] = nilStruct; 
  }
  state->lastMobId = -1;
  state->mobCount = 0;

  //Spawn mobs
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    Vector2 center = {1280/2, 720/2};
    Vector2 radius = {SPAWN_RADIUS, 0};
    float angle = ((float)i / MAX_MOB_COUNT)*PI*2;
    Vector2 rotRadius = Vector2Rotate(radius, angle);
    Vector2 finalPos = Vector2Add(rotRadius, center);
    createMob(state,(int) finalPos.x, (int) finalPos.y);
  }
}

void Input(struct GameState *state){
  if(IsKeyPressed(KEY_W)){
    state->lastKeyPressed = "W";
  }
  if(IsKeyPressed(KEY_S)){
    state->lastKeyPressed = "S";
  }
  if(IsKeyPressed(KEY_D)){
    state->lastKeyPressed = "D";
  }
  if(IsKeyPressed(KEY_A)){
    state->lastKeyPressed = "A";
  }
  state->lastMousePos = GetMousePosition();
}

void Draw(struct GameState state){
  BeginDrawing();
  ClearBackground(BLACK);
  DrawText("Hello Raylib", 200,200,20, WHITE);
  DrawText(state.lastKeyPressed, 200,400,20, WHITE);
  char buffer[100];
  int cx;
  cx = snprintf(buffer, 100, "Mouse Pos: %f, %f", state.lastMousePos.x, state.lastMousePos.y);
  DrawText(buffer, 200,400,20, WHITE);
  DrawTexture(state.wabbit,
	      GetScreenWidth()/2 - state.wabbit.width/2,
	      GetScreenHeight()/2 - state.wabbit.height/2,
	      WHITE);
  for(int i = 0; i < MAX_MOB_COUNT; i++){
    if(state.mobs[i].id == -1){
      continue;
    }
    DrawTexture(state.wabbit,
		state.mobs[i].posX - state.wabbit.width/2,
		state.mobs[i].posY - state.wabbit.height/2,
		WHITE);
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
  state->lastKeyPressed = "";
  while (!WindowShouldClose())
    {
      Input(state);
      mobs(state);
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
