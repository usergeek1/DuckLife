//#include "vec_list.h"
#include "body.h"
#include "vector.h"
#include <color.h>
#include <math.h>
#include <sdl_wrapper.h>
#include <stdio.h>
#include <stdlib.h>

const double ROTATION = 0.05;
const double NUM_POINTS = 15;
const double PACMAN_MASS = 10;

const double CIRC_IN_RADS = 6.28319;

const double OUTER_RAD = 100.0;
const double INNER_RAD = 100 * 0.382;

const vector_t VELO = (vector_t){100, 100};

const double SCREEN_WIDTH = 1000;
const double SCREEN_HEIGHT = 500;
const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};

list_t *get_poly_points(scene_t *scene) {
  // put a free function in there
  list_t *poly_points = list_init(NUM_POINTS * 2, free);
  double delta_angle = CIRC_IN_RADS / NUM_POINTS / 2;

  for (size_t i = 0; i < NUM_POINTS * 2; i++) {
    vector_t *new_vec = malloc(sizeof(vector_t));

    if (i % 2 == 0) {
      new_vec->x = OUTER_RAD * cos(delta_angle * i);
      new_vec->y = OUTER_RAD * sin(delta_angle * i);
    } else {
      new_vec->x = INNER_RAD * cos(delta_angle * i);
      new_vec->y = INNER_RAD * sin(delta_angle * i);
    }

    list_add(poly_points, (void *)new_vec);
  }

  vector_t move_to_middle = {.x = SCREEN_WIDTH / 2, .y = SCREEN_HEIGHT / 2};
  body_translate(poly_points, move_to_middle);
  return poly_points;
}

scene_t *emscripten_init() {
  scene_t *scene = scene_init();
  sdl_init(FRAME_BOTTOM_LEFT, FRAME_TOP_RIGHT);
  vector_t centroid = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
  body_t *body = body_init(get_poly_points(scene), PACMAN_MASS, rand_color());
  body_set_centroid(body, centroid);
  body_set_velocity(body, VELO);
  scene_add_body(scene, body);
  sdl_render_scene(scene);
  sdl_at_scene(scene);
  return scene;
}

void check_edge(scene_t *scene) {
  size_t items_inscene = scene_bodies(scene);

  for (size_t i = 0; i < items_inscene; i++) {
    body_t *curr_body = scene_get_body(scene, i);
    list_t *body_pts = get_body_points(curr_body);
    size_t num_vecs_in_body = list_size(body_pts);

    for (size_t j = 0; j < num_vecs_in_body; j++) {
      vector_t vec = *(vector_t *)list_get(body_pts, j);
      // top or bottom
      if (vec.y <= 0) {
        body_set_y_velo(curr_body, fabs(body_get_y_velo(curr_body)));
      } else if (vec.y >= SCREEN_HEIGHT) {
        body_set_y_velo(curr_body, -1.0 * fabs(body_get_y_velo(curr_body)));
      }
      // right or left
      if (vec.x <= 0) {
        body_set_x_velo(curr_body, fabs(body_get_x_velo(curr_body)));
      } else if (vec.x >= SCREEN_WIDTH) {
        body_set_x_velo(curr_body, -1.0 * fabs(body_get_y_velo(curr_body)));
      }
    }
  }
}

void emscripten_main(scene_t *scene) {
  check_edge(scene);
  sdl_render_scene(scene);
  scene_tick(scene, time_since_last_tick());
}

void emscripten_free(scene_t *scene) { scene_free(scene); }
