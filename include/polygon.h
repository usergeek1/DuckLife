#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <stdbool.h>

/**
 * @brief Struct that stores a polygon (fields: color, points, velocity)
 *
 */
typedef struct polygon polygon_t;

/**
 * @brief Frees polygon struct instance
 *
 */
void free_polygon(polygon_t *polygon);

/**
 * @brief Checks if all points in polygon are at right
 *
 * @param poly
 * @param right_vec
 * @return true
 * @return false
 */
bool all_at_right(polygon_t *poly, vector_t right_vec);

/**
 * @brief Checks if one point in polygon is at bottom
 *
 * @param poly
 * @param bottom_vec
 * @return true
 * @return false
 */
bool one_at_bottom(polygon_t *poly, vector_t bottom_vec);

/**
 * Computes the area of a polygon.
 * See https://en.wikipedia.org/wiki/Shoelace_formula#Statement.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the area of the polygon
 */
double polygon_area(list_t *polygon);

/**
 * Computes the center of mass of a polygon.
 * See https://en.wikipedia.org/wiki/Centroid#Of_a_polygon.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the centroid of the polygon
 */
vector_t polygon_centroid(list_t *polygon);

/**
 * Translates all vertices in a polygon by a given vector.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param translation the vector to add to each vertex's position
 */
void polygon_translate(list_t *polygon, vector_t translation);

/**
 * Rotates vertices in a polygon by a given angle about a given point.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param angle the angle to rotate the polygon, in radians.
 * A positive angle means counterclockwise.
 * @param point the point to rotate around
 */
void polygon_rotate(list_t *polygon, double angle, vector_t point);

#endif // #ifndef __POLYGON_H__
