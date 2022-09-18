#include "forces.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "list.h"
#include "scene.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL_mixer.h>
#include "sdl_wrapper.h"



const int CONSTANT_DIMENSION = 2;
typedef struct force_info {
  list_t *bodies;
  double constant; // set to either k, gamma, or G (depending on creating
                   // gravity, spring, or drag)
} force_info_t;

typedef struct collision_force_info {
  list_t *bodies;              // bodies involved in colliding
  collision_handler_t handler; // how to handle collision
  vector_t axis;               // axis on which colliding
  bool currently_colliding;
  void *aux;
} collision_force_info_t;

typedef struct buoyancy_force_info{
  list_t *bodies;
  double G;
  double p;
  double water_level;
} buoyancy_force_info_t;

typedef struct duck_gravity_force_info{
  list_t *bodies;
  double G;
  double water_level;
}duck_gravity_force_info_t;

void free_force_info(force_info_t *inf) { free(inf); }

void free_collision_force_info(collision_force_info_t *inf) { free(inf); }

list_t *force_get_bodies(force_info_t *inf) { return inf->bodies; }
force_info_t *gravity_force_init(scene_t *scene, double G, body_t *body1,
                                 body_t *body2) {
  force_info_t *grav_force = malloc(sizeof(force_info_t));
  assert(grav_force != NULL);
  grav_force->bodies = list_init(2, (free_func_t)NULL);
  list_add(grav_force->bodies, body1);
  list_add(grav_force->bodies, body2);
  grav_force->constant = G;
  return grav_force;
}

force_info_t *spring_force_init(scene_t *scene, double k, body_t *body1,
                                body_t *body2) {
  force_info_t *spring_force = malloc(sizeof(force_info_t));
  assert(spring_force != NULL);
  spring_force->bodies = list_init(2, (free_func_t)NULL);
  list_add(spring_force->bodies, body1);
  list_add(spring_force->bodies, body2);
  spring_force->constant = k;
  return spring_force;
}

// Initialize collision info t to  have the bodies + elasticity if it exists (it
// would be stored in aux)
collision_force_info_t *collision_force_init(scene_t *scene, body_t *body1,
                                             body_t *body2, void *aux,
                                             collision_handler_t handler) {
  collision_force_info_t *collision_force =
      malloc(sizeof(collision_force_info_t));
  assert(collision_force != NULL);
  collision_force->bodies = list_init(2, (free_func_t)NULL);
  list_add(collision_force->bodies, body1);
  list_add(collision_force->bodies, body2);
  collision_force->aux = aux;
  collision_force->handler = handler;
  return collision_force;
}

force_info_t *drag_force_init(scene_t *scene, double gamma, body_t *body) {
  force_info_t *drag_force = malloc(sizeof(force_info_t));
  assert(drag_force != NULL);
  drag_force->bodies = list_init(1, (free_func_t)NULL);
  list_add(drag_force->bodies, body);
  drag_force->constant = gamma;
  return drag_force;
}

vector_t calc_spring_force(double k, body_t *body1, body_t *body2) {
  vector_t center_1 = body_get_centroid(body1);
  vector_t center_2 = body_get_centroid(body2);
  double x_coord = k * (center_2.x - center_1.x);
  double y_coord = k * (center_2.y - center_1.y);
  return (vector_t){x_coord, y_coord};
}

vector_t calc_gravity_force(double g, body_t *body1, body_t *body2) {
  double mass_1 = body_get_mass(body1);
  double mass_2 = body_get_mass(body2);
  vector_t center_1 = body_get_centroid(body1);
  vector_t center_2 = body_get_centroid(body2);
  double dist = vec_distance(center_1, center_2);
  if (dist < 5)
    return (vector_t){0, 0};

  double mag_force = (g * mass_1 * mass_2) / (dist * dist);

  double x_coord = mag_force * (center_2.x - center_1.x) / dist;
  double y_coord = mag_force * (center_2.y - center_1.y) / dist;
  //printf("gravity coord %f %f\n", x_coord, y_coord);
  return (vector_t){x_coord, y_coord};
}

vector_t calc_drag_force(body_t *body, double gamma) {
  return vec_multiply(gamma, (body_get_velocity(body)));
}

// Adds gravity forces to bodies
void gravity_func(void *ginf) {
  force_info_t *ginfo = (force_info_t *)ginf;

  body_t *body_1 = list_get(ginfo->bodies, 0);
  body_t *body_2 = list_get(ginfo->bodies, 1);
  vector_t force = calc_gravity_force(ginfo->constant, body_1, body_2);

  vector_t force_p = {0, 0};
  force_p.x = force.x;
  force_p.y = force.y;
  body_add_force(body_1, force_p);

  vector_t force_pn = {0, 0};
  force_pn.x = -force.x;
  force_pn.y = -force.y;
  body_add_force(body_2, force_pn);
}

// Add spring forces to bodies
void spring_func(void *sinf) {
  force_info_t *sinfo = (force_info_t *)sinf;

  body_t *body_1 = list_get(sinfo->bodies, 0);
  body_t *body_2 = list_get(sinfo->bodies, 1);
  vector_t force = calc_spring_force(sinfo->constant, body_1, body_2);

  vector_t force_p = {0, 0};
  force_p.x = force.x;
  force_p.y = force.y;
  body_add_force(body_1, force_p);

  vector_t force_pn = {0, 0};
  force_pn.x = -force.x;
  force_pn.y = -force.y;
  body_add_force(body_2, force_pn);
}

// Add drag forces to bodies
void drag_func(void *dinf) {
  force_info_t *dinfo = (force_info_t *)dinf;

  body_t *body_1 = list_get(dinfo->bodies, 0);
  vector_t force = calc_drag_force(body_1, dinfo->constant);

  // Force points opposite
  vector_t force_pn = {0, 0};
  force_pn.x = -force.x;
  force_pn.y = -force.y;
  body_add_force(body_1, force_pn);
}

// Collision func checks for if there is a collision and if so, applies
// collision handler
void collision_func(void *cinf) {
  collision_force_info_t *cinfo = (collision_force_info_t *)cinf;
  body_t *body1 = list_get(cinfo->bodies, 0);
  body_t *body2 = list_get(cinfo->bodies, 1);
  collision_handler_t handler = cinfo->handler;
  void *col_aux = cinfo->aux;
  collision_info_t col_info =
      find_collision(body_get_shape(body1), body_get_shape(body2));

  // If already in collision,  don't apply more impulses
  // if it is currenly colliding and it wasn't before
  // apply the first impulse

  if (col_info.collided && !cinfo->currently_colliding) {
    cinfo->currently_colliding = true;
    vector_t axis_of_collision = col_info.axis;
    set_collision_body(body1, true, body2);
    set_collision_body(body2, true, body1);
    cinfo->currently_colliding = true;
    // Apply collision handler
    handler(body1, body2, axis_of_collision, col_aux);
  }
  // if it is not currently colliding but it used to collide
  // the next tick should tell code that it wasnt collidng in thee last tick
  if (!col_info.collided && cinfo->currently_colliding) {
    cinfo->currently_colliding = false;
    return;
  }

  // if it is currently collding and it used to
  // change nothing; do not apply more impulse
  if (col_info.collided && cinfo->currently_colliding) {
    return;
  }

  // if it is currently not colliding and it didnt use to collide
  if (!col_info.collided && cinfo->currently_colliding) {
    return;
  }
}

// Buoynacy force init function
buoyancy_force_info_t *buoyancy_force_init(scene_t *scene, double G, body_t *body1, double p, double water_level) {
  buoyancy_force_info_t *buoyancy_force = malloc(sizeof(buoyancy_force_info_t));
  assert(buoyancy_force != NULL);
  // Initialize fields of the buoyancy_force_info_t
  buoyancy_force->bodies = list_init(1, (free_func_t)NULL);
  list_add(buoyancy_force->bodies, body1);
  buoyancy_force->G = G;
  buoyancy_force->p = p;
  buoyancy_force->water_level = water_level;
  return buoyancy_force;
}

vector_t calc_buoyancy_force(double g, body_t *body1, double p, double water_level) {
  // body1 is the duck body
  vector_t center = body_get_centroid(body1);
  // If the duck position is above the water level return weird force (-inf, -inf)
  if(center.y >= water_level){
    //printf("ABOVE WATER LEVEl %f\n", center.y);
    return (vector_t){-INFINITY, -INFINITY};
  }
  list_t *body_shape = body_get_shape(body1);
  double area_submerged = body_area(body_shape);
  double volume_param = CONSTANT_DIMENSION * area_submerged;
  double mag_force = volume_param * p * g;
  double y_coord = mag_force * center.y;  

  return (vector_t){center.x, y_coord};
}

void buoyancy_func(void *ginf) {
  buoyancy_force_info_t *ginfo = (buoyancy_force_info_t *)ginf;
  body_t *body_1 = list_get(ginfo->bodies, 0);
  vector_t force = calc_buoyancy_force(ginfo->G, body_1, ginfo->p,ginfo->water_level);
  if(!(force.x == -INFINITY && force.y == -INFINITY)) body_add_force(body_1, force);
  //printf("%f\n", body_get_mass(body_1));
}


void create_buoyancy(scene_t *scene, double G, body_t *body1, double p, double water_level) {
  buoyancy_force_info_t *binfo = buoyancy_force_init(scene, G, body1, p, water_level);
  scene_add_bodies_force_creator(scene, buoyancy_func, (void *)binfo,
                                 binfo->bodies, (free_func_t)free_force_info);
}


// Duck gravity force force init function
duck_gravity_force_info_t *duck_gravity_force_init(scene_t *scene, double G, double water_level, body_t *body1, body_t *body2) {
  duck_gravity_force_info_t *duck_gravity_force = malloc(sizeof(duck_gravity_force_info_t));
  assert(duck_gravity_force != NULL);
  // Initialize fields of the duck_gravity_force_t
  duck_gravity_force->bodies = list_init(2, (free_func_t)NULL);
  list_add(duck_gravity_force->bodies, body1);
  list_add(duck_gravity_force->bodies, body2);
  duck_gravity_force->G = G;
  duck_gravity_force->water_level = water_level;
  return duck_gravity_force;
}

void duck_gravity_func(void *dinf){
  duck_gravity_force_info_t *dinfo = (duck_gravity_force_info_t *)dinf;
  body_t *body_1 = list_get(dinfo->bodies, 0);
  body_t *body_2 = list_get(dinfo->bodies, 1);
  vector_t force = calc_gravity_force(dinfo->G, body_1, body_2);
  double water_level = dinfo->water_level;
  // Duck body centroid
  vector_t duck_centroid = body_get_centroid(body_1);

  // Add gravity force to duck/earth if not below water level
  if(duck_centroid.y > water_level) {
    body_add_force(body_1, force);
    body_add_force(body_2, vec_negate(force));
  }
}
void create_duck_gravity(scene_t *scene, double G, double water_level, body_t *body1, body_t *body2){
  duck_gravity_force_info_t *dinfo =  duck_gravity_force_init(scene, G, water_level, body1, body2);
  scene_add_bodies_force_creator(scene, duck_gravity_func, (void *)dinfo,
                                 dinfo->bodies, (free_func_t)free_force_info);
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  force_info_t *finfo = gravity_force_init(scene, G, body1, body2);

  scene_add_bodies_force_creator(scene, gravity_func, (void *)finfo,
                                 finfo->bodies, (free_func_t)free_force_info);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  force_info_t *finfo = spring_force_init(scene, k, body1, body2);
  scene_add_bodies_force_creator(scene, spring_func, (void *)finfo,
                                 finfo->bodies, (free_func_t)free_force_info);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  force_info_t *finfo = drag_force_init(scene, gamma, body);
  scene_add_bodies_force_creator(scene, drag_func, (void *)finfo, finfo->bodies,
                                 (free_func_t)free_force_info);
}

/**
 * Handles two bodies destructively colliding
 **/
void destructive_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                   void *aux) {
  body_remove(body1);
  body_remove(body2);
}

// Applies impulses
void apply_impulse(body_t *body1, body_t *body2, vector_t axis,
                   double elasticity) {

  vector_t vel1 = body_get_velocity(body1);
  vector_t vel2 = body_get_velocity(body2);
  double mass1 = body_get_mass(body1);
  double mass2 = body_get_mass(body2);
  double vel_dif = -vec_dot(vel1, axis) + vec_dot(vel2, axis);

  // Get reduced mass (dependent on whether a mass is INFINITY)
  double red_mass;
  if (mass1 == INFINITY) {
    red_mass = mass2;
  } else if (mass2 == INFINITY) {
    red_mass = mass1;
  } else {
    red_mass = (mass1 * mass2) / (mass1 + mass2);
  }

  // Calculate + add impulses to both bodies
  vector_t impulse = vec_multiply(red_mass * (elasticity + 1) * vel_dif, axis);
  body_add_impulse(body1, (impulse));
  body_add_impulse(body2, vec_negate(impulse));
}



/**
 * Handles two bodies nondstructively colliding (colliding with physics)
 * In this case, aux will contain a double that represents elasticity
 *
 */
void physics_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux) {
  // Auxillary function should contain elasticity
  double elasticity = *((double *)aux);
  apply_impulse(body1, body2, axis, elasticity);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_force_info_t *col_info =
      collision_force_init(scene, body1, body2, aux, handler);

  scene_add_bodies_force_creator(scene, (force_creator_t)collision_func,
                                 (void *)col_info, col_info->bodies, freer);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {

  play_coin();
  create_collision(scene, body1, body2,
                   (collision_handler_t)destructive_collision_handler,
                   (void *)scene, NULL);
  // Return list of bodies associated with destructive collision
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  double *elas_aux = malloc(sizeof(double));
  *elas_aux = elasticity;
  // Call create_collision with all of the info (auxillary contains elasticity)
  create_collision(scene, body1, body2,
                   (collision_handler_t)physics_collision_handler,
                   (void *)elas_aux, (free_func_t)free);
}
