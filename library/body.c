#include "body.h"
#include "color.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int INITIAL_LIST_SIZE = 10;

typedef struct body {
  list_t *shape;
  vector_t velo;
  double mass;
  rgb_color_t color;
  vector_t centroid;
  double angle;
  vector_t forces;
  vector_t impulses;
  void *type_of_bod;
  int remove_flag;
  bool in_collision;
  body_t *col_body;
  char *image_path;

} body_t;

char *body_get_image_path(body_t *bod){
  return bod->image_path;
}

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  body_t *body = malloc(sizeof(body_t));
  body->shape = shape;

  body->forces = (vector_t){0, 0};

  body->impulses = (vector_t){0, 0};

  body->color = color;
  body->mass = mass;
  body->velo = (vector_t){0, 0};
  body->centroid = body_centroid(shape);
  body->angle = 0.0;
  body->remove_flag = 0;
  body->type_of_bod = (void *)NULL;
  body->in_collision = false;
  body->image_path = NULL;
  return body;
}

body_t *body_init_sprite(list_t *shape, double mass, rgb_color_t color, char *image_path) {
  body_t *body = malloc(sizeof(body_t));
  body->shape = shape;

  body->forces = (vector_t){0, 0};

  body->impulses = (vector_t){0, 0};

  body->color = color;
  body->mass = mass;
  body->velo = (vector_t){0, 0};
  body->centroid = body_centroid(shape);
  body->angle = 0.0;
  body->remove_flag = 0;
  body->type_of_bod = (void *)NULL;
  body->in_collision = false;
  body->image_path = image_path;
  return body;
}



body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color, void *type_of_bod) {
  body_t *body = body_init(shape, mass, color);
  body->type_of_bod = type_of_bod;
  return body;
}

body_t *body_init_with_info_with_image(list_t *shape, double mass, rgb_color_t color, char *image_path, void *type_of_bod) {
  body_t *body = body_init_sprite(shape, mass, color, image_path);
  body->type_of_bod = type_of_bod;
  return body;
}

void body_free(body_t *body) {
  list_free(body->shape);
  free(body);
}

void *body_get_info(body_t *body_type) { return body_type->type_of_bod; }

double body_area(list_t *polygon) {
  double area = 0.0;
  size_t i;
  size_t size = list_size(polygon);
  size_t index = size - 1;
  for (i = 0; i < size; i++) {
    vector_t *point1 = (vector_t *)list_get(polygon, index);
    vector_t *point2 = (vector_t *)list_get(polygon, i);
    area += (point1->x + point2->x) * (point1->y - point2->y);
    index = i;
  }
  return fabs(area / 2.0);
}

void body_translate(list_t *polygon, vector_t translation) {
  size_t i;
  size_t size = list_size(polygon);
  for (i = 0; i < size; i++) {
    vector_t *point = (vector_t *)list_get(polygon, i);
    point->x += translation.x;
    point->y += translation.y;
  }
}

void body_rotate(list_t *polygon, double angle, vector_t point) {
  size_t i;
  size_t size = list_size(polygon);
  double x_coord;
  double y_coord;
  for (i = 0; i < size; i++) {
    vector_t *cur_point = (vector_t *)list_get(polygon, i);
    x_coord = cur_point->x;
    y_coord = cur_point->y;
    cur_point->x = point.x + (cos(angle) * (x_coord - point.x) -
                              sin(angle) * (y_coord - point.y));
    cur_point->y = point.y + (sin(angle) * (x_coord - point.x) +
                              cos(angle) * (y_coord - point.y));
  }
}

list_t *body_get_shape(body_t *body) {
  int body_shape_size = list_size(body->shape);
  list_t *new_shape = list_init(body_shape_size, free);
  for (size_t i = 0; i < body_shape_size; i++) {
    vector_t *element = malloc(sizeof(vector_t));
    *element = *((vector_t *)list_get(body->shape, i));
    list_add(new_shape, element);
  }
  assert(list_size(new_shape) == body_shape_size);
  return new_shape;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

double body_get_area(body_t *body) { return body_area(body->shape); }

vector_t body_get_velocity(body_t *body) { return body->velo; }

double body_get_x_velo(body_t *body) { return body->velo.x; }

double body_get_y_velo(body_t *body) { return body->velo.y; }

void body_set_color(body_t *body, rgb_color_t col) { body->color = col; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

double body_get_mass(body_t *body) { return body->mass; }

void body_set_centroid(body_t *body, vector_t x) {
  vector_t translation = vec_subtract(x, body_get_centroid(body));
  body_translate(body->shape, translation);
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velo = v; }

void body_set_x_velo(body_t *body, double vx) { body->velo.x = vx; }

void body_set_y_velo(body_t *body, double vy) { body->velo.y = vy; }

double body_get_angle(body_t *body) { return body->angle; }

void body_set_rotation(body_t *body, double angle) {
  vector_t centroid = body_get_centroid(body);
  body_rotate(body->shape, angle - body_get_angle(body), centroid);
  body->angle = angle;
}

void body_set_rotation_relative(body_t *body, double angle) {
  vector_t centroid = body_get_centroid(body);
  body_rotate(body->shape, angle, centroid);
  body->angle = angle;
}

void body_tick(body_t *body, double dt) {
  double vel_x = 0;
  double vel_y = 0;
  // list_t *fs = body->forces;
  vector_t fs = body->forces;
  vector_t is = body->impulses;

  // Iterate through all forces and add to velocities
  vel_x += fs.x / body->mass * dt;
  vel_y += fs.y / body->mass * dt;
  // Iterate through all impulses and add to velocities
  vel_x += is.x / body->mass;
  vel_y += is.y / body->mass;
  body->forces = (vector_t){0, 0};
  body->impulses = (vector_t){0, 0};
  vector_t prev_vel = body_get_velocity(body);

  body_set_velocity(body, vec_add(prev_vel, (vector_t){vel_x, vel_y}));
  // body_get_velocity(body).y);
  // Translate body with average of previous and new velocities
  vector_t translation = vec_multiply(
      dt, vec_multiply(0.5, vec_add(prev_vel, body_get_velocity(body))));
  // Update centroid
  body_set_centroid(body, vec_add(body->centroid, translation));
  
}

list_t *get_body_points(body_t *body) { return body->shape; }

void body_add_force(body_t *body, vector_t force) {
  body->forces = vec_add(body->forces, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulses = vec_add(body->impulses, impulse);
}

vector_t body_centroid(list_t *polygon) {
  // Find polygon's signed area as described by shoelace formula
  double area = body_area(polygon);
  size_t array_size = list_size(polygon);
  double x_coord = 0;
  double y_coord = 0;

  size_t index = array_size - 1;
  // Calculate x,y coordinates
  for (size_t i = 0; i < array_size; i++) {
    vector_t *point1 = (vector_t *)list_get(polygon, index);
    vector_t *point2 = (vector_t *)list_get(polygon, i);
    x_coord += (point1->x + point2->x) *
               (point1->x * point2->y - point2->x * point1->y);
    y_coord += (point1->y + point2->y) *
               (point1->x * point2->y - point2->x * point1->y);
    index = i;
  }
  x_coord = x_coord / (6.0 * area);
  y_coord = y_coord / (6.0 * area);

  // Return instance of vector_t
  vector_t centroid = {x_coord, y_coord};
  return centroid;
}

void body_remove(body_t *body) { body->remove_flag = 1; }

bool body_is_removed(body_t *body) { return body->remove_flag == 1; }

list_t *find_edges(list_t *shape) {
  list_t *edges = list_init(list_size(shape), free);
  for (size_t i = 0; i < list_size(shape) - 1; i++) {
    // current point
    vector_t cur_point = *(vector_t *)list_get(shape, i);
    // next point
    vector_t next_point = *(vector_t *)list_get(shape, i + 1);
    vector_t *e = malloc(sizeof(vector_t));
    e->x = vec_subtract(next_point, cur_point).x;
    e->y = vec_subtract(next_point, cur_point).y;
    list_add(edges, e);
  }
  return edges;
}

list_t *find_axes(list_t *edges) {
  list_t *axes = list_init(list_size(edges), free);
  for (size_t i = 0; i < list_size(edges); i++) {
    vector_t cur_edge;
    cur_edge.x = ((vector_t *)(list_get(edges, i)))->x;
    cur_edge.y = ((vector_t *)(list_get(edges, i)))->y;
    vector_t *axis = malloc(sizeof(vector_t));
    axis->x = cur_edge.y;
    axis->y = -cur_edge.x;
    axis->x =
        cur_edge.y / sqrt(cur_edge.y * cur_edge.y + cur_edge.x * cur_edge.x);
    axis->y =
        -cur_edge.x / sqrt(cur_edge.y * cur_edge.y + cur_edge.x * cur_edge.x);
    list_add(axes, axis);
  }
  return axes;
}

vector_t body_proj_on_axis(list_t *sh, vector_t *axis) {
  double min_value = INFINITY;
  double max_value = -INFINITY;

  // Iterate through each of the points in shape, compute dot product with unit
  // vector axis
  vector_t axis2;
  axis2.x = axis->x;
  axis2.y = axis->y;
  axis2 = find_unit_vector(axis2);
  for (size_t i = 0; i < list_size(sh); i++) {
    double dot_prod = vec_dot(axis2, *(vector_t *)(list_get(sh, i)));
    min_value = min(dot_prod, min_value);
    max_value = max(dot_prod, max_value);
  }
  return (vector_t){min_value, max_value};
}

double check_overlap_axis(list_t *shape1, list_t *shape2, vector_t *axis) {
  vector_t proj1 = body_proj_on_axis(shape1, axis);
  vector_t proj2 = body_proj_on_axis(shape2, axis);
  if (is_overlapping(proj1, proj2)) {
    return amount_overlapping(proj1, proj2);
  } else
    return -1;
}

void body_set_shape(body_t *body, list_t *shape) {
  list_free(body->shape);
  body->shape = shape;
  body->centroid = body_centroid(shape);
}

bool check_in_collision(body_t *body) { return body->in_collision; }

void set_collision_body(body_t *body, bool val, body_t *col_body) {
  body->in_collision = val;
  if (col_body != NULL)
    body->col_body = col_body;
}

body_t *get_collision_body(body_t *body) {
  if (!check_in_collision(body))
    return NULL;
  return body->col_body;
}
