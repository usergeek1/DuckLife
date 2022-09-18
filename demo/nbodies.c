#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};
const vector_t STAR_VELOCITY = {50, -50};
const double ROTATION = M_PI / 81;
const double MAX_OUTER_STAR_RADIUS = 30;
const double INNER_STAR_RADIUS = 15;
const int MAX_NUM_POINTS = 10;
const double ACCELERATION = 9.81;
const double MASS = 50;
const int NUM_STARS = 30;
const double G = 6.6743 * 20;

/**
 * @brief Given a desired number of points, return a list of the star points
 *        centered at some random location in the screen
 *
 * @param num_points - number of points for the star
 * @return vec_list_t*
 */
list_t *get_star_points(size_t num_points) {

  list_t *poly_points = list_init(num_points, (free_func_t)free);
  double outer_star_radius = rand() % 10 + (MAX_OUTER_STAR_RADIUS - 10);
  double inner_star_radius = outer_star_radius / 2;

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
        outer_star_radius *
            cos(2 * (double)M_PI * i / half_points + (double)M_PI / 2),
        outer_star_radius *
            sin(2 * (double)M_PI * i / half_points + (double)M_PI / 2)};
    vector_t inner_point = {
        inner_star_radius * cos((2 * (double)M_PI * i / half_points) +
                                (M_PI / 2 + M_PI / half_points)),
        inner_star_radius * sin((2 * M_PI * i / half_points) +
                                (M_PI / 2 + M_PI / half_points))};

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

  // Generate random x value in range
  // Generate random y value in range

  double x_point = abs(rand() % (int)FRAME_TOP_RIGHT.x);
  double y_point = abs(rand() % (int)FRAME_TOP_RIGHT.y);
  body_translate(poly_points, (vector_t){x_point, y_point});
  return poly_points;
}

size_t rand_num_points() {
  size_t num_points = 0;
  while (num_points <= 2) {
    num_points = rand() % MAX_NUM_POINTS;
  }
  return num_points;
}

/**
 * @brief Create list of stars
 *
 * @return list_t
 */
list_t *get_stars_list() {
  srand(time(NULL));
  list_t *stars = list_init(NUM_STARS, (free_func_t)body_free);
  for (size_t i = 0; i < NUM_STARS; i++) {
    list_t *star_points = get_star_points(rand_num_points());
    body_t *star_body = body_init(star_points, MASS, rand_color());
    body_set_centroid(star_body, body_centroid(star_points));
    list_add(stars, star_body);
  }

  return stars;
}

/**
 * @brief Given a list of stars, add to gravity lists for each pair of stars
 *
 * @param scene
 * @param stars
 */
void add_gravity(scene_t *scene, list_t *stars) {
  for (size_t i = 0; i < list_size(stars); i++) {
    for (size_t j = 0; j < list_size(stars); j++) {
      if (i != j) {
        body_t *body_1 = list_get(stars, i);
        body_t *body_2 = list_get(stars, j);
        create_newtonian_gravity(scene, G, body_1, body_2);
      }
    }
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

  // Add all of the stars
  list_t *stars = get_stars_list();
  for (size_t i = 0; i < list_size(stars); i++) {
    scene_add_body(scene, list_get(stars, i));
  }
  add_gravity(scene, stars);

  sdl_render_scene(scene);
  sdl_at_scene(scene);
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
}

/**
 * @brief Frees emscripten
 *
 * @param scene
 */
void emscripten_free(scene_t *scene) { scene_free(scene); }