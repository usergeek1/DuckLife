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

// Pellet constants
const double PELLET_RADIUS = 5;
const size_t NUM_PELLETS = 10;
const size_t PELLET_POINTS = 5;
const double PELLET_MASS = 3;
const rgb_color_t PELLET_COLOR = {.6275, .792, .8472};

// Pacman constants
const double PACMAN_RADIUS = 40;
const size_t PACMAN_POINTS = 100;
const double MOUTH_PERCENTAGE = 0.25;
const double PACMAN_MASS = 10;
const rgb_color_t PACMAN_COLOR = {1, .682, .259};
const double PACMAN_STARTING_X = 500;
const double PACMAN_STARTING_Y = 250;
const double PACMAN_ACCELERATION = 30;

/**
 * @brief Returns a list of points for a pellet
 *
 * @param center_x - x coordinate of center of pellet
 * @param center_y - y coordinate of center of pellet
 * @return list_t* - list of points for the pellet
 */

enum off_screen {
  ALL_TOP = 1,
  ALL_BOTTOM = 2,
  ALL_RIGHT = 3,
  ALL_LEFT = 4,
  INSIDE = 0
};

/**
 * @brief Returns a list of points for a pellet object
 *
 * @param center_x - center of pellet (x)
 * @param center_y  - center of pellet (y)
 * @return list_t*
 */
list_t *get_pellet_points(double center_x, double center_y) {
  list_t *pellet_points = list_init(PELLET_POINTS, (free_func_t)free);

  for (size_t i = 0; i < PELLET_POINTS; i++) {
    double x_point =
        center_x + PELLET_RADIUS * cos(i * 2 * M_PI / PELLET_POINTS);
    double y_point =
        center_y + PELLET_RADIUS * sin(i * 2 * M_PI / PELLET_POINTS);
    vector_t *pel_p = malloc(sizeof(vector_t));
    vector_t pel = {x_point, y_point};
    *pel_p = pel;
    list_add(pellet_points, pel_p);
  }
  return pellet_points;
}

/**
 * @brief Returns a list of points for the pacman
 *
 * @param center_x - x coordinate of center of pacman
 * @param center_y - y coordinate of center of pacman
 * @return list_t*
 */
list_t *get_pacman_points(double center_x, double center_y) {
  list_t *pacman_points = list_init(PACMAN_POINTS, (free_func_t)free);

  int first_mouth_bound = PACMAN_POINTS * MOUTH_PERCENTAGE / 2;
  int second_mouth_bound = PACMAN_POINTS - PACMAN_POINTS * MOUTH_PERCENTAGE / 2;

  // Add points for first part of mouth of pacman (upper half) - need to alter y
  // coordinate based on angle
  double right_x_bound = cos(first_mouth_bound * 2 * M_PI / PACMAN_POINTS);
  for (size_t i = 0; i < right_x_bound; i++) {
    double y_value = i * tan(first_mouth_bound * 2 * M_PI / PACMAN_POINTS);
    vector_t pac = {center_x + i, center_y + y_value};
    vector_t *pac_p = malloc(sizeof(vector_t));
    *pac_p = pac;
    list_add(pacman_points, pac_p);
  }
  // Add points for circular part of pacman (from 45 to 315)
  for (size_t i = first_mouth_bound; i < second_mouth_bound; i++) {
    double x_point =
        center_x + PACMAN_RADIUS * cos(i * 2 * M_PI / PACMAN_POINTS);
    double y_point =
        center_y + PACMAN_RADIUS * sin(i * 2 * M_PI / PACMAN_POINTS);
    vector_t *pac_p = malloc(sizeof(vector_t));
    vector_t pac = {x_point, y_point};
    *pac_p = pac;
    list_add(pacman_points, pac_p);
  }

  // Add points for second part of mouth of pacman (from 315 (inclusive) to 360
  // (exclusive)) - need to alternate y coordinate based on angle
  for (size_t i = 0; i < right_x_bound; i++) {
    double y_value = -i * tan(first_mouth_bound * 2 * M_PI / PACMAN_POINTS);
    vector_t pac = {center_x + i, center_y + y_value};
    vector_t *pac_p = malloc(sizeof(vector_t));
    *pac_p = pac;
    list_add(pacman_points, pac_p);
  }
  return pacman_points;
}

/**
 * @brief Given a pellet and pacman, determine whether the
 * pellet is in the polgyon
 *
 * @return true - the pellet is in pacman
 * @return false - the pellet is not in pacman
 */
bool is_inside_pacman(body_t *pacman, body_t *pellet) {
  vector_t pac_centroid = body_get_centroid(pacman);
  vector_t pel_centroid = body_get_centroid(pellet);

  if (vec_distance(pac_centroid, pel_centroid) <= PACMAN_RADIUS) {
    return true;
  }
  return false;
}

/**
 * @brief Check if point is inside screen
 *
 * @param point
 * @return true
 * @return false
 */
bool is_inside_screen(vector_t point) {
  if (point.x >= FRAME_BOTTOM_LEFT.x && point.x <= FRAME_TOP_RIGHT.x &&
      point.y >= FRAME_BOTTOM_LEFT.y && point.y <= FRAME_TOP_RIGHT.y) {
    return true;
  }
  return false;
}

/**
 * @brief Check if the pacman has consumed the pellet
 *
 * @param scene
 */
void check_hit_pellet(scene_t *scene) {
  // Get first body (pacman)
  body_t *pacman = scene_get_body(scene, 0);
  for (size_t i = 1; i < scene_bodies(scene); i++) {
    // pellet body
    body_t *pellet = scene_get_body(scene, i);
    if (is_inside_pacman(pacman, pellet)) {
      scene_remove_body(scene, i);
    }
  }
}

/**
 * @brief Given a key (UP_ARROW, DOWN_ARROW, etc.), rotates the Pacman body to
 * face correct direction (Pacman originally begins facing the right direction)
 *
 * @param pacman
 * @param key
 */
void rotate_pacman(body_t *pacman, char key) {
  switch (key) {
  case UP_ARROW:
    body_set_rotation(pacman, M_PI / 2);
    break;

  case DOWN_ARROW:
    body_set_rotation(pacman, 3 * M_PI / 2);
    break;

  case RIGHT_ARROW:
    body_set_rotation(pacman, 0);
    break;

  case LEFT_ARROW:
    body_set_rotation(pacman, M_PI);
    break;
  }
}

/**
 * @brief Given a key (UP_ARROW, DOWN_ARROW, etc.) + hold time, updates Pacman
 * velocity
 */
void update_pacman_vel(body_t *pacman, char key, double held_time) {

  vector_t cur_vel = {0, 0};

  switch (key) {
  case UP_ARROW:
    cur_vel.y += PACMAN_ACCELERATION * held_time;
    body_set_velocity(pacman, cur_vel);
    break;

  case DOWN_ARROW:
    cur_vel.y -= PACMAN_ACCELERATION * held_time;
    body_set_velocity(pacman, cur_vel);
    break;

  case RIGHT_ARROW:
    cur_vel.x += PACMAN_ACCELERATION * held_time;
    body_set_velocity(pacman, cur_vel);
    break;

  case LEFT_ARROW:
    cur_vel.x -= PACMAN_ACCELERATION * held_time;
    body_set_velocity(pacman, cur_vel);
    break;
  }
}

/**
 * @brief Performs whatever updates need to be made when a key is being pressed
 *
 * @param key
 * @param type
 * @param held_time
 * @param scene
 */
void onKey(char key, key_event_type_t type, double held_time, void *sc) {
  scene_t *scene = (scene_t *)sc;
  // First index is pacman body
  assert(scene != NULL);

  body_t *pacman = scene_get_body(scene, 0);
  if (type == KEY_PRESSED) {
    rotate_pacman(pacman, key);
    update_pacman_vel(pacman, key, held_time);
  }
}

/**
 * @brief Check if the pacman body is off screen
 *
 * @param scene - the pointer to the scene
 * @return enum off_screen - if the pacman is off the screen (types returned
 * defined in enum)
 */
enum off_screen check_pacman_off_screen(scene_t *scene) {
  // 0th index is pacman body
  body_t *pacman = scene_get_body(scene, 0);

  vector_t pac_centroid = body_get_centroid(pacman);

  if (pac_centroid.x > FRAME_TOP_RIGHT.x) {
    return ALL_RIGHT;
  }
  if (pac_centroid.x < FRAME_BOTTOM_LEFT.x) {
    return ALL_LEFT;
  }
  if (pac_centroid.y > FRAME_TOP_RIGHT.y) {
    return ALL_TOP;
  }
  if (pac_centroid.y < FRAME_BOTTOM_LEFT.y) {
    return ALL_BOTTOM;
  }
  return INSIDE;
}

/**
 * @brief  Wraps around pacman if needed
 *
 * @param scene
 * @param pac_bound
 */
void wrap_around(scene_t *scene, enum off_screen pac_bound) {
  // Get first body (pacman)
  // Check if all of the points of the first body is out of the screen
  // cases:
  // If all points are off left -- move body position to right (centroid)
  // If all points are off right - move body position to left
  // If all points are off top - move body position to bottom
  // If all points are off bottom - move body position to top
  body_t *pacman = scene_get_body(scene, 0);
  vector_t change_centroid = body_get_centroid(pacman);
  switch (pac_bound) {
  case ALL_TOP:
    body_set_centroid(scene_get_body(scene, 0),
                      (vector_t){change_centroid.x, FRAME_BOTTOM_LEFT.y});
    break;
  case ALL_BOTTOM:
    body_set_centroid(scene_get_body(scene, 0),
                      (vector_t){change_centroid.x, FRAME_TOP_RIGHT.y});
    break;
  case ALL_RIGHT:
    body_set_centroid(scene_get_body(scene, 0),
                      (vector_t){FRAME_BOTTOM_LEFT.x, change_centroid.y});
    break;
  case ALL_LEFT:
    body_set_centroid(scene_get_body(scene, 0),
                      (vector_t){FRAME_TOP_RIGHT.x, change_centroid.y});
    break;
  case INSIDE:; // Do nothing
  }
}

/**
 * @brief Initializes scene
 *
 * @return scene_t*
 */
scene_t *emscripten_init() {
  scene_t *scene = scene_init();
  // Initialize sdl
  sdl_init(FRAME_BOTTOM_LEFT, FRAME_TOP_RIGHT);

  list_t *pacman_pts = get_pacman_points(PACMAN_STARTING_X, PACMAN_STARTING_Y);
  body_t *pacman = body_init(pacman_pts, NUM_PELLETS, PACMAN_COLOR);
  body_set_centroid(pacman, (vector_t)body_centroid(pacman_pts));

  // first elem is pacman, rest are pellets
  scene_add_body(scene, pacman);

  srand(time(NULL));

  for (size_t i = 0; i < NUM_PELLETS; i++) {

    // Pacman boundary constraints
    double left_x_constraint =
        PACMAN_STARTING_X - PACMAN_RADIUS - PELLET_RADIUS;
    double right_x_constraint =
        PACMAN_STARTING_X + PACMAN_RADIUS + PELLET_RADIUS;
    double top_y_constraint = PACMAN_STARTING_Y + PACMAN_RADIUS + PELLET_RADIUS;
    double bottom_y_constraint =
        PACMAN_STARTING_Y - PACMAN_RADIUS - PELLET_RADIUS;

    // Generate random x, y location points for each of the pellets

    double x_point = abs(rand() % (int)FRAME_TOP_RIGHT.x);
    double y_point = abs(rand() % (int)FRAME_TOP_RIGHT.y);

    // Generate pellet location points (regenerate if inside of pacman)
    while ((x_point >= left_x_constraint) && (x_point <= right_x_constraint) &&
           (y_point <= bottom_y_constraint) && (y_point <= top_y_constraint)) {
      x_point = abs(rand() % (int)FRAME_TOP_RIGHT.x);
      y_point = abs(rand() % (int)FRAME_TOP_RIGHT.y);
    }
    assert(is_inside_screen((vector_t){x_point, y_point}));

    vector_t pellet_coordinates = {x_point, y_point};
    // Create pellet + add to scene
    body_t *pellet = body_init(get_pellet_points(x_point, y_point), PELLET_MASS,
                               PELLET_COLOR);
    body_set_centroid(pellet, pellet_coordinates);
    assert(pellet != NULL);
    scene_add_body(scene, pellet);
  }
  assert(scene_bodies(scene) >= 10);
  sdl_render_scene(scene);
  sdl_at_scene(scene);
  sdl_on_key((key_handler_t)onKey);
  return scene;
}

/**
 * @brief Main emscripten function
 *
 * @param scene
 */
void emscripten_main(scene_t *scene) {
  assert(scene != NULL);
  sdl_render_scene(scene);
  scene_tick(scene, time_since_last_tick());
  check_hit_pellet(scene);
  if (check_pacman_off_screen(scene) != INSIDE) {
    wrap_around(scene, check_pacman_off_screen(scene));
  }
}

/**
 * @brief Frees emscripten
 *
 * @param scene
 */
void emscripten_free(scene_t *scene) { scene_free(scene); }
