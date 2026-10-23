#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

int main ()
{
  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
  // Create the window and OpenGL context
  InitWindow(800, 600, "Hello Raylib");
  // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
  SearchAndSetResourceDir("resources");
  // Load a texture from the resources directory
  Texture wabbit = LoadTexture("wabbit_alpha.png");
  // Exit if user closes window or presses escape.
  while (!WindowShouldClose())
    {
  	BeginDrawing();
  	ClearBackground(BLACK);
  	DrawText("Hello Raylib", 200,200,20,WHITE);
  	DrawTexture(wabbit, 400, 200, WHITE);
  	EndDrawing();
  }
  UnloadTexture(wabbit);
  CloseWindow();
  return 0;
}
