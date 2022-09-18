#include "forces.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <color.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define M_PI 3.14159265358979323846 /* pi */

const size_t NUMBER_POINTS = 75;
const size_t RADIUS = 10;
const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};
const size_t NUMBER_CIRCLES = 50;
const size_t INVS_NUMBER_CIRCLES = 3;
const double CIRCLE_MASS = 8.0;
const double DRAG_CONST = 2;
const double SPRING_CONST = 7;
const double INIT_AMPLITUDE = 100;
const double WAVE_INCREMENT = 10;
const int TRANSLATION_UP = 5;

// Returns a list of points that describe a single circle
list_t *make_circle_points(double center_x, double center_y, size_t rad,
                           size_t num_points) {
  list_t *circle_points = list_init(num_points, (free_func_t)free);

  for (size_t i = 0; i < NUMBER_POINTS; i++) {
    double x_point = center_x + rad * cos(i * 2 * M_PI / num_points);
    double y_point = center_y + rad * sin(i * 2 * M_PI / num_points);
    vector_t *circle_p = malloc(sizeof(vector_t));
    vector_t pel = {x_point, y_point};
    *circle_p = pel;
    list_add(circle_points, circle_p);
  }
  return circle_points;
}

// Goes through a bunch of circles and sets them to their
// proper position along the curve
list_t *set_circle_position(scene_t *scene) {
  int old_y_pos = FRAME_TOP_RIGHT.y / 2;
  list_t *all_circles = list_init(NUMBER_CIRCLES, (free_func_t)free);
  for (size_t i = 1; i <= NUMBER_CIRCLES; i++) {
    double x_pos = RADIUS * 2 * i;

    double y_pos = 0;
    y_pos = old_y_pos + INIT_AMPLITUDE * sin(M_PI * i / WAVE_INCREMENT) +
            TRANSLATION_UP;
    vector_t pos = {x_pos, y_pos};
    list_t *circle_pts =
        make_circle_points(x_pos, y_pos, RADIUS, NUMBER_POINTS);
    body_t *circle = body_init(circle_pts, CIRCLE_MASS, rand_color());
    body_set_centroid(circle, pos);
    list_add(all_circles, circle);
  }
  return all_circles;
}

list_t *set_mid_circles(scene_t *scene) {

  list_t *all_circles = list_init(NUMBER_CIRCLES, (free_func_t)free);
  for (size_t i = 1; i <= NUMBER_CIRCLES; i++) {
    double x_pos = RADIUS * 2 * i;
    double y_pos = FRAME_TOP_RIGHT.y / 2;

    vector_t pos = {x_pos, y_pos};
    list_t *circle_pts =
        make_circle_points(x_pos, y_pos, 1, INVS_NUMBER_CIRCLES);
    // White circles
    body_t *circle = body_init(circle_pts, INFINITY, (rgb_color_t){1, 1, 1});
    body_set_centroid(circle, pos);
    list_add(all_circles, circle);
  }
  return all_circles;
}

void add_drag(scene_t *scene, list_t *circs) {
  for (size_t i = 0; i < NUMBER_CIRCLES; i++) {
    body_t *body = list_get(circs, i);
    create_drag(scene, DRAG_CONST, body);
  }
}

void add_spring(scene_t *scene, list_t *circs, list_t *midbodies) {

  for (size_t i = 0; i < NUMBER_CIRCLES; i++) {
    body_t *body1 = list_get(circs, i);
    body_t *body2 = list_get(midbodies, i);
    create_spring(scene, SPRING_CONST, body1, body2);
  }
}

scene_t *emscripten_init() {
  scene_t *scene = scene_init();
  // Initialize sdl
  sdl_init(FRAME_BOTTOM_LEFT, FRAME_TOP_RIGHT);

  list_t *circles = set_circle_position(scene);
  list_t *midbodies = set_mid_circles(scene);

  // add circles to scene
  for (size_t i = 0; i < NUMBER_CIRCLES; i++) {
    scene_add_body(scene, list_get(midbodies, i));
    scene_add_body(scene, list_get(circles, i));
  }
  // add drag force to them
  add_drag(scene, circles);
  // add_drag(scene, midbodies);
  // add spring force
  add_spring(scene, circles, midbodies);

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
