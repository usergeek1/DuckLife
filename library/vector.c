#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const vector_t VEC_ZERO = {0, 0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t sum = {v1.x + v2.x, v1.y + v2.y};
  return sum;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  v2 = vec_negate(v2);
  return vec_add(v1, v2);
}

vector_t vec_negate(vector_t v) { return vec_multiply(-1, v); }

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t product = {scalar * v.x, scalar * v.y};
  return product;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) {
  double zComponent = v1.x * v2.y - v1.y * v2.x;
  return zComponent;
}

vector_t vec_rotate(vector_t v, double angle) {
  vector_t rotation = {v.x * cos(angle) - v.y * sin(angle),
                       v.x * sin(angle) + v.y * cos(angle)};
  return rotation;
}

double vec_distance(vector_t point1, vector_t point2) {
  return sqrt((point1.x - point2.x) * (point1.x - point2.x) +
              (point1.y - point2.y) * (point1.y - point2.y));
}
double vec_magnitude(vector_t point) {
  return sqrt(point.x * point.x + point.y * point.y);
}

vector_t find_unit_vector(vector_t point) {
  double mag = vec_magnitude(point);
  return (vector_t){point.x / mag, point.y / mag};
}

double min(double a, double b) {
  if (a < b)
    return a;
  return b;
}

double max(double a, double b) {
  if (a > b)
    return a;
  return b;
}

int is_overlapping(vector_t point1, vector_t point2) {
  return (max(point1.x, point2.x) <= min(point1.y, point2.y));
}

double amount_overlapping(vector_t point1, vector_t point2) {
  return (min(point1.y, point2.y) - max(point1.x, point2.x));
}
