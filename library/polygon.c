#include "color.h"
#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct polygon {
  rgb_color_t col;
  list_t *points;
  vector_t *velocity;
} polygon_t;

void free_polygon(polygon_t *polygon) {
  free(polygon->velocity);
  list_free(polygon->points);
  free(polygon);
}
bool all_at_right(polygon_t *poly, vector_t right_vec) {

  for (size_t i = 0; i < list_size(poly->points); i++) {
    vector_t *cur_point = (vector_t *)list_get(poly->points, i);
    if (cur_point->x < right_vec.x) {
      return false;
    }
  }
  return true;
}

bool one_at_bottom(polygon_t *poly, vector_t bottom_vec) {
  for (size_t i = 0; i < list_size(poly->points); i++) {
    vector_t *cur_point = (vector_t *)list_get(poly->points, i);
    if (cur_point->y <= bottom_vec.y) {
      return true;
    }
  }
  return false;
}

double polygon_area(list_t *polygon) {
  double area = 0.0;
  size_t i;
  int size = list_size(polygon);
  size_t index = size - 1;
  for (i = 0; i < size; i++) {
    vector_t *point1 = (vector_t *)list_get(polygon, index);
    vector_t *point2 = (vector_t *)list_get(polygon, i);
    area += (point1->x + point2->x) * (point1->y - point2->y);
    index = i;
  }
  return fabs(area / 2.0);
}

vector_t polygon_centroid(list_t *polygon) {
  // Find polygon's signed area as described by shoelace formula
  double area = polygon_area(polygon);
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

void polygon_translate(list_t *polygon, vector_t translation) {
  size_t i;
  size_t size = list_size(polygon);
  for (i = 0; i < size; i++) {
    vector_t *point = (vector_t *)list_get(polygon, i);
    point->x += translation.x;
    point->y += translation.y;
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
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
