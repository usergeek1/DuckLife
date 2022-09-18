
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_mixer.h>
#include "sdl_wrapper.h"
#include "body.h"
#include "list.h"
#include "scene.h"

typedef struct cursor_pos {
  list_t *bodies;              // bodies involved in colliding
  // collision_handler_t handler; // how to handle collision
  vector_t axis;               // axis on which colliding
  bool currently_colliding;
  void *aux;
} collision_force_info_t;



Mix_Music *coin;
Mix_Music *splash;


const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * The mousepress handler, or NULL if none has been configured.
 */
mouse_handler_t mouse_handler = NULL;

void *pacman_scene = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE;
    case SDL_BUTTON_LEFT:
    return BUTTON_LEFT;
  case SDL_BUTTON_RIGHT:
    return BUTTON_RIGHT;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}

vector_t get_cursor_coordinates() {
    int x, y;
    Uint32 buttons;
    SDL_PumpEvents();  // make sure we have the latest mouse state.
    buttons = SDL_GetMouseState(&x, &y);
    vector_t cursor_coordinates = {x, y};
    return cursor_coordinates;
}

bool sdl_is_done(void *scene) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, pacman_scene);
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      if (mouse_handler == NULL)
        break;
      char mouse = event->button.button;
      if (mouse == '\0')
        break;
      vector_t cursor_coordinates = get_cursor_coordinates();
      mouse_event_type_t mouse_type =
          event->type == SDL_MOUSEBUTTONDOWN ? MOUSE_PRESSED : MOUSE_RELEASED;
      mouse_handler(mouse, cursor_coordinates, mouse_type, pacman_scene);

    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}


void sdl_render_image(char *image_info_path, body_t *bod) {
  int width;
  int height;
  vector_t inputs = {WINDOW_WIDTH/2, WINDOW_HEIGHT/2};
  vector_t centroid_pos = get_window_position(body_get_centroid(bod), inputs);
  //getting undefined reference to `IMG_LoadTexture' & 'IMG_Init'
  //IMG_Init(IMG_INIT_PNG);
  SDL_Texture *image = IMG_LoadTexture(renderer, image_info_path);
  SDL_QueryTexture( image, NULL, NULL, &width, &height);
  SDL_Rect dimensions;
  dimensions.x = centroid_pos.x - width* 0.9/2;
  dimensions.y = centroid_pos.y - height*0.9/2;
  dimensions.w = width * 0.9;
  dimensions.h = height * 0.9;
  SDL_RenderCopy(renderer, image, NULL, &dimensions);

}


void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  // assert(body_count >= 3);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    assert(shape != NULL);
    assert(body != NULL);
    char *image_path = body_get_image_path(body);
    if(image_path != NULL) {
      sdl_render_image(image_path, body);
    }
    // Only draw polygon if image is not being rendered
    else{
      sdl_draw_polygon(shape, body_get_color(body));
    }
    list_free(shape);
    assert(body != NULL);
  }
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

void sdl_on_mouse(mouse_handler_t handler) { mouse_handler = handler; }

void sdl_at_scene(void *scene) { pacman_scene = scene; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}




void play_coin() {
  Mix_PlayMusic(coin, 1);
}

void sound_init() {
  Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048);
  coin = Mix_LoadMUS("assets/coin-drop-1(1).wav");
}

void play_splash() {
  Mix_PlayMusic(splash, 1);
}

void splash_init() {
  Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048);
  splash = Mix_LoadMUS("assets/352105__inspectorj__splash-jumping-e.wav");
}

SDL_Texture *sdl_make_text(char *string, TTF_Font *font, rgb_color_t color) {
  SDL_Color textColor = {color.r * 125, color.g * 125, color.b * 125, 125};
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, string, textColor);
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_FreeSurface(textSurface);
  return textTexture;
}

void sdl_render_text(SDL_Texture *textTexture, vector_t position, vector_t size) {
  SDL_Rect textRect;
  textRect.x = position.x;
  textRect.y = WINDOW_HEIGHT - position.y;
  textRect.w = size.x;
  textRect.h = size.y;

  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
}

/*
SDL_Texture *sdl_make_image(SDL_Surface *image) {
  SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer, image);
  return image_texture;
}

void sdl_put_image_on_body(SDL_Texture *image_texture, body_t *body) {
  list_t *list_of_points = body_get_shape(body);
  SDL_Rect textRect;

  textRect.x = (*(vector_t *)list_get(list_of_points, 3)).x;
  textRect.y = WINDOW_HEIGHT - (*(vector_t *)list_get(list_of_points, 2)).y;
  textRect.w = (*(vector_t *)list_get(list_of_points, 1)).x - (*(vector_t *)list_get(list_of_points, 0)).x;
  textRect.h = (*(vector_t *)list_get(list_of_points, 2)).y - (*(vector_t *)list_get(list_of_points, 1)).y;

  SDL_RenderCopy(renderer, image_texture, NULL, &textRect);
}
 */