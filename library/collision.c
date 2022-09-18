
#include "collision.h"
#include "body.h"
#include "color.h"
#include "list.h"
#include "vector.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

#include <time.h>

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t col_info;
  // Find list of axes perpendicular to both polygons
  list_t *edges1 = find_edges(shape1);
  list_t *edges2 = find_edges(shape2);

  // Find perpendicular axes
  list_t *axes1 = find_axes(edges1);
  list_t *axes2 = find_axes(edges2);

  double min_overlap = INFINITY;
  vector_t collision_axis;

  // Check if there are intersections for all of the axes on the list axes1
  // If so, return true
  for (size_t i = 0; i < list_size(axes1); i++) {
    // If no intersection for some axis, return false
    double overlap_check =
        check_overlap_axis(shape1, shape2, list_get(axes1, i));
    if (overlap_check == -1) {
      list_free(edges1);
      list_free(edges2);
      list_free(axes1);
      list_free(axes2);
      list_free(shape1);
      list_free(shape2);
      col_info.collided = false;
      return col_info;
    } else {
      if (overlap_check < min_overlap) {
        min_overlap = overlap_check;
        collision_axis.x = ((vector_t *)list_get(axes1, i))->x;
        collision_axis.y = ((vector_t *)list_get(axes1, i))->y;
      }
    }
  }
  // Check if there are intersections for all of the axes on the list axes2
  // If so, return true
  for (size_t i = 0; i < list_size(axes2); i++) {
    // If no intersection for some axis, return false
    double overlap_check2 =
        check_overlap_axis(shape1, shape2, list_get(axes2, i));
    if (overlap_check2 == -1) {
      list_free(edges1);
      list_free(edges2);
      list_free(axes1);
      list_free(axes2);
      list_free(shape1);
      list_free(shape2);
      col_info.collided = false;
      return col_info;
    } else {
      if (overlap_check2 < min_overlap) {
        min_overlap = overlap_check2;
        collision_axis.x = ((vector_t *)list_get(axes2, i))->x;
        collision_axis.y = ((vector_t *)list_get(axes2, i))->y;
      }
    }
  }
  list_free(edges1);
  list_free(edges2);
  list_free(axes1);
  list_free(axes2);
  list_free(shape1);
  list_free(shape2);
  col_info.collided = true;
  col_info.axis = (vector_t)find_unit_vector(collision_axis);
  return col_info;
}
