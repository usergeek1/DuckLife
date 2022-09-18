#include "color.h"
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

rgb_color_t rand_color() {
  rgb_color_t new_col;
  new_col.r = (double)rand() / (double)RAND_MAX;
  new_col.g = (double)rand() / (double)RAND_MAX;
  new_col.b = (double)rand() / (double)RAND_MAX;
  return new_col;
}

rgb_color_t rand_purp_color() {
  rgb_color_t rand_col = rand_color();
  return (rgb_color_t){(rand_col.r + 0) / 2, (rand_col.g + 0) / 2,
                       (rand_col.b + 1) / 2};
}

rgb_color_t white_color() {
  rgb_color_t new_col;
  new_col.r = 1.0;
  new_col.g = 1.0;
  new_col.b = 1.0;
  return new_col;
}