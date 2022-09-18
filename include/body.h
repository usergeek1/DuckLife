#ifndef __BODY_H__
#define __BODY_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <stdbool.h>

/**
 * A rigid body constrained to the plane.
 * Implemented as a polygon with uniform density.
 * Bodies can accumulate forces and impulses during each tick.
 * Angular physics (i.e. torques) are not currently implemented.
 */
typedef struct body body_t;

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the mass is positive and that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving)
 * @param color the color of the body, used to draw it on the screen
 * @return a pointer to the newly allocated body
 */
body_t *body_init(list_t *shape, double mass, rgb_color_t color);

body_t *body_init_sprite(list_t *shape, double mass, rgb_color_t color, char *path);

/**
 * @brief Initializes body with type of body additional info
 *
 * @param shape
 * @param mass
 * @param color
 * @param type_of_bod
 * @return body_t*
 */
body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *type_of_bod);

body_t *body_init_with_info_with_image(list_t *shape, double mass, rgb_color_t color, char *image_path, void *type_of_bod);
/**
 * Releases the memory allocated for a body.
 *
 * @param body a pointer to a body returned from body_init()
 */
void body_free(body_t *body);

char *body_get_image_path(body_t *bod);

/**
 * Gets the current shape of a body.
 * Returns a newly allocated vector list, which must be list_free()d.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the polygon describing the body's current position
 */
list_t *body_get_shape(body_t *body);

/**
 * Gets the current center of mass of a body.
 * While this could be calculated with polygon_centroid(), that becomes too slow
 * when this function is called thousands of times every tick.
 * Instead, the body should store its current centroid.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's center of mass
 */
vector_t body_get_centroid(body_t *body);

/**
 * Gets the current velocity of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's velocity vector
 */
vector_t body_get_velocity(body_t *body);

/**
 * Gets the mass of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the mass passed to body_init(), which must be greater than 0
 */
double body_get_mass(body_t *body);

/**
 * Gets the display color of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the color passed to body_init(), as an (R, G, B) tuple
 */
rgb_color_t body_get_color(body_t *body);

/**
 * Translates a body to a new position.
 * The position is specified by the position of the body's center of mass.
 *
 * @param body a pointer to a body returned from body_init()
 * @param x the body's new centroid
 */
void body_set_centroid(body_t *body, vector_t x);

/**
 * Changes a body's velocity (the time-derivative of its position).
 *
 * @param body a pointer to a body returned from body_init()
 * @param v the body's new velocity
 */
void body_set_velocity(body_t *body, vector_t v);

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */
void body_set_rotation(body_t *body, double angle);

double body_get_x_velo(body_t *body);

double body_get_y_velo(body_t *body);

void body_set_x_velo(body_t *body, double vx);

void body_set_y_velo(body_t *body, double vy);

/**
 * Applies a force to a body over the current tick.
 * If multiple forces are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param force the force vector to apply
 */
void body_add_force(body_t *body, vector_t force);

/**
 * Applies an impulse to a body.
 * An impulse causes an instantaneous change in velocity,
 * which is useful for modeling collisions.
 * If multiple impulses are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param impulse the impulse vector to apply
 */
void body_add_impulse(body_t *body, vector_t impulse);

/**
 * Updates the body after a given time interval has elapsed.
 * Sets acceleration and velocity according to the forces and impulses
 * applied to the body during the tick.
 * The body should be translated at the *average* of the velocities before
 * and after the tick.
 * Resets the forces and impulses accumulated on the body.
 *
 * @param body the body to tick
 * @param dt the number of seconds elapsed since the last tick
 */
void body_tick(body_t *body, double dt);

/**
 * @brief Returns the list of the points of the body
 *
 * @param body the body to get the list of points of
 * @return list_t*
 */

list_t *get_body_points(body_t *body);

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */

void body_set_rotation_relative(body_t *body, double angle);

/**
 * Computes the area of a body.
 * See https://en.wikipedia.org/wiki/Shoelace_formula#Statement.
 *
 * @param polygon the list of vertices that make up the body,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the area of the body
 */
double body_area(list_t *polygon);

/**
 * Computes the center of mass of the body.
 * See https://en.wikipedia.org/wiki/Centroid#Of_a_polygon.
 *
 * @param polygon the list of vertices that make up the body.
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the centroid of the body
 */

vector_t body_centroid(list_t *polygon);

/**
 * Translates all vertices in a body by a given vector.
 * Note: mutates the original body.
 *
 * @param polygon the list of vertices that make up the body
 * @param translation the vector to add to each vertex's position
 */

void body_translate(list_t *polygon, vector_t translation);

/**
 * Rotates vertices in a body by a given angle about a given point.
 * Note: mutates the original body.
 *
 * @param polygon the list of vertices that make up the body
 * @param angle the angle to rotate the body, in radians.
 * A positive angle means counterclockwise.
 * @param point the point to rotate around
 */

void body_rotate(list_t *polygon, double angle, vector_t point);

/**
 * @brief Returns the mass of the body
 *
 * @param body the pointer to the body
 * @return double the mass of the body
 */
double body_get_mass(body_t *body);

/**
 * Marks a body for removal--future calls to body_is_removed() will return true.
 * Does not free the body.
 * If the body is already marked for removal, does nothing.
 *
 * @param body the body to mark for removal
 */
void body_remove(body_t *body);

/**
 * Returns whether a body has been marked for removal.
 * This function returns false until body_remove() is called on the body,
 * and returns true afterwards.
 *
 * @param body the body to check
 * @return whether body_remove() has been called on the body
 */
bool body_is_removed(body_t *body);

/**
 * @brief Finds the edges associated with the shape of a body
 *
 * @param list_t*
 * @return list_t*
 */
list_t *find_edges(list_t *sh);

/**
 * @brief Given a list of edges, returns the corresponding axes
 * (i.e., perpendicular lines)
 *
 * @param edges
 * @return list_t*
 */
list_t *find_axes(list_t *edges);

/**
 * @brief Given a body and an axis, find min/max projection points on axis
 *
 * @param body
 * @param axis
 * @return vector_t - contains the min and max values of projection
 */
vector_t body_proj_on_axis(list_t *sh, vector_t *axis);

/**
 * @brief Check if the projections of the bodies on the axis intersect
 * (collision on that axis)
 *
 * @param shape1
 * @param shape2
 * @param axis
 * @return non -1 - the amount of overlap on the axis
 * @return -1 - no collision on axis
 */
double check_overlap_axis(list_t *shape1, list_t *shape2, vector_t *axis);
/**
 * Gets the information associated with a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the info passed to body_init()
 */
void *body_get_info(body_t *body);

/**
 * @brief Returns whether a body is in collision
 *
 * @param body
 * @return true
 * @return false
 */
bool check_in_collision(body_t *body);

/**
 * @brief Mark that a body is in collision
 *
 * @param body
 */
void set_collision_body(body_t *body, bool val, body_t *col_body);

/**
 * @brief Set Color of body
 *
 * @param body
 * @param col
 * @return rgb_color_t
 */
void body_set_color(body_t *body, rgb_color_t col);

/**
 * @brief Set shape of body
 *
 */
void body_set_shape(body_t *body, list_t *shape);

/**
 * @brief Get the collision body of the body passed in
 *
 * @param body
 * @return body_t*
 */
body_t *get_collision_body(body_t *body);

#endif // #ifndef __BODY_H__
