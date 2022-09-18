#include "sdl_wrapper.h"
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define M_PI 3.14159265358979323846 /* pi */
#include "body.h"
#include "color.h"
#include "scene.h"
#include "state.h"
#ifdef __EMSCRIPTEN__
#include "list.h"
#include <emscripten.h>
#endif

const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};
const vector_t STAR_VELOCITY = {50, -50};
const double ROTATION = M_PI / 81;
const double OUTER_STAR_RADIUS = 50;
const double INNER_STAR_RADIUS = 20;
const int MAX_NUM_POINTS = 20;
const double ACCELERATION = 9.81;
const double MASS = 30;
const double NEW_POLY_VEL = 2;
const int INITIAL_NUM_POLYGONS = 10;
const double ELASTICITY_START_RANGE = 90;

/**
 * @brief Returns a random number of points for a polygon bounded by constant
 * max_num_points
 *
 * @return size_t
 */
size_t rand_num_points() {
  srand(time(NULL));
  size_t num_points = 0;
  while (num_points <= 2) {
    num_points = rand() % MAX_NUM_POINTS;
  }
  return num_points;
}

/**
 * @brief Frees the memory associated with state (i.e., all memory overall)
 *
 * @param scene - pointer to scene
 */
void emscripten_free(scene_t *scene) { scene_free(scene); }

/**
 * @brief Given a desired number of points, return a list of the star points
 *        centered at the top left of the screen
 *
 * @param num_points - number of points for the star
 * @return vec_list_t*
 */
list_t *get_star_points(size_t num_points) {

  list_t *poly_points = list_init(num_points, (free_func_t)free);

  // Finds the number of half points (accounts for an odd number of points
  // through integer division)
  size_t half_points = num_points / 2;

  // For the case of a triangle, let the number of half points equal 3
  if (num_points == 3) {
    half_points = 3;
  }

  // Create star points as vector_t's
  for (size_t i = 0; i < half_points; i++) {
    vector_t outer_point = {
        OUTER_STAR_RADIUS *
            cos(2 * (double)M_PI * i / half_points + (double)M_PI / 2),
        OUTER_STAR_RADIUS *
                sin(2 * (double)M_PI * i / half_points + (double)M_PI / 2) +
            FRAME_TOP_RIGHT.y};
    vector_t inner_point = {
        INNER_STAR_RADIUS * cos((2 * (double)M_PI * i / half_points) +
                                (M_PI / 2 + M_PI / half_points)),
        INNER_STAR_RADIUS * sin((2 * M_PI * i / half_points) +
                                (M_PI / 2 + M_PI / half_points)) +
            FRAME_TOP_RIGHT.y};

    // Creating pointer + adding outerpoint to vector list
    vector_t *pointer1 = malloc(sizeof(vector_t));
    *pointer1 = outer_point;
    list_add(poly_points, pointer1);

    // Creating pointer + adding innerpoint to vector list
    // Don't need to add inner points if a triangle
    if (num_points != 3) {
      vector_t *pointer2 = malloc(sizeof(vector_t));
      *pointer2 = inner_point;
      list_add(poly_points, pointer2);
    }
  }
  assert(list_size(poly_points) > 0);
  return poly_points;
}

/**
 * @brief Initializes sdl as well as the variables needed
 *        Creates + stores necessary variables for demo
 *        Returns a pointer to the overall state
 *
 * @rturn state_t*
 */
scene_t *emscripten_init() {
  // Initialize scene
  scene_t *scene = scene_init();
  // Initialize sdl
  sdl_init(FRAME_BOTTOM_LEFT, FRAME_TOP_RIGHT);

  list_t *polygon_points = get_star_points(rand_num_points());
  body_t *star = body_init(polygon_points, MASS, rand_color());
  body_set_centroid(star, body_centroid(polygon_points));
  body_set_velocity(star, STAR_VELOCITY);
  // Add first random polygon
  scene_add_body(scene, star);
  sdl_render_scene(scene);
  sdl_at_scene(scene);
  return scene;
}

bool all_right(body_t *bod, vector_t right_vec) {
  list_t *points = get_body_points(bod);
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *cur_point = (vector_t *)list_get(get_body_points(bod), i);
    if (cur_point->x < right_vec.x) {
      return false;
    }
  }
  return true;
}

void check_all_at_right(scene_t *scene) {
  size_t size = scene_bodies(scene);
  for (size_t i = 0; i < size; i++) {
    if (all_right(scene_get_body(scene, i), FRAME_TOP_RIGHT)) {
      scene_remove_body(scene, i);
    }
  }
}

bool at_bottom(body_t *bod, vector_t bottom_vec) {
  list_t *body_points = get_body_points(bod);
  for (size_t i = 0; i < list_size(body_points); i++) {
    vector_t *cur_point = (vector_t *)list_get(body_points, i);
    if (cur_point->y <= bottom_vec.y) {
      return true;
    }
  }
  return false;
}

/**
 * @brief If a body is at the bottom, updates the velocity of that body
 *
 * @param scene
 */
void check_one_at_bottom(scene_t *scene) {
  size_t size = scene_bodies(scene);
  for (size_t i = 0; i < size; i++) {
    body_t *cur_body = scene_get_body(scene, i);
    // want to change velocity of body
    if (at_bottom(cur_body, FRAME_BOTTOM_LEFT)) {
      double elasticity = (ELASTICITY_START_RANGE + (rand() % 10)) / 100.0;
      vector_t cur_velocity = body_get_velocity(cur_body);
      body_set_velocity(
          cur_body,
          (vector_t){cur_velocity.x, fabs(cur_velocity.y) * elasticity});
    }
  }
}

/**
 * @brief Called on each tick of the program
 *        Updates the state variables and display as necessary, depending on the
 * time that has passed
 *
 * @param ov_state
 */
void emscripten_main(scene_t *scene) {
  double dt = time_since_last_tick();

  assert(scene != NULL);

  sdl_render_scene(scene);

  scene_tick(scene, time_since_last_tick());

  size_t num_bodies = scene_bodies(scene);
  body_t *last_body = scene_get_body(scene, num_bodies - 1);

  // If the y velocity of the previous polygon is close to 0, then add a new
  // polygon (if there's more to add)
  if (body_get_velocity(last_body).y < NEW_POLY_VEL &&
      body_get_velocity(last_body).y > -NEW_POLY_VEL) {
    // Add new polygon with random number of sides + random color
    list_t *polygon_points = get_star_points(rand_num_points());
    body_t *star = body_init(polygon_points, MASS, rand_color());
    body_set_centroid(star, body_centroid(polygon_points));
    body_set_velocity(star, STAR_VELOCITY);
    scene_add_body(scene, star);
  }

  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *cur_body = scene_get_body(scene, i);
    // If all at right, remove
    if (all_right(scene_get_body(scene, i), FRAME_TOP_RIGHT)) {
      scene_remove_body(scene, i);
    }
    // If one at bottom, update velocity accordingly
    else if (at_bottom(cur_body, FRAME_BOTTOM_LEFT)) {
      double elasticity = (ELASTICITY_START_RANGE + (rand() % 10)) / 100.0;
      vector_t cur_velocity = body_get_velocity(cur_body);
      body_set_velocity(
          cur_body,
          (vector_t){cur_velocity.x, fabs(cur_velocity.y) * elasticity});
      body_set_rotation_relative(cur_body, ROTATION);
    }
    // Otherwise, update velocity normally
    else {
      vector_t cur_velocity = body_get_velocity(cur_body);
      body_set_velocity(cur_body,
                        (vector_t){cur_velocity.x,
                                   cur_velocity.y - ACCELERATION * MASS * dt});
      body_set_rotation_relative(cur_body, ROTATION);
    }
  }
}
