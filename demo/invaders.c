#include "sdl_wrapper.h"
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define M_PI 3.14159265358979323846 /* pi */

// Screen constants
const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};
const vector_t FRAME_CENTER = {500, 250};

// Enemy constants
const double ENEMY_X_RADIUS = 25;
const double ENEMY_Y_RADIUS = 35;
const size_t NUM_ENEMY_POINTS = 80;
const double EMPTY_PERCENTAGE = 0.66;
const double ENEMY_MASS = 30;
const rgb_color_t ENEMY_COLOR = {1, .682, .682};

const double NUM_ENEMIES = 4;
const double ENEMY_STARTING_X = 450;
const double ENEMY_STARTING_Y = 900;
const double ENEMY_SPACE_BUFFER = 20;

// Player constants
const double PLAYER_X_RAD = 40;
const double PLAYER_Y_RAD = 20;
const double PLAYER_STARTING_X = 500;
const double PLAYER_STARTING_Y = 60;
const double PLAYER_EDGE_BUFFER = 20;
const double PLAYER_MASS = 30;
const size_t NUM_PLAYER_POINTS = 80;
const rgb_color_t PLAYER_COLOR = {1, .682, .259};
const vector_t P_MOVING_VEL_R = {150, 0};
const vector_t P_MOVING_VEL_L = {-150, 0};

// Bullet constants
const double BULLET_WIDTH = 10;
const double BULLET_HEIGHT = 30;
const double NUM_BULLET_POINTS = 40;
const rgb_color_t BULLET_COLOR_1 = {1, .4, .4};
const rgb_color_t BULLET_COLOR_1 = {1, .2, .3};

enum off_screen {
  ALL_TOP = 1,
  ALL_BOTTOM = 2,
  ALL_RIGHT = 3,
  ALL_LEFT = 4,
  INSIDE = 0
};

list_t *get_player_points(double center_x, double center_y) {
  list_t *player_points = list_init(NUM_PLAYER_POINTS, (free_func_t)free);

  // Add points for circular part of enemy (from 45 to 315)
  for (size_t i = 0; i < NUM_PLAYER_POINTS; i++) {
    double x_point =
        center_x + PLAYER_X_RAD * cos(i * 2 * M_PI / NUM_PLAYER_POINTS);
    double y_point =
        center_y + PLAYER_Y_RAD * sin(i * 2 * M_PI / NUM_PLAYER_POINTS);

    vector_t *pt_p = malloc(sizeof(vector_t));
    vector_t pt = {x_point, y_point};
    *pt_p = pt;
    list_add(player_points, pt_p);
  }

  return player_points;
}

list_t *get_enemy_points(double center_x, double center_y) {
  list_t *enemy_points = list_init(NUM_ENEMY_POINTS, (free_func_t)free);

  int first_mouth_bound = NUM_ENEMY_POINTS * EMPTY_PERCENTAGE / 2;
  int second_mouth_bound =
      NUM_ENEMY_POINTS - NUM_ENEMY_POINTS * EMPTY_PERCENTAGE / 2;

  // Add points for right flat edge
  double right_x_bound = cos(first_mouth_bound * 2 * M_PI / NUM_ENEMY_POINTS);
  for (size_t i = 0; i < right_x_bound; i++) {
    double y_value = i * tan(first_mouth_bound * 2 * M_PI / NUM_ENEMY_POINTS);
    vector_t pac = {center_x + i, center_y + y_value};
    vector_t *pac_p = malloc(sizeof(vector_t));
    *pac_p = pac;
    list_add(enemy_points, pac_p);
  }
  // Add points for circular part of enemy
  for (size_t i = first_mouth_bound; i < second_mouth_bound; i++) {
    double x_point =
        center_x + ENEMY_X_RADIUS * cos(i * 2 * M_PI / NUM_ENEMY_POINTS);
    double y_point =
        center_y + ENEMY_Y_RADIUS * sin(i * 2 * M_PI / NUM_ENEMY_POINTS);
    vector_t *pac_p = malloc(sizeof(vector_t));
    vector_t pac = {x_point, y_point};
    *pac_p = pac;
    list_add(enemy_points, pac_p);
  }

  // Add points for left flat edge
  for (size_t i = 0; i < right_x_bound; i++) {
    double y_value = -i * tan(first_mouth_bound * 2 * M_PI / NUM_ENEMY_POINTS);
    vector_t pac = {center_x + i, center_y + y_value};
    vector_t *pac_p = malloc(sizeof(vector_t));
    *pac_p = pac;
    list_add(enemy_points, pac_p);
  }

  return enemy_points;
}

list_t *get_bullet_points(double center_x, double center_y) {
  list_t *bullet_points = list_init(NUM_BULLET_POINTS, (free_func_t)free);

  int num_pts_short =
      NUM_BULLET_POINTS * (BULLET_WIDTH / (BULLET_HEIGHT + BULLET_WIDTH)) / 2;
  int num_pts_long =
      NUM_BULLET_POINTS * (BULLET_HEIGHT / (BULLET_HEIGHT + BULLET_WIDTH)) / 2;

  int left_edge_x = center_x - BULLET_WIDTH / 2;
  int right_edge_x = center_x + BULLET_WIDTH / 2;
  int top_edge_y = center_y + BULLET_HEIGHT / 2;
  int bottom_edge_y = center_y - BULLET_HEIGHT / 2;

  int x_gap = BULLET_WIDTH / num_pts_short;
  int y_gap = BULLET_HEIGHT / num_pts_long;

  int counter = 0;

  for (int i = 0; i < NUM_BULLET_POINTS; i++) {
    if (i < num_pts_short) { // top edge
      vector_t pac = {left_edge_x + i * x_gap, top_edge_y};
      vector_t *pac_p = malloc(sizeof(vector_t));
      *pac_p = pac;
      list_add(bullet_points, pac_p);
    } else if (i < num_pts_short + num_pts_long) { // left edge
      vector_t pac = {left_edge_x, top_edge_y - counter * y_gap};
      vector_t *pac_p = malloc(sizeof(vector_t));
      *pac_p = pac;
      list_add(bullet_points, pac_p);
      counter++;
      if (i = num_pts_short + num_pts_long - 1)
        counter = 0;
    } else if (i < 2 * num_pts_short + num_pts_long) { // bottom edge
      vector_t pac = {left_edge_x + counter * x_gap, bottom_edge_y};
      vector_t *pac_p = malloc(sizeof(vector_t));
      *pac_p = pac;
      list_add(bullet_points, pac_p);
      counter++;
      if (i = 2 * num_pts_short + num_pts_long - 1)
        counter = 0;
    } else { // right edge
      vector_t pac = {right_edge_x, bottom_edge_y + counter * y_gap};
      vector_t *pac_p = malloc(sizeof(vector_t));
      *pac_p = pac;
      list_add(bullet_points, pac_p);
      counter++;
    }
  }

  return bullet_points;
}

void move_player(body_t *player, char key) {

  if (key == LEFT_ARROW) {
    body_set_velocity(player, P_MOVING_VEL_L);
  } else if (key == RIGHT_ARROW) {
    body_set_velocity(player, P_MOVING_VEL_R);
  }
}

// needs to have acces to scene one way or another, can pass in through onkey if
// //youd like
void shoot_bullet(body_t *player) {}

void onKey(char key, key_event_type_t type, double held_time, void *sc) {
  scene_t *scene = (scene_t *)sc;
  assert(scene != NULL);

  body_t *player = scene_get_body(scene, 0);
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      move_player(player, key);
      break;
    case RIGHT_ARROW:
      move_player(player, key);
      break;
    case SPACE:
      shoot_bullet(player);
      break;
    }
  }
}

bool is_player_offscreen(scene_t *scene) {
  body_t *player = scene_get_body(scene, 0);
  vector_t player_cent = body_get_centroid(player);

  if (player_cent.x - PLAYER_X_RAD - PLAYER_EDGE_BUFFER < 0) {
    return true;
  } else if (player_cent.x + PLAYER_X_RAD + PLAYER_EDGE_BUFFER < 1000) {
    return true;
  }
  return false;
}

scene_t *emscripten_init() {
  scene_t *scene = scene_init();
  sdl_init(FRAME_BOTTOM_LEFT, FRAME_TOP_RIGHT);

  // adds player to center of screen
  list_t *player_pts = get_player_points(PLAYER_STARTING_X, PLAYER_STARTING_Y);
  body_t *player = body_init(player_pts, PLAYER_MASS, PLAYER_COLOR);
  body_set_centroid(player, (vector_t)body_centroid(player_pts));
  scene_add_body(scene, player);

  // adds player to center of screen
  int enem_per_line =
      FRAME_TOP_RIGHT.x / (ENEMY_X_RADIUS * 2 + ENEMY_SPACE_BUFFER);
  int new_enemy_y = ENEMY_STARTING_Y;
  int new_enemy_x = ENEMY_STARTING_X;

  for (size_t i = 0; i < NUM_ENEMIES; i++) {
    list_t *enemy_pts = get_player_points(new_enemy_x, new_enemy_y);

    new_enemy_x -= 2 * i * ENEMY_X_RADIUS + ENEMY_SPACE_BUFFER;
    if (enem_per_line-- == 0) {
      new_enemy_y -= ENEMY_Y_RADIUS * 2 + ENEMY_SPACE_BUFFER;
      new_enemy_x = ENEMY_STARTING_X;
      enem_per_line =
          FRAME_TOP_RIGHT.x / (ENEMY_X_RADIUS * 2 + ENEMY_SPACE_BUFFER);
    }

    body_t *enemy = body_init(enemy_pts, ENEMY_MASS, ENEMY_COLOR);
    body_set_centroid(enemy, (vector_t)body_centroid(enemy_pts));
    scene_add_body(scene, enemy);
  }

  assert(scene_bodies(scene) >= 10);
  sdl_render_scene(scene);
  sdl_at_scene(scene);
  sdl_on_key((key_handler_t)onKey);

  return scene;
}

void emscripten_main(scene_t *scene) {
  assert(scene != NULL);
  sdl_render_scene(scene);
  scene_tick(scene, time_since_last_tick());
}

void emscripten_free(scene_t *scene) { scene_free(scene); }