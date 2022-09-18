#include "collision.h"
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
const vector_t ZERO_VEC = {0, 0};

// Enemy constants
const double ENEMY_X_RADIUS = 25;
const double ENEMY_Y_RADIUS = 12;
const size_t NUM_ENEMY_POINTS = 80;
const double EMPTY_PERCENTAGE = 0.25;
const double ENEMY_MASS = 30;
const rgb_color_t ENEMY_COLOR = {1, .682, .682};
const double ENEMY_NEW_POS_FROM_EDGE = 1;

const double NUM_ENEMIES = 10;
const double ENEMY_STARTING_X = 0;
const double ENEMY_STARTING_Y = 480;
const double ENEMY_END_SPACE_BUFFER = 10;
const double ENEMY_ROW_SPACE_BUFFER = 40;
const vector_t ENEMY_STARTING_VEL = (vector_t){30, 0};
const double NUM_ENEMY_ROWS = 2;

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
const double BULLET_X_RAD = 3;
const double BULLET_Y_RAD = 5;
const double BULLET_VELOCITY = 300;
const rgb_color_t BULLET_COLOR = {1, .5, .5};
const double BULLET_MASS = 15;
enum off_screen {
  ALL_TOP = 1,
  ALL_BOTTOM = 2,
  ALL_RIGHT = 3,
  ALL_LEFT = 4,
  INSIDE = 0
};
enum body_type { PLAYER = 0, ENEMY = 1, PLAYER_BULLET = 2, ENEMY_BULLET = 3 };

// Time variable
static clock_t prev_time = 0;
const int TIME_PER_SHOOT = 3;

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

  // Left / Right Straight parts
  for (size_t i = 0; i <= NUM_ENEMY_POINTS / 4; i++) {
    double y_value = i * tan(first_mouth_bound * 2 * M_PI / NUM_ENEMY_POINTS);
    vector_t pac = {center_x + i, center_y + y_value - ENEMY_X_RADIUS};
    vector_t *pac_p = malloc(sizeof(vector_t));
    *pac_p = pac;
    list_add(enemy_points, pac_p);
  }

  for (size_t i = 0; i <= NUM_ENEMY_POINTS / 2; i++) {
    double x_point =
        center_x + ENEMY_X_RADIUS * cos(i * 2 * M_PI / NUM_ENEMY_POINTS);
    double y_point =
        center_y + ENEMY_Y_RADIUS * sin(i * 2 * M_PI / NUM_ENEMY_POINTS);
    vector_t *pac_p = malloc(sizeof(vector_t));
    vector_t pac = {x_point, y_point};
    *pac_p = pac;
    list_add(enemy_points, pac_p);
  }
  return enemy_points;
}

list_t *get_bullet_points(double center_x, double center_y) {
  list_t *bullet_points = list_init(4, (free_func_t)free);
  vector_t *point_1 = malloc(sizeof(vector_t));
  point_1->x = center_x - BULLET_X_RAD;
  point_1->y = center_y - BULLET_Y_RAD;
  vector_t *point_2 = malloc(sizeof(vector_t));
  point_2->x = center_x - BULLET_X_RAD;
  point_2->y = center_y + BULLET_Y_RAD;
  vector_t *point_3 = malloc(sizeof(vector_t));
  point_3->x = center_x + BULLET_X_RAD;
  point_3->y = center_y + BULLET_Y_RAD;
  vector_t *point_4 = malloc(sizeof(vector_t));
  point_4->x = center_x + BULLET_X_RAD;
  point_4->y = center_y - BULLET_Y_RAD;
  list_add(bullet_points, point_4);
  list_add(bullet_points, point_3);
  list_add(bullet_points, point_2);
  list_add(bullet_points, point_1);
  return bullet_points;
}

enum off_screen body_offscreen(scene_t *scene, int ind) {
  body_t *cur_body = scene_get_body(scene, ind);
  vector_t body_cent = body_get_centroid(cur_body);
  double x_rad = ENEMY_X_RADIUS;
  double y_rad = ENEMY_Y_RADIUS;
  if (body_get_info(cur_body) == (void *)PLAYER) {
    x_rad = PLAYER_X_RAD;
    y_rad = PLAYER_Y_RAD;
  }
  if (body_get_info(cur_body) == (void *)PLAYER_BULLET ||
      body_get_info(cur_body) == (void *)ENEMY_BULLET) {
    x_rad = BULLET_X_RAD;
    y_rad = BULLET_Y_RAD;
  }
  if (body_cent.x - x_rad < FRAME_BOTTOM_LEFT.x) {
    return ALL_LEFT;
  } else if (body_cent.x + x_rad > FRAME_TOP_RIGHT.x) {
    return ALL_RIGHT;
  } else if (body_cent.y - y_rad <= FRAME_BOTTOM_LEFT.y) {
    return ALL_BOTTOM;
  } else if (body_cent.y + y_rad >= FRAME_TOP_RIGHT.y) {
    return ALL_TOP;
  }

  return INSIDE;
}

void move_player(scene_t *scene, body_t *player, char key) {

  if (key == LEFT_ARROW && body_offscreen(scene, 0) != ALL_LEFT) {
    body_set_velocity(player, P_MOVING_VEL_L);
  } else if (key == RIGHT_ARROW && body_offscreen(scene, 0) != ALL_RIGHT) {
    body_set_velocity(player, P_MOVING_VEL_R);
  } else if (body_offscreen(scene, 0) == ALL_LEFT) {
    body_set_velocity(player, ZERO_VEC);
    body_set_centroid(player, (vector_t){PLAYER_X_RAD, PLAYER_STARTING_Y});
  } else if (body_offscreen(scene, 0) == ALL_RIGHT) {
    body_set_velocity(player, ZERO_VEC);
    body_set_centroid(player, (vector_t){FRAME_TOP_RIGHT.x - PLAYER_X_RAD,
                                         PLAYER_STARTING_Y});
  } else
    body_set_velocity(player, ZERO_VEC);
}

void shoot_bullet(scene_t *scene, int ind) {
  // Player body
  body_t *body = scene_get_body(scene, ind);
  // X position of bullet
  double x_bullet = body_get_centroid(body).x;
  // initial Y position of bullet
  double y_pos = body_get_centroid(body).y + PLAYER_Y_RAD;
  list_t *bullet_points = get_bullet_points(x_bullet, y_pos);

  body_t *bullet_bod;
  if (body_get_info(body) == (void *)PLAYER) {
    bullet_bod = body_init_with_info(bullet_points, BULLET_MASS, BULLET_COLOR,
                                     (void *)PLAYER_BULLET);
    body_set_centroid(bullet_bod, body_centroid(bullet_points));
    body_set_velocity(bullet_bod, (vector_t){0, BULLET_VELOCITY});
  } else {
    bullet_bod = body_init_with_info(bullet_points, BULLET_MASS, BULLET_COLOR,
                                     (void *)ENEMY_BULLET);
    body_set_centroid(bullet_bod, body_centroid(bullet_points));
    body_set_velocity(bullet_bod, (vector_t){0, -BULLET_VELOCITY});
  }
  scene_add_body(scene, bullet_bod);
}

void onKey(char key, key_event_type_t type, double held_time, void *sc) {
  scene_t *scene = (scene_t *)sc;
  assert(scene != NULL);

  body_t *player = scene_get_body(scene, 0);
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      move_player(scene, player, key);
      break;
    case RIGHT_ARROW:
      move_player(scene, player, key);
      break;
    case SPACE:
      shoot_bullet(scene, 0);
    }
  } else {
    // sets velo to 0
    body_set_velocity(player, ZERO_VEC);
  }
}

scene_t *emscripten_init() {
  srand(time(NULL));
  scene_t *scene = scene_init();
  sdl_init(FRAME_BOTTOM_LEFT, FRAME_TOP_RIGHT);

  // Adds player to center of screen
  list_t *player_pts = get_player_points(PLAYER_STARTING_X, PLAYER_STARTING_Y);
  body_t *player = body_init_with_info(player_pts, PLAYER_MASS, PLAYER_COLOR,
                                       (void *)PLAYER);
  body_set_centroid(player, (vector_t)body_centroid(player_pts));
  scene_add_body(scene, player);

  // First set of enemies
  for (size_t j = 0; j < NUM_ENEMY_ROWS; j++) {
    // Changing y for each row
    double new_enemy_y = ENEMY_STARTING_Y - (j)*ENEMY_ROW_SPACE_BUFFER;
    double enem_dist = FRAME_TOP_RIGHT.x / NUM_ENEMIES - ENEMY_END_SPACE_BUFFER;
    double new_enemy_x = ENEMY_STARTING_X + enem_dist / 2;
    for (size_t i = 0; i < NUM_ENEMIES; i++) {
      // Row of enemies
      list_t *enemy_pts = get_enemy_points(new_enemy_x, new_enemy_y);
      // Changing x for each enemy in a row
      new_enemy_x += enem_dist;
      body_t *enemy =
          body_init_with_info(enemy_pts, ENEMY_MASS, ENEMY_COLOR, (void *)1);
      body_set_centroid(enemy, (vector_t)body_centroid(enemy_pts));
      body_set_velocity(enemy, ENEMY_STARTING_VEL);
      scene_add_body(scene, enemy);
    }
  }
  // Second set of enemies (moving opposite direction)
  for (size_t j = 0; j < NUM_ENEMY_ROWS; j++) {
    // Changing y for each row
    double new_enemy_y =
        ENEMY_STARTING_Y - (j + NUM_ENEMY_ROWS) * ENEMY_ROW_SPACE_BUFFER;
    double enem_dist = FRAME_TOP_RIGHT.x / NUM_ENEMIES - ENEMY_END_SPACE_BUFFER;
    double new_enemy_x = FRAME_TOP_RIGHT.x - ENEMY_STARTING_X - enem_dist / 2;
    for (size_t i = 0; i < NUM_ENEMIES; i++) {
      // Row of enemies
      list_t *enemy_pts = get_enemy_points(new_enemy_x, new_enemy_y);
      // Changing x for each enemy in a row
      new_enemy_x -= enem_dist;
      body_t *enemy =
          body_init_with_info(enemy_pts, ENEMY_MASS, ENEMY_COLOR, (void *)1);
      body_set_centroid(enemy, (vector_t)body_centroid(enemy_pts));
      body_set_velocity(enemy, vec_negate(ENEMY_STARTING_VEL));
      scene_add_body(scene, enemy);
    }
  }

  // test for bullet

  assert(scene_bodies(scene) >= 2);
  sdl_render_scene(scene);
  sdl_at_scene(scene);
  sdl_on_key((key_handler_t)onKey);

  return scene;
}

int get_num_enemies(scene_t *scene) {
  int counter = 0;
  for (size_t i = 0; i < scene_bodies(scene); i++) {

    body_t *cur_enemy_body = scene_get_body(scene, i);
    if (body_get_info(cur_enemy_body) == (void *)ENEMY) {
      counter++;
    }
  }
  return counter;
}

bool check_shoot_bullet() {
  clock_t cur_time = clock();
  if ((cur_time - prev_time) / CLOCKS_PER_SEC == TIME_PER_SHOOT) {
    prev_time = cur_time;
    return true;
  }
  return false;
}
void check_player_bullet_collision(scene_t *scene) {
  // Check for collision between all active player bullets (2) and all active
  // enemies (1)
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *cur_body = scene_get_body(scene, i);
    // Bullet, iterate through all bodies to check collision
    if (body_get_info(cur_body) == (void *)PLAYER_BULLET) {
      for (size_t j = 0; j < scene_bodies(scene); j++) {
        body_t *cur_body2 = scene_get_body(scene, j);
        // If collision between player bullet and enemy, remove btoh
        if (body_get_info(cur_body2) == (void *)ENEMY && i != j &&
            ((collision_info_t)find_collision(body_get_shape(cur_body),
                                              body_get_shape(cur_body2)))
                    .collided == true) {
          body_remove(cur_body);
          body_remove(cur_body2);
          return;
        }
      }
    }
  }
}

void check_enemy_bullet_collision(scene_t *scene) {
  // Check for collision between all active enemy bullets (3) and player (0)
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *cur_body = scene_get_body(scene, i);
    // Bullet, iterate through all bodies to check collision
    if (body_get_info(cur_body) == (void *)ENEMY_BULLET) {
      body_t *cur_body2 = scene_get_body(scene, 0);
      // If collision between enemy bullet and player, remove btoh
      if (body_get_info(cur_body2) == (void *)PLAYER &&
          (((collision_info_t)find_collision(body_get_shape(cur_body),
                                             body_get_shape(cur_body2)))
               .collided == true)) {
        body_remove(cur_body);
        body_remove(cur_body2);
        exit(0);
      }
    }
  }
}

void update_enemy_pos(scene_t *scene) {
  // Check if enemies are out of bounds, if so shift down / exit if all bottom
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *cur_enemy_body = scene_get_body(scene, i);
    if (body_get_info(cur_enemy_body) == (void *)ENEMY) {
      // If at bottom
      if (body_offscreen(scene, i) == ALL_BOTTOM) {
        exit(0);
      }
      // If at right
      if (body_offscreen(scene, i) == ALL_RIGHT) {
        vector_t new_pos =
            vec_add(body_get_centroid(cur_enemy_body),
                    (vector_t){-ENEMY_NEW_POS_FROM_EDGE,
                               -NUM_ENEMY_ROWS * ENEMY_ROW_SPACE_BUFFER});
        body_set_centroid(cur_enemy_body, new_pos);

        body_set_velocity(cur_enemy_body,
                          vec_negate(body_get_velocity(cur_enemy_body)));
      }
      // If at left
      else if (body_offscreen(scene, i) == ALL_LEFT) {
        vector_t new_pos =
            vec_add(body_get_centroid(cur_enemy_body),
                    (vector_t){ENEMY_NEW_POS_FROM_EDGE,
                               -NUM_ENEMY_ROWS * ENEMY_ROW_SPACE_BUFFER});
        body_set_centroid(cur_enemy_body, new_pos);

        body_set_velocity(cur_enemy_body,
                          vec_negate(body_get_velocity(cur_enemy_body)));
      }
    }
    // Remove bullets if out of bounds
    if (body_get_info(cur_enemy_body) == (void *)PLAYER_BULLET) {
      if (body_offscreen(scene, i) == ALL_TOP) {
        body_remove(cur_enemy_body);
      }
    } else if (body_get_info(cur_enemy_body) == (void *)ENEMY_BULLET) {

      if (body_offscreen(scene, i) == ALL_BOTTOM) {
        body_remove(cur_enemy_body);
      }
    }
  }
}

void emscripten_main(scene_t *scene) {
  assert(scene != NULL);
  sdl_render_scene(scene);

  // If num of enemies is 0, done with game
  if (get_num_enemies(scene) == 0) {
    exit(0);
  }

  // If player not there, done with game
  if (body_get_info(scene_get_body(scene, 0)) != (void *)PLAYER) {
    exit(0);
  }

  update_enemy_pos(scene);
  check_player_bullet_collision(scene);
  scene_tick(scene, time_since_last_tick());

  // Randomly choose an enemy to shoot a bullet
  // Pick random number from 1 to get_num_enemies(scene)
  if (check_shoot_bullet()) {
    int r = rand() % get_num_enemies(scene) + 1;
    shoot_bullet(scene, r);
  }

  check_enemy_bullet_collision(scene);
  scene_tick(scene, time_since_last_tick());
}

void emscripten_free(scene_t *scene) { scene_free(scene); }
