#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "list.h"

#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>




#define MAX ((vector_t){.x = 1000.0, .y = 500.0})
// Screen constants
const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};
const vector_t FRAME_CENTER = {500, 250};
const vector_t ZERO_VEC = {0, 0};



// Physics numbers
#define G 6.67E-11          // N m^2 / kg^2
#define M 6E24              // kg
#define g 9.8               // m / s^2
#define R (sqrt(G * M / g))/7.9 // m
#define P 2.8E8      // buoyancy force

#define CIRCLE_POINTS 5

// Random generator constraints / buffers
const double BOTTOM_BUFFER = 30;

const double WIN_TIME_THRESHOLD = 60; // idk man
double time_until_next_obs;

// Duck constants
const size_t DUCK_INDEX = 1;
const vector_t DUCK_JUMP_VEL = {0, 500};
const vector_t DUCK_DIVE_VEL = {0, -500};
const vector_t DUCK_NORMAL_VEL = {300, 0};
const double DUCK_WIDTH = 65.0;
const double DUCK_HEIGHT = 75.0;
const double DUCK_MASS = 40;
const rgb_color_t DUCK_COLOR = (rgb_color_t){1, .83, .36};
const double DUCK_EDGE_BUFFER = 75.0;
const double DUCK_START_ADD = 75;
const vector_t DUCK_MOVE_VEL = {10,0};

// Obstacle Constants
const vector_t OBSTACLE_VEL = {-100, 0};
const double OBSTACLE_MASS = INFINITY;
const double OBSTACLE_DISAPPEAR_BOUND = -70;
const size_t OBS_ID = 7;

const double ICEBERG_W = 90;
const double ICEBERG_H = 140;
const vector_t ICEBERG_STARTING_VEC = {1000 + 100, 220};
const rgb_color_t ICEBERG_COLOR = (rgb_color_t){.906, 0.9375, 0.969};

const double FLOAT_W = 80;
const double FLOAT_H = 60;
const vector_t FLOAT_STARTING_VEC = {1000 + 100, 200};
const rgb_color_t FLOAT_COLOR = (rgb_color_t){.4, 0.3375, 0.969};

const double SHIP_W = 250;
const double SHIP_H = 115;
const vector_t SHIP_STARTING_VEC = {1000 + 100, 250};
const rgb_color_t SHIP_COLOR = (rgb_color_t){.506, 0.6375, 0.369};

//Ocean Definitions
const double OCEAN_WIDTH = 1000;
const double OCEAN_HEIGHT = 250;
const double OCEAN_MASS = INFINITY;
const rgb_color_t OCEAN_COLOR = (rgb_color_t){0, 0.41, .578};

//Wall Definitions
#define WALL_ANGLE atan2(ROW_SPACING, COL_SPACING / 2)
#define WALL_LENGTH hypot(MAX.x / 2, MAX.y)
#define WALL_WIDTH 1.0
#define WALL_COLOR ((rgb_color_t) {1, 1, 1})
#define N_ROWS 11
#define ROW_SPACING 3.6
#define COL_SPACING 3.5

//Text Definitions 
const rgb_color_t TEXT_COLOR = {0, 0, 0};
const vector_t TEXT_SIZE = {40.0, 40.0};

// Coin Definitions
const double COIN_RADIUS = 10.0;
const double COIN_ELASTICITY = 1;
const double COIN_MASS = 40;
const rgb_color_t COIN_COLOR = (rgb_color_t){1, 1, 0};
const double TOTAL_COINS = 10;

// Start Screen constants
const double BUTTON_W = 222;
const double BUTTON_H = 80;
const double BUTTON_Y = 100;

const double EASY_BUTTON_X = 150;
const double EASY_BUTTON_Y = 100;
const double EASY_BUTTON_MASS = INFINITY;
const rgb_color_t EASY_BUTTON_COLOR = (rgb_color_t){1, .83, .36};

const double MEDIUM_BUTTON_X = 500;
const double MEDIUM_BUTTON_Y = 100;
const double MEDIUM_BUTTON_MASS = INFINITY;

const double HARD_BUTTON_X = 850;
const double HARD_BUTTON_Y = 100;
const double HARD_BUTTON_MASS = INFINITY;
const rgb_color_t HARD_BUTTON_COLOR = (rgb_color_t){1, .83, .36};

// Wall constants
const size_t EARTH_INDEX = 6;
const double BOTTOM_WALL_MASS = 100000000;


// Timer vars
const double LOSE_TIME_THRESHOLD = 10000000000; // idk man
const double RAND_TIME_CONST = 0.1;

// Mode constants
const double EASY_SCROLLING_SCREEN = -300;
const double MED_SCROLLING_SCREEN = -350;
const double HARD_SCROLLING_SCREEN = -400;
const double EASY_OBS_TIME = 8;
const double MED_OBS_TIME = 5;
const double HARD_OBS_TIME = 2;


// Elasticity constants
const double ICEBERG_ELASTICITY = 0.05;
const double FLOAT_ELASTICITY = 0.05;
const double SHIP_ELASTICITY = 0.05;

// Text Generation constants
const int FONT_SIZE = 80;
const int DEFAULT_STRING = 50;
const rgb_color_t ORANGE_COLOR = {0.94, 0.43, 0.14};
const SDL_Texture *score_texture;
const vector_t SCORE_POSITION = {20, 500};
const vector_t SCORE_SIZE_VECTOR = {100, 50};

// Generates a random number between 0 and 1
double rand_double(void) { return (double)rand() / RAND_MAX; }

// Enum for type of bodies (duck, obstacles, coin)
typedef enum { 
  DUCK = 0, 
  ICEBERG = 1, 
  FLOAT = 2, 
  SHIP =3,
  COIN = 4,
  OCEAN = 5,
  LEFT_WALL = 6,
  RIGHT_WALL = 7, 
  TOP_WALL = 8,
  BOTTOM_WALL = 9,
  GRAVITY = 10, 
  BACKGROUND = 11,
  EASYBUTTON = 12,
  MEDIUMBUTTON = 13,
  HARDBUTTON = 14
} body_type_t;

// Enum for duck position (i.e., above water, at water level, below water level)
typedef enum {
  UNDERWATER = 0,
  ABOVEWATER = 1,
  ATWATER = 2,
  NODUCK = 3
} duck_pos_t;

// Enum for current scene type
typedef enum scene_type {
  OPENING = 0,
  WIN = 1,
  LOSE = 2,
  GAMEPLAY = 3
} scene_type_t;

// Enum for various off screen
enum off_screen {
  ALL_TOP = 1,
  ALL_BOTTOM = 2,
  ALL_RIGHT = 3,
  ALL_LEFT = 4,
  INSIDE = 0
};

// Enum for game modes
typedef enum game_mode{
  HARD = 1,
  MEDIUM = 2,
  EASY = 3
} game_mode_t;

// State
typedef struct state {
  scene_t *scene;         // Stores current scene
  scene_type_t cur_scene; // Store current scene type

  duck_pos_t
      duck_pos; // Stores the current position of the duck (if duck is visible)
  double exact_duck_pos; // Stores exact position of duck (y coordinate)
  size_t num_coins;      // Stores number of coins collected so far
  double time_elap;      // Stores time elapsed since start of gameplay stage
  double time_of_last_obstacle; // time of last obstacle added
  double time_until_next;
  double num_sec_buffer;
  bool buoyancy_acting;
  bool gravity_acting;
  double scrolling_screen_speed;

} state_t;


// Functions to make/get type info functions for body_type_t's
body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(*info));
  *info = type;
  return info;
}

body_type_t get_type(body_t *body) {
  return *(body_type_t *)body_get_info(body);
}

// Functions to get/set scene_type_t
scene_type_t get_cur_scene(state_t *state) { return state->cur_scene; }

void set_cur_scene(state_t *state, scene_type_t type) {
  state->cur_scene = type;
}

// Functions to get/set duck positions (y coordinate)
duck_pos_t get_duck_pos(state_t *state) { return state->duck_pos; }

void set_duck_pos(state_t *state, double duck_y) {
  state->exact_duck_pos = duck_y;
  if(duck_y > OCEAN_HEIGHT){
    state->duck_pos = ABOVEWATER;
  }
  else if(duck_y == OCEAN_HEIGHT){
    state->duck_pos = ATWATER;
  } else {
    state->duck_pos = UNDERWATER;
  }
}

// Creates a list of points for a circle given a radius
list_t *circle_init(double radius) {
  list_t *circle = list_init(CIRCLE_POINTS, free);
  double arc_angle = 2 * M_PI / CIRCLE_POINTS;
  vector_t point = {.x = radius, .y = 0.0};
  for (size_t i = 0; i < CIRCLE_POINTS; i++) {
    vector_t *v = malloc(sizeof(*v));
    *v = point;
    list_add(circle, v);
    point = vec_rotate(point, arc_angle);
  }
  return circle;
}

// Removes any bodies (obstacles/coins) that are off screen (left)
void remove_bodies_off_screen(state_t *state) {
  scene_t *scene = state->scene;

  // Iterate through all bodies in scene if any of them have an obstacle ID of 2,3,4 remove them
  for(size_t i = 0; i < scene_bodies(scene); i++){
    body_t* cur_body = scene_get_body(scene, i);
    if(get_type(cur_body) == ICEBERG || get_type(cur_body) == FLOAT || get_type(cur_body) == COIN || get_type(cur_body) == SHIP){
      double cur_body_x = ((vector_t)body_get_centroid(cur_body)).x;
      if(cur_body_x < OBSTACLE_DISAPPEAR_BOUND) scene_remove_body(scene,i);
    }
  }

}


// Checks if the duck has been pushed off screen
bool duck_pushed_off_screen(state_t *state) {
  scene_t *scene = state->scene;
  body_t *duck = scene_get_body(scene, DUCK_INDEX);
  double duck_x = ((vector_t)body_get_centroid(duck)).x;

  if (duck_x < -DUCK_WIDTH)
    return true;
  return false;
}

// Checks if a duck body is off screen (specifically touching end of screens)
enum off_screen duck_off_screen(scene_t *scene) {
  body_t *cur_body = scene_get_body(scene, DUCK_INDEX);
  vector_t body_cent = body_get_centroid(cur_body);
  double x_rad = DUCK_WIDTH;
  double y_rad = DUCK_HEIGHT;
  if (body_cent.x - DUCK_EDGE_BUFFER <= FRAME_BOTTOM_LEFT.x) {
    return ALL_LEFT;
  } else if (body_cent.x + x_rad >= FRAME_TOP_RIGHT.x) {
    return ALL_RIGHT;
  } else if (body_cent.y - y_rad <= FRAME_BOTTOM_LEFT.y) {
    return ALL_BOTTOM;
  } else if (body_cent.y + y_rad >= FRAME_TOP_RIGHT.y) {
    return ALL_TOP;
  }
  return INSIDE;
}

// If duck is out of bounds (left), update velocity + centroid
void update_duck_off_left(state_t* state){
    scene_t* scene = state->scene;
    body_t *duck = scene_get_body(scene, DUCK_INDEX);
    double duck_y = ((vector_t)body_get_centroid(duck)).y;
    set_duck_pos(state, duck_y);

    if (duck_off_screen(scene) == ALL_LEFT) {
      body_set_velocity(duck, ZERO_VEC);
      body_set_centroid(duck, (vector_t){DUCK_EDGE_BUFFER, duck_y});
    }
  
}

// Performs actions depending on key press when duck is at water level
void at_water_press(state_t*state, body_t* duck, char key){
  
  switch(key){
    case LEFT_ARROW:
      break;
    case RIGHT_ARROW:
      break;
    case UP_ARROW:
      // Marks the addition of gravity + sets duck to jump velocity
      if(state->gravity_acting == false && state->buoyancy_acting == false){
        body_set_velocity(duck, DUCK_JUMP_VEL);
        state->gravity_acting = true;
      }
      break;
    case DOWN_ARROW:
      // Marks the addition of buoyancy + sets to dive velocity
      play_splash();
      if(state->gravity_acting == false && state->buoyancy_acting == false){
        body_set_velocity(duck, DUCK_DIVE_VEL);
        state->buoyancy_acting = true;
      }
      break;
  }
}


// Generates/adds a duck
void generate_duck(scene_t *scene) {
  list_t *duck_points = rect_init(DUCK_WIDTH, DUCK_HEIGHT);
  char *image_path = malloc(sizeof(char) * 500);
  image_path = ("assets/single_duck.png");
  body_t *duck_body = body_init_with_info_with_image(duck_points, DUCK_MASS, DUCK_COLOR, image_path, (void *)make_type_info(DUCK));
  body_set_centroid(duck_body, (vector_t){.x = FRAME_BOTTOM_LEFT.x + DUCK_EDGE_BUFFER + DUCK_START_ADD, .y = (FRAME_TOP_RIGHT.y/2)});
  scene_add_body(scene, duck_body);
 
}

// Generates ocean cloud background
void generate_ocean_cloud_background(scene_t *scene) {

  list_t *start_screen = rect_init(FRAME_TOP_RIGHT.x, FRAME_TOP_RIGHT.y);
  vector_t translation1 = {FRAME_TOP_RIGHT.x/2, FRAME_TOP_RIGHT.y/2};
  body_translate(start_screen, translation1);  
  char *image_path = malloc(sizeof(char) * DEFAULT_STRING);
  image_path = ("assets/background.png");
  body_t *start_screen_bod = body_init_with_info_with_image(start_screen, DUCK_MASS, DUCK_COLOR, image_path, (void *)make_type_info(BACKGROUND));
  scene_add_body(scene, start_screen_bod);
 
}


// Adds iceberg
void add_iceberg(state_t *state){
  scene_t *scene = state->scene;
  body_t *duck = scene_get_body(scene, DUCK_INDEX);
  list_t *rect_pts = rect_init(ICEBERG_W, ICEBERG_H);
   char *image_path = malloc(sizeof(char) * DEFAULT_STRING);
  image_path = ("assets/iceberg.png");
  body_t *iceberg= body_init_with_info_with_image(rect_pts, INFINITY, DUCK_COLOR, image_path, (void *)make_type_info(BACKGROUND)); 
  body_set_centroid(iceberg, (vector_t)ICEBERG_STARTING_VEC);
  body_set_velocity(iceberg, OBSTACLE_VEL);  
  scene_add_body(scene, iceberg);

  // Collision between duck and iceberg created
  create_physics_collision(scene, ICEBERG_ELASTICITY, duck, iceberg);
}

// Adds the walls to the scene 
void add_walls(scene_t *scene) {
  // Add left wall
  list_t *rect = rect_init(WALL_LENGTH, WALL_WIDTH);
  polygon_translate(rect, (vector_t){.x = WALL_LENGTH / 2, .y = 0});
  polygon_rotate(rect, M_PI / 2, VEC_ZERO);
  body_t *body =
      body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(LEFT_WALL));
  body_set_centroid(body, body_centroid(rect));
  scene_add_body(scene, body);

  // Add right wall
  rect = rect_init(WALL_LENGTH, WALL_WIDTH);
  polygon_translate(rect, (vector_t){.x = WALL_LENGTH / 2, .y = -MAX.x});
  polygon_rotate(rect, M_PI / 2, VEC_ZERO);
  body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(RIGHT_WALL));
  body_set_centroid(body, body_centroid(rect));
  scene_add_body(scene, body);

  // Add ground/bottom wall
  rect = rect_init(MAX.x, WALL_WIDTH);
  body =
      body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(BOTTOM_WALL));
  body_set_centroid(body, (vector_t){.x = MAX.x / 2, .y = WALL_WIDTH / 2});
  scene_add_body(scene, body);

  // Add top wall
  rect = rect_init(MAX.x, WALL_WIDTH);
  polygon_translate(rect, (vector_t){MAX.x / 2, MAX.y});
  body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(TOP_WALL));
  body_set_centroid(body, body_centroid(rect));
  scene_add_body(scene, body);
}


// Adds a gravity body 
void add_gravity_body(scene_t *scene) {
  // Will be offscreen, so shape is irrelevant
  list_t *gravity_ball = rect_init(1, 1);
  body_t *body =
      body_init_with_info(gravity_ball, M, WALL_COLOR, make_type_info(GRAVITY));

  // Move a distnace R below the scene
  vector_t gravity_center = {.x = MAX.x / 2, .y = -R};
  body_set_centroid(body, gravity_center);
  scene_add_body(scene, body);
}

// Removes everything on screen between scenes
scene_t* remove_all(scene_t *scene) {
  size_t number_of_bodies = scene_bodies(scene);
  for (size_t i = 0; i < number_of_bodies; i++) {
    scene_remove_body(scene, i);
  }
  scene_tick(scene,0);
  return scene;
}

// Generates game scene
void generate_game_scene(scene_t *scene) {
  generate_ocean_cloud_background(scene);
  generate_duck(scene); 
  add_walls(scene);
  add_gravity_body(scene);    
}

// Creates state given a scrolling speed and a time buffer for random obstacle generation
state_t* state_init(state_t* state, double scrolling_speed, double sec_buffer){  
  state->time_until_next = rand_double()*RAND_TIME_CONST+1;
  state->num_coins =0;
  state->buoyancy_acting = false;
  state->gravity_acting = false;
  state->scrolling_screen_speed = scrolling_speed;
  state->num_sec_buffer = sec_buffer;
  state->cur_scene = GAMEPLAY;
  return state;
}

// Changes game to gameplay scene
state_t* change_to_game_scene(state_t* state, game_mode_t mode){
  scene_t* scene = state->scene;
  scene = remove_all(scene);
  generate_game_scene(scene);

  // Add buoyancy force on duck
  body_t* duck = scene_get_body(scene, DUCK_INDEX);
  create_buoyancy(scene, G, duck,P, OCEAN_HEIGHT);

  // Add gravity force between duck and earth
  body_t* earth = scene_get_body(scene, EARTH_INDEX);
  create_duck_gravity(scene, G,OCEAN_HEIGHT, duck, earth);

  state->cur_scene = GAMEPLAY;

  switch(mode){
    case HARD:
      return state_init(state, HARD_SCROLLING_SCREEN, HARD_OBS_TIME);
      break;
    case MEDIUM:
      return state_init(state, MED_SCROLLING_SCREEN, MED_OBS_TIME);
      break;
    case EASY:
      return state_init(state, EASY_SCROLLING_SCREEN, EASY_OBS_TIME);
      break;
  }
  return state;
}

// Generates opening scene
void generate_opening_scene(scene_t *scene) {
  sdl_render_scene(scene);

  // Background
  list_t *start_screen = rect_init(FRAME_TOP_RIGHT.x, FRAME_TOP_RIGHT.y);
  vector_t translation1 = {FRAME_TOP_RIGHT.x/2, FRAME_TOP_RIGHT.y/2};
  body_translate(start_screen, translation1);  

  char *image_path = malloc(sizeof(char) * DEFAULT_STRING);
  image_path = ("assets/actualhomescreen.png");
  body_t *start_screen_bod = body_init_with_info_with_image(start_screen, DUCK_MASS, DUCK_COLOR, image_path, (void *)make_type_info(BACKGROUND));
  scene_add_body(scene, start_screen_bod);  

}

// Changes game to opening screen (after loes scene)
state_t* change_to_opening_screen(state_t* state){
  state->cur_scene = OPENING;
  remove_all(state->scene);
  generate_opening_scene(state->scene);
  return state; 
}

// Checks if easy button has been clicked
bool clicked_easy_level_button(vector_t click, state_t *state) {
  double click_x = click.x;
  double click_y = click.y;

  if(click_x > EASY_BUTTON_X - BUTTON_W/2 && click_x < EASY_BUTTON_X + BUTTON_W/2){
    if(click_y > FRAME_TOP_RIGHT.y - EASY_BUTTON_Y - BUTTON_H/2 && click_y < FRAME_TOP_RIGHT.y - EASY_BUTTON_Y + EASY_BUTTON_Y/2){
      return true;
    }
  }
  return false;
}

// Checks if medium button has been clicked
bool clicked_medium_level_button(vector_t click, state_t *state) {
  double click_x = click.x;
  double click_y = click.y;
  if(click_x > MEDIUM_BUTTON_X - BUTTON_W/2 && click_x < MEDIUM_BUTTON_X + BUTTON_W/2){
    if(click_y > FRAME_TOP_RIGHT.y - MEDIUM_BUTTON_Y - BUTTON_H/2 && click_y < FRAME_TOP_RIGHT.y - MEDIUM_BUTTON_Y + MEDIUM_BUTTON_Y/2){
      return true;
    }
  }
  return false;
}

// Checks if hard button has been clicked
bool clicked_hard_level_button(vector_t click, state_t *state) {
  double click_x = click.x;
  double click_y = click.y;
  if(click_x > HARD_BUTTON_X - BUTTON_W/2 && click_x < HARD_BUTTON_X + BUTTON_W/2){
    if(click_y > FRAME_TOP_RIGHT.y - HARD_BUTTON_Y - BUTTON_H/2 && click_y < FRAME_TOP_RIGHT.y - HARD_BUTTON_Y + HARD_BUTTON_Y/2){
      return true;
    }
  }
  return false;
}


// Function called when player is clicking
void onClick(char mouse, vector_t cursor_coordinates, mouse_event_type_t type, void *st) {
  state_t *state = (state_t *)st;
  scene_type_t curr_scene = get_cur_scene(state);
  if (type == MOUSE_PRESSED) {
          if (curr_scene == OPENING) {
              if (clicked_easy_level_button(cursor_coordinates, state)) {
                state = change_to_game_scene(state, (game_mode_t)EASY);             
              }
              if (clicked_medium_level_button(cursor_coordinates, state)) {
                state = change_to_game_scene(state, (game_mode_t)MEDIUM);             
              }
              if (clicked_hard_level_button(cursor_coordinates, state)) {
                state = change_to_game_scene(state, (game_mode_t)HARD);             
              }
          }
        }
  } 


// Function called when player is currently pressing a key (note that void*
// refers to a state now)
void onKey(char key, key_event_type_t type, double held_time, void *st) {
  state_t *state = (state_t *) st;
  scene_t *scene = state->scene;

  if(state->cur_scene == LOSE){
    if(type == KEY_PRESSED && key == SPACE){
      state = change_to_opening_screen(state);
    }
  }

  // If in opening mode, right/left/up arrow keys represent mode slection
  if(state->cur_scene == OPENING){
    if (type == KEY_PRESSED) {
        if(key == LEFT_ARROW){
          state = change_to_game_scene(state, (game_mode_t)EASY);
        }
        else if(key == RIGHT_ARROW){
          state = change_to_game_scene(state, (game_mode_t)HARD);
        }
        else if(key == UP_ARROW){
          state = change_to_game_scene(state, (game_mode_t)MEDIUM);
        }
    }

  }
  
  // Only when in gameplay mode
  if(state->cur_scene == GAMEPLAY){
    body_t *duck = scene_get_body(scene, DUCK_INDEX);
    duck_pos_t duck_pos = state->duck_pos;
    // Key pressed
    if (type == KEY_PRESSED) {
      switch (duck_pos) {
        case UNDERWATER:
          break;
        case ABOVEWATER:
          break;
        case ATWATER:
          at_water_press(state,duck, key);
          break;
        case NODUCK:
          break;
      }
    } 
  }
  
}


// Sets moving screen 
void set_moving_screen(state_t *state) {
  for(size_t i = 7; i < scene_bodies(state->scene); i++) {
    body_set_x_velo(scene_get_body(state->scene, i), state->scrolling_screen_speed);
  }
}


// Add sprites for lose screen
void generate_lose_scene(scene_t *scene) {
  sdl_render_scene(scene);

  // Background
  list_t *start_screen = rect_init(FRAME_TOP_RIGHT.x, FRAME_TOP_RIGHT.y);
  vector_t translation1 = {FRAME_TOP_RIGHT.x/2, FRAME_TOP_RIGHT.y/2};
  body_translate(start_screen, translation1);  

  char *image_path = malloc(sizeof(char) * DEFAULT_STRING);
  image_path = ("assets/gameover.png");
  body_t *end_screen_bod = body_init_with_info_with_image(start_screen, DUCK_MASS, DUCK_COLOR, image_path, (void *)make_type_info(BACKGROUND));
  scene_add_body(scene, end_screen_bod);
}



// Collision handler for hitting coin
void coin_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                            void *aux) {
  body_t *coin = body2;
  state_t *state = (state_t *)aux;
  play_coin();
  // Remove coin + update score
  body_remove(coin);
  state->num_coins++;
}

// random because y position changes
void add_random_coin(state_t *state) {
  scene_t *scene = state->scene;
  body_t *duck = scene_get_body(scene, DUCK_INDEX);
  list_t *coin_points = circle_init(COIN_RADIUS);
  body_t *coin = body_init_with_info(coin_points, COIN_MASS, COIN_COLOR,
                                     (void *)make_type_info(COIN));
  body_set_centroid(coin, (vector_t){.x = FRAME_TOP_RIGHT.x + COIN_RADIUS, .y = rand_double()*0.8*(FRAME_TOP_RIGHT.y) + BOTTOM_BUFFER});
  scene_add_body(scene, coin);

  // Collision between duck and coin added
  create_collision(scene, duck, coin, coin_collision_handler,(void*)state, NULL);

}



// Adds float to scene
void add_float(state_t *state){
  scene_t *scene = state->scene;
  body_t *duck = scene_get_body(scene, DUCK_INDEX);
  list_t *rect_pts = rect_init(FLOAT_W, FLOAT_H);
  char *image_path = malloc(sizeof(char) * DEFAULT_STRING);
  image_path = ("assets/smallfloat.png");
  body_t *floaty = body_init_with_info_with_image(rect_pts, OBSTACLE_MASS, FLOAT_COLOR, image_path,
                                        (void *)make_type_info(FLOAT));
  body_set_centroid(floaty, (vector_t)FLOAT_STARTING_VEC);
  body_set_velocity(floaty, OBSTACLE_VEL);
  scene_add_body(scene, floaty);

  // Collision between duck and float created
  create_physics_collision(scene, FLOAT_ELASTICITY, duck, floaty);
}

// Adds ship to scene
void add_ship(state_t *state){
  scene_t *scene = state->scene;
  body_t *duck = scene_get_body(scene, DUCK_INDEX);
  list_t *rect_pts = rect_init(SHIP_W, SHIP_H);
  char *image_path = malloc(sizeof(char) * DEFAULT_STRING);
  image_path = ("assets/bigship.png");
  body_t *ship = body_init_with_info_with_image(rect_pts, OBSTACLE_MASS, SHIP_COLOR, image_path,
                                        (void *)make_type_info(SHIP));
  body_set_centroid(ship, (vector_t)SHIP_STARTING_VEC);
  body_set_velocity(ship, OBSTACLE_VEL);
  scene_add_body(scene, ship);

  // Collision between duck and ship created
  create_physics_collision(scene, FLOAT_ELASTICITY, duck, ship);
}


// Changes state to lose scene
state_t* change_to_lose_scene(state_t* state){
  state->cur_scene = LOSE;
  remove_all(state->scene);
  generate_lose_scene(state->scene);
  return state;
}

// Creates opening scene
state_t* create_opening(){
  scene_t *opening_scene = scene_init();
  generate_opening_scene(opening_scene);
  

  state_t *state = malloc(sizeof(state_t));
  state->scene = opening_scene;
  state->time_until_next = rand_double()*RAND_TIME_CONST+1;
  state->num_coins =0;
  state->buoyancy_acting = false;
  state->gravity_acting = false;
  state->scrolling_screen_speed = 0;
  state->num_sec_buffer = 0;
  state->cur_scene = OPENING;
  return state;
}

// Generates obstacles / coins randomly depending on mode
void generate_obstacles(state_t *state) {  
  if (false) {
      add_random_coin(state);
  }

  if (state->time_of_last_obstacle > state->time_until_next) {
    double pick_obs = rand_double();

    if (pick_obs < 0.33) {
      add_ship(state);
    } else if (pick_obs < 0.66) {
      add_iceberg(state);
    } else {
      add_float(state);
    } 
    
    add_random_coin(state);

    state->time_of_last_obstacle = 0.0;
    state->time_until_next =
            rand_double()*RAND_TIME_CONST + state->num_sec_buffer;

    // now, need to update position of these randomly added objects to be moving
    // to the left
    set_moving_screen(state);
  }
}




state_t *emscripten_init(void) {
  srand(time(NULL));

  // Initialize scene
  sdl_init(VEC_ZERO, FRAME_TOP_RIGHT);
  sound_init();
  splash_init();

  state_t* state = create_opening();

  // Make score_texture a constant
  sdl_render_scene(state->scene);
  sdl_at_scene(state);
  sdl_on_mouse((mouse_handler_t) onClick);
  sdl_on_key((key_handler_t)onKey);

  return state;
}

// Function to update duck y position during scene tick
void update_duck_ypos(state_t *state){
    scene_t *scene = state->scene;
    body_t *duck = scene_get_body(scene, DUCK_INDEX);
    double duck_y = ((vector_t)body_get_centroid(duck)).y;
    set_duck_pos(state, duck_y);
    
    // If duck is in Gravity + at water level, set y velocity to 0
    if(state->gravity_acting ){
      if(duck_y < OCEAN_HEIGHT){
        state->gravity_acting = false;
        vector_t cur_centroid = body_get_centroid(duck);
        body_set_centroid(duck, (vector_t){cur_centroid.x, OCEAN_HEIGHT});
        set_duck_pos(state, OCEAN_HEIGHT);
        body_set_velocity(duck, ZERO_VEC);
      }
    }
    // If duck is in Buoyancy + at water level, set y velocity to 0
    if(state->buoyancy_acting ){
      if(duck_y > OCEAN_HEIGHT){
        state->buoyancy_acting = false;
        vector_t cur_centroid = body_get_centroid(duck);
        body_set_centroid(duck, (vector_t){cur_centroid.x, OCEAN_HEIGHT});
        set_duck_pos(state, OCEAN_HEIGHT);
        body_set_velocity(duck, ZERO_VEC);
      
      }
    }
}

// Generates text for gameplay (score, timer)
void generate_gameplay_text(state_t *state){
  TTF_Font *score_font = TTF_OpenFont("assets/verdana.ttf",
        FONT_SIZE);
    char *score_string = malloc(DEFAULT_STRING * sizeof(char));
    sprintf(score_string, "Score %zu", state->num_coins);
    SDL_Texture *score_tex = sdl_make_text(score_string, score_font, ORANGE_COLOR);
    sdl_render_text(score_tex, SCORE_POSITION, SCORE_SIZE_VECTOR);

    TTF_Font *timer_font = TTF_OpenFont("assets/verdana.ttf",
        FONT_SIZE);
    char *timer_string = malloc(DEFAULT_STRING * sizeof(char));


    sprintf(timer_string, "Time %f", state->time_elap);
    SDL_Texture *timer_tex = sdl_make_text(timer_string, timer_font, ORANGE_COLOR);
    vector_t new_pos = {20, 450};
    vector_t new_size_vec = {190, SCORE_SIZE_VECTOR.y};
    sdl_render_text(timer_tex, new_pos, new_size_vec);
    TTF_CloseFont(score_font); 
    TTF_CloseFont(timer_font); 
}

// Generates text for lose screen (score)
void generate_lose_text(state_t *state){
    TTF_Font *score_font = TTF_OpenFont("assets/ARCADECLASSIC.TTF",
    FONT_SIZE);
    char *score_string = malloc(DEFAULT_STRING * sizeof(char));
    sprintf(score_string, "Score %zu", state->num_coins);
    vector_t new_pos = {869, 500};
    SDL_Texture *score_tex = sdl_make_text(score_string, score_font, ORANGE_COLOR);
    sdl_render_text(score_tex, new_pos, SCORE_SIZE_VECTOR);
    TTF_CloseFont(score_font); 
    state->time_elap = 0.0;
}

void emscripten_main(state_t *state){ 
  double dt = time_since_last_tick();

  scene_t *scene = state->scene;
  scene_type_t scene_type = state->cur_scene;

  // If state is in gameplay mode
  if (scene_type == GAMEPLAY) {
    
  // Generates obstacles + coin
   generate_obstacles(state);
   
   // Removes bodies (obstacles/coins) that are off screen from the left 
   remove_bodies_off_screen(state);
  
    // Constantly set/update duck water position, velocity, bounds (as needed)
    state->time_elap += dt;
    state->time_of_last_obstacle += dt;

    // Updates duck y position
    update_duck_ypos(state);
    
    // Move to lose screen if duck pushed off screen
    if(duck_pushed_off_screen(state)){
        state = change_to_lose_scene(state);
    }

  }
  
  
  // Scene tick + rendering
  scene_tick(scene, dt);
  sdl_render_scene(scene);
  
  // Text implementation depending on gameplay/lose modes
  if(state->cur_scene == GAMEPLAY) {
    generate_gameplay_text(state);
  } else if (state->cur_scene == LOSE) {
    generate_lose_text(state);
  }
  sdl_show();


}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}




