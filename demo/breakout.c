#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CIRCLE_POINTS 40
#define MAX ((vector_t){.x = 1000, .y = 500.0})

// Ball Definitions
#define BALL_RADIUS 10.0
#define BALL_ELASTICITY 1
#define WALL_WIDTH 1.0
#define BALL_START_VELOCITY ((vector_t){.void remove_all(scene_t *scene) {
size_t number_of_bodies = scene_bodies(scene);
for (size_t i = 0; i < number_of_bodies; i++) {
  scene_remove_body(scene, i);
}
}
x = 100.0, .y = -400.0
})
#define BALL_MAX_ADD_SIZE 10
#define BALL_MASS 40
#define BALL_COLOR ((rgb_color_t){1, 0, 0})

// Wall definitionss
#define WALL_LENGTH hypot(MAX.x / 2, MAX.y)
#define WALL_COLOR ((rgb_color_t){0, 0, 1})

// Additional constants
// Screen constants
const vector_t FRAME_BOTTOM_LEFT = {0, 0};
const vector_t FRAME_TOP_RIGHT = {1000, 500};
const vector_t FRAME_CENTER = {500, 250};
const vector_t ZERO_VEC = {0, 0};

// Player constants
const double PLAYER_WIDTH = 80;
const double PLAYER_HEIGHT = 20;
const double PLAYER_STARTING_X = 500;
const double PLAYER_STARTING_Y = 60;
const double PLAYER_EDGE_BUFFER = 20;
const size_t NUM_PLAYER_POINTS = 80;
const rgb_color_t PLAYER_COLOR = {1, .682, .259};
const vector_t PLAYER_VEL = {400, 0};
const double CONST_COLOR = 100;
const size_t PLAYER_INDEX = 1;

// Brick constants
const double BRICK_W = 100;
const double BRICK_H = 30;
const double BRICK_MASS = INFINITY;
const double NUM_BRICK_ROWS = 2;
const double NUM_BRICKS = 7;
const double BRICK_STEP_X = 100;
const double BRICK_STEP_Y = 20;
const double BRICK_X_MOVE = 400;
const double BRICK_Y_MOVE = 50;
const double BRICK_STARTING_X = 25;
const double BRICK_STARTING_Y = 460;
const double BRICK_END_SPACE_BUFFER = 10;
const double BRICK_ROW_SPACE_BUFFER = 40;

// Color Constants
const double COLOR_CONSTANT_DIVIDE = 30;
const double COLOR_CONSTANT_MULT_1 = 2;
const double COLOR_CONSTANT_MULT_2 = 4;

// Physics Constants
#define G 6.67E-11          // N m^2 / kg^2
#define M 6E24              // kg
#define g 9.8               // m / s^2
#define R (sqrt(G * M / g)) // m

// Enum for type of body
typedef enum {
  BALL,
  WALL,
  BOTTOM, // or peg
  BRICK,
  PLAYER
} body_type_t;

// Enum for off screen
enum off_screen {
  ALL_TOP = 1,
  ALL_BOTTOM = 2,
  ALL_RIGHT = 3,
  ALL_LEFT = 4,
  INSIDE = 0
};

body_type_t *make_type_info(body_type_t type) {
  body_type_t *info = malloc(sizeof(*info));
  *info = type;
  return info;
}

body_type_t get_type(body_t *body) {
  return *(body_type_t *)body_get_info(body);
}

/** Generates a random number between 0 and 1 */
double rand_double(void) { return (double)rand() / RAND_MAX; }

/** Constructs a rectangle with the given dimensions centered at (0, 0) */
list_t *rect_init(double width, double height) {
  vector_t half_width = {.x = width / 2, .y = 0.0},
           half_height = {.x = 0.0, .y = height / 2};
  list_t *rect = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = vec_add(half_width, half_height);
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = vec_subtract(half_height, half_width);
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = vec_negate(*(vector_t *)list_get(rect, 0));
  list_add(rect, v);
  v = malloc(sizeof(*v));
  *v = vec_subtract(half_width, half_height);
  list_add(rect, v);
  return rect;
}

/** Constructs a circles with the given radius centered at (0, 0) */
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

/** Creates a ball with the given starting position and velocity */
body_t *get_ball(vector_t center, vector_t velocity) {
  list_t *shape = circle_init(BALL_RADIUS);
  body_t *ball =
      body_init_with_info(shape, BALL_MASS, BALL_COLOR, make_type_info(BALL));

  body_set_centroid(ball, center);
  body_set_velocity(ball, velocity);

  return ball;
}

// Removes all the bodies from the scene

// Collision handler to remove body and apply impulses
void remove_body_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                   void *aux) {
  // Auxillary function should contain elasticity
  double elasticity = *((double *)aux);

  // Resets ball to random color / radius size
  vector_t cur_centroid = body_get_centroid(body1);
  list_t *new_shape =
      circle_init(BALL_RADIUS + rand_double() * BALL_MAX_ADD_SIZE);
  body_set_shape(body1, new_shape);
  body_set_centroid(body1, cur_centroid);

  body_set_color(body1, rand_color());

  // Applies impulse to both bodies
  apply_impulse(body1, body2, axis, elasticity);

  // Removes second body
  body_remove(body2);
}

/** Adds the boxes (at top) to the scene */
void add_boxes(scene_t *scene) {
  for (size_t j = 0; j < NUM_BRICK_ROWS; j++) {
    // Changing y for each row
    double new_brick_y = BRICK_STARTING_Y - (j)*BRICK_ROW_SPACE_BUFFER;
    double brick_dist = FRAME_TOP_RIGHT.x / NUM_BRICKS - BRICK_END_SPACE_BUFFER;
    double new_brick_x = BRICK_STARTING_X + brick_dist / 2;

    for (size_t i = 0; i < NUM_BRICKS; i++) {
      float r_val = fabs(sin(CONST_COLOR * i * COLOR_CONSTANT_DIVIDE));
      float g_val = fabs(sin((CONST_COLOR * i * COLOR_CONSTANT_DIVIDE) +
                             ((COLOR_CONSTANT_MULT_1 * M_PI / NUM_BRICKS))));
      float b_val = fabs(sin((CONST_COLOR * i * COLOR_CONSTANT_DIVIDE) +
                             ((COLOR_CONSTANT_MULT_2 * M_PI / NUM_BRICKS))));

      rgb_color_t random_color = {r_val, g_val, b_val};

      list_t *brick_pts = rect_init(BRICK_W, BRICK_H);
      vector_t translation = {new_brick_x, new_brick_y};
      body_translate(brick_pts, translation);
      // Changing x for each brick in a row
      new_brick_x += brick_dist;
      body_t *brick = body_init_with_info(brick_pts, BRICK_MASS, random_color,
                                          (void *)make_type_info(BRICK));
      body_set_centroid(brick, (vector_t)body_centroid(brick_pts));
      scene_add_body(scene, brick);
    }
  }
}

/** Adds the walls to the scene */
void add_walls(scene_t *scene) {
  // Add left wall
  list_t *rect = rect_init(WALL_LENGTH, WALL_WIDTH);
  polygon_translate(rect, (vector_t){.x = WALL_LENGTH / 2, .y = 0});
  polygon_rotate(rect, M_PI / 2, VEC_ZERO);
  body_t *body =
      body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(WALL));
  body_set_centroid(body, body_centroid(rect));
  scene_add_body(scene, body);

  // Add right wall
  rect = rect_init(WALL_LENGTH, WALL_WIDTH);
  polygon_translate(rect, (vector_t){.x = WALL_LENGTH / 2, .y = -MAX.x});
  polygon_rotate(rect, M_PI / 2, VEC_ZERO);
  body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(WALL));
  body_set_centroid(body, body_centroid(rect));
  scene_add_body(scene, body);

  // Add ground wall
  rect = rect_init(MAX.x, WALL_WIDTH);
  body =
      body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(BOTTOM));
  body_set_centroid(body, (vector_t){.x = MAX.x / 2, .y = WALL_WIDTH / 2});
  scene_add_body(scene, body);

  // Add top wall
  rect = rect_init(MAX.x, WALL_WIDTH);
  polygon_translate(rect, (vector_t){MAX.x / 2, MAX.y});
  body = body_init_with_info(rect, INFINITY, WALL_COLOR, make_type_info(WALL));
  body_set_centroid(body, body_centroid(rect));
  scene_add_body(scene, body);
}

// Checks if a player body is off screen
enum off_screen player_off_screen(scene_t *scene, int ind) {
  body_t *cur_body = scene_get_body(scene, ind);
  vector_t body_cent = body_get_centroid(cur_body);
  double x_rad = PLAYER_WIDTH;
  double y_rad = PLAYER_HEIGHT;
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

// Player to move the functions (depending on the key pressed)
void move_player(scene_t *scene, body_t *player, char key) {

  if (key == LEFT_ARROW && player_off_screen(scene, PLAYER_INDEX) != ALL_LEFT) {
    body_set_velocity(player, vec_negate(PLAYER_VEL));
  } else if (key == RIGHT_ARROW &&
             player_off_screen(scene, PLAYER_INDEX) != ALL_RIGHT) {
    body_set_velocity(player, PLAYER_VEL);
  } else if (player_off_screen(scene, PLAYER_INDEX) == ALL_LEFT) {
    body_set_velocity(player, ZERO_VEC);
    body_set_centroid(player, (vector_t){PLAYER_WIDTH / 2, PLAYER_STARTING_Y});
  } else if (player_off_screen(scene, PLAYER_INDEX) == ALL_RIGHT) {
    body_set_velocity(player, ZERO_VEC);
    body_set_centroid(player, (vector_t){FRAME_TOP_RIGHT.x - PLAYER_WIDTH / 2,
                                         PLAYER_STARTING_Y});
  } else
    body_set_velocity(player, ZERO_VEC);
}

// Function for handling keyboard input (specifically left/right arrows for
// player)
void onKey(char key, key_event_type_t type, double held_time, void *sc) {
  scene_t *scene = (scene_t *)sc;
  assert(scene != NULL);

  body_t *player = scene_get_body(scene, 1);
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      move_player(scene, player, key);
      break;
    case RIGHT_ARROW:
      move_player(scene, player, key);
      break;
    }
  } else {
    body_set_velocity(player, ZERO_VEC);
  }
}

// Regenerates everything
void regenerate(scene_t *scene) {

  // Add ball to be right above player with initial downward velocity
  list_t *ball_points = circle_init(BALL_RADIUS);
  body_t *ball = body_init_with_info(ball_points, BALL_MASS, BALL_COLOR,
                                     (void *)make_type_info(BALL));
  body_set_centroid(ball, (vector_t){PLAYER_STARTING_X,
                                     PLAYER_STARTING_Y + BALL_RADIUS * 10});
  body_set_velocity(ball, BALL_START_VELOCITY);
  scene_add_body(scene, ball);

  // Adds player to desired starting location
  // Set player to infinite mass so it isnt affected by collision
  list_t *player_pts = rect_init(PLAYER_WIDTH, PLAYER_HEIGHT);
  body_t *player = body_init_with_info(player_pts, INFINITY, PLAYER_COLOR,
                                       (void *)make_type_info(PLAYER));
  body_set_centroid(player, (vector_t){PLAYER_STARTING_X, PLAYER_STARTING_Y});
  scene_add_body(scene, player);

  // Add elements (boxes + walls) to the scene
  add_boxes(scene);
  add_walls(scene);
}

// Restarts game (resets all bodies to original positions)
void game_restart(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  remove_all((scene_t *)(aux));
  regenerate((scene_t *)(aux));
}

/** Adds collisions between ball and every other body */
void add_collisions(scene_t *scene) {
  // Get ball body (first body in scene)
  body_t *ball = scene_get_body(scene, 0);
  size_t body_count = scene_bodies(scene);
  double *elas = malloc(sizeof(double));
  *elas = BALL_ELASTICITY;
  // Add force creators with other bodies
  for (size_t i = 1; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    switch (get_type(body)) {
    case PLAYER:
      // Player hits ball (ball should bounce off - elastic collision)
      create_physics_collision(scene, BALL_ELASTICITY, ball, body);
      break;
    case WALL:
      // Bounce off walls and pegs
      create_physics_collision(scene, BALL_ELASTICITY, ball, body);
      break;
    // In the case of a brick, want to get rid of the brick (need to update this
    // )
    case BRICK:
      create_collision(scene, ball, body,
                       (collision_handler_t)remove_body_collision_handler,
                       (void *)elas, free);
      break;
    case BALL:
      break; // not a possible case
    case BOTTOM:
      create_collision(scene, ball, body, (collision_handler_t)game_restart,
                       scene, NULL);
      break;
    }
  }
}

// Emscripten init to create screen and generate everything for the first time
scene_t *emscripten_init(void) {
  srand(time(NULL));
  // Initialize scene
  sdl_init(VEC_ZERO, MAX);
  scene_t *scene = scene_init();

  regenerate(scene);
  sdl_at_scene(scene);
  sdl_on_key((key_handler_t)onKey);

  return scene;
}

// Function that returns whether all bricks have been hit
bool all_bricks_hit(scene_t *scene) {
  size_t num_bodies = scene_bodies(scene);
  for (size_t i = 0; i < num_bodies; i++) {
    body_t *cur_body = scene_get_body(scene, i);
    if (get_type(cur_body) == BRICK)
      return false;
  }
  return true;
}

void emscripten_main(scene_t *scene) {
  double dt = time_since_last_tick();

  // Add collisions between bodies
  add_collisions(scene);

  scene_tick(scene, dt);

  // Check if all bricks have been hit, if so regenerate
  if (all_bricks_hit(scene)) {
    remove_all(scene);
    regenerate(scene);
  }

  sdl_render_scene(scene);
}

// Frees scene
void emscripten_free(scene_t *scene) {
  scene_free(scene);
  free(scene);
}
