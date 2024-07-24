#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <raylib.h>

#define MATH_IMPL
#include "math.h"

#define DS_IMPL
#include "ds.h"

#define m_at(m, i, j) (m).buf[(i) + (j) * (m).w]

// Doesn't work with anything other than 6, just avoiding magic numbers
#define NUM_FACES 6

typedef struct {
  int faces[NUM_FACES + 1];
  int top;
  // enum {
  //   NORTH = 
  // } dir;
} Dice;

Dice def_dice = {
  .faces = { 0, 1, 2, 3, 6, 5, 4, },
};

typedef struct {
  Dice *buf;
  int w, h;
} Map;

void alloc_map(Map *m) {
  m->buf = malloc(m->w * m->h * sizeof(m->buf[0]));
  memset(m->buf, 0.0, m->w * m->h * sizeof(m->buf[0]));
}

int main() {
  srand(time(0));

  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(
    900.0f/1366.0f * GetMonitorWidth(GetCurrentMonitor()),
    700.0f/768.0f * GetMonitorHeight(GetCurrentMonitor()),
    "dice"
  );
  SetTargetFPS(60);

  Arena scratch = arena_init(256 * 1024 * 1024);

  Texture2D face_texts[NUM_FACES];
  {
    char literal[] = "recs/face-1.png";
    char *path = arena_alloc(&scratch, sizeof(literal));
    memcpy(path, literal, sizeof(literal));
    for (int i = 0; i < NUM_FACES; i++) {
      path[10] = '0' + i + 1;
      face_texts[i] = LoadTexture(path);
    }
  }

  Map map = { .w = 30, .h = 30, };
  alloc_map(&map);
  for (int i = 0; i < map.w; i++) {
    for (int j = 0; j < map.h; j++) {
      m_at(map, i, j) = def_dice;
      m_at(map, i, j).top = randi(1, NUM_FACES);
    }
  }

  Camera2D cam = {
    .zoom = 0.3,
    .target = {
      map.w * face_texts[0].width/2, 
      map.h * face_texts[0].height/2,
    },
  };
  while (!WindowShouldClose()) {
    arena_free_all(&scratch);

    float delta = GetFrameTime();

    cam.offset = (Vector2){ GetRenderWidth()/2, GetRenderHeight()/2, };

    if (IsKeyDown(KEY_N)) cam.zoom *= 1.0 + 2 * delta;
    if (IsKeyDown(KEY_M)) cam.zoom /= 1.0 + 2 * delta;

    if (IsKeyDown(KEY_H)) cam.target.x -= 1000 * delta / cam.zoom;
    if (IsKeyDown(KEY_J)) cam.target.y += 1000 * delta / cam.zoom;
    if (IsKeyDown(KEY_K)) cam.target.y -= 1000 * delta / cam.zoom;
    if (IsKeyDown(KEY_L)) cam.target.x += 1000 * delta / cam.zoom;

    BeginDrawing();
    BeginMode2D(cam);
    ClearBackground(WHITE);
    {
      for (int i = 0; i < map.w; i++) {
        for (int j = 0; j < map.h; j++) {
          int face = m_at(map, i, j).faces[m_at(map, i, j).top];
          Texture2D text = face_texts[face - 1];
          DrawTextureEx(
            text,
            (Vector2){ i * text.width, j * text.height, },
            0.0, 1.0, WHITE
          );
        }
      }
    }
    EndMode2D();
    EndDrawing();
  }

  return 0;
}
