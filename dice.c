#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// TODO: Make cross-platform
#include <unistd.h>

int usleep(int usec);

#include <raylib.h>

#define MATH_IMPL
#include "math.h"

#define DS_IMPL
#include "ds.h"

#define front_at(m, i, j) (m).front[(i) + (j) * (m).w]
#define back_at(m, i, j) (m).back[(i) + (j) * (m).w]

// Doesn't work with anything other than 6, just avoiding magic numbers
typedef enum {
  DIR_TOP = 0,
  DIR_BOTTOM,
  DIR_NORTH,
  DIR_SOUTH,
  DIR_EAST,
  DIR_WEST,
  NUM_DIRS,
} Direction;

typedef struct {
  int faces[NUM_DIRS];
} Dice;

Dice def_dice = {
  .faces = {
    [DIR_TOP] = 1,
    [DIR_BOTTOM] = 6,
    [DIR_NORTH] = 2,
    [DIR_SOUTH] = 5,
    [DIR_EAST] = 3,
    [DIR_WEST] = 4
  },
};

typedef struct {
  Dice *front;
  Dice *back;
  int w, h;
} Map;

void alloc_map(Map *m) {
  Dice *buf = calloc(2 * m->w * m->h, sizeof(m->front[0]));
  m->front = buf;
  buf += m->w * m->h;
  m->back = buf;
}

Dice rotate_dice(Dice dice, Direction dir) {
  switch (dir) {
  case DIR_NORTH:
    return (Dice) {
      .faces = {
        [DIR_TOP] = dice.faces[DIR_SOUTH],
        [DIR_BOTTOM] = dice.faces[DIR_NORTH],
        [DIR_NORTH] = dice.faces[DIR_TOP],
        [DIR_SOUTH] = dice.faces[DIR_BOTTOM],
        [DIR_EAST] = dice.faces[DIR_EAST],
        [DIR_WEST] = dice.faces[DIR_WEST],
      },
    };
  case DIR_EAST:
    return (Dice) {
      .faces = {
        [DIR_TOP] = dice.faces[DIR_WEST],
        [DIR_BOTTOM] = dice.faces[DIR_EAST],
        [DIR_NORTH] = dice.faces[DIR_NORTH],
        [DIR_SOUTH] = dice.faces[DIR_SOUTH],
        [DIR_EAST] = dice.faces[DIR_TOP],
        [DIR_WEST] = dice.faces[DIR_BOTTOM],
      },
    };
  case DIR_SOUTH:
    return (Dice) {
      .faces = {
        [DIR_TOP] = dice.faces[DIR_NORTH],
        [DIR_BOTTOM] = dice.faces[DIR_SOUTH],
        [DIR_NORTH] = dice.faces[DIR_BOTTOM],
        [DIR_SOUTH] = dice.faces[DIR_TOP],
        [DIR_EAST] = dice.faces[DIR_EAST],
        [DIR_WEST] = dice.faces[DIR_WEST],
      },
    };
  case DIR_WEST:
    return (Dice) {
      .faces = {
        [DIR_TOP] = dice.faces[DIR_EAST],
        [DIR_BOTTOM] = dice.faces[DIR_WEST],
        [DIR_NORTH] = dice.faces[DIR_NORTH],
        [DIR_SOUTH] = dice.faces[DIR_SOUTH],
        [DIR_EAST] = dice.faces[DIR_BOTTOM],
        [DIR_WEST] = dice.faces[DIR_TOP],
      },
    };
  default: assert(0 && "Invalid direction");
  }
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

  Texture2D face_texts[NUM_DIRS];
  {
    char literal[] = "recs/face-1.png";
    char *path = arena_alloc(&scratch, sizeof(literal));
    memcpy(path, literal, sizeof(literal));
    for (int i = 0; i < NUM_DIRS; i++) {
      path[10] = '0' + i + 1;
      face_texts[i] = LoadTexture(path);
    }
  }

  Map map = { .w = 100, .h = 100, };
  alloc_map(&map);
  for (int i = 0; i < map.w; i++) {
    for (int j = 0; j < map.h; j++) {
      back_at(map, i, j) = randi(0, 6) == 0
        ? rotate_dice(def_dice, randi(DIR_BOTTOM + 1, NUM_DIRS - 1))
        : (Dice) {0};
    }
  }

  Camera2D cam = {
    .zoom = 0.3,
    .target = {
      map.w * face_texts[0].width/2, 
      map.h * face_texts[0].height/2,
    },
    .zoom = 0.1,
  };
  while (!WindowShouldClose()) {
    arena_free_all(&scratch);

    /* ===== Input ===== */

    float delta = GetFrameTime();

    cam.offset = (Vector2){ GetRenderWidth()/2, GetRenderHeight()/2, };

    if (IsKeyDown(KEY_N)) cam.zoom *= 1.0 + 2 * delta;
    if (IsKeyDown(KEY_M)) cam.zoom /= 1.0 + 2 * delta;

    if (IsKeyDown(KEY_H)) cam.target.x -= 1000 * delta / cam.zoom;
    if (IsKeyDown(KEY_J)) cam.target.y += 1000 * delta / cam.zoom;
    if (IsKeyDown(KEY_K)) cam.target.y -= 1000 * delta / cam.zoom;
    if (IsKeyDown(KEY_L)) cam.target.x += 1000 * delta / cam.zoom;

    /* ===== Simulation ===== */

    {
      Dice *temp = map.front;
      map.front = map.back;
      map.back = temp;
    }

    for (int i = 0; i < map.w; i++) {
      for (int j = 0; j < map.h; j++) {
        back_at(map, i, j) = front_at(map, i, j);
      }
    }

    /* ===== Rendering ===== */

    BeginDrawing();
    BeginMode2D(cam);
    ClearBackground(WHITE);
    {
      for (int i = 0; i < map.w; i++) {
        for (int j = 0; j < map.h; j++) {
          int face = front_at(map, i, j).faces[0];
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

    usleep(1000000 / 100);
  }

  return 0;
}
