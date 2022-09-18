
#include "scene.h"
#include "forces.h"
#include "polygon.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
//#include "test_util.h"
#include <math.h>

const int INIT_NUM = 10;

typedef struct scene {
  list_t *bodies;
  list_t *force_creators;
  list_t *force_info;
  list_t *force_freers;
  list_t *force_bodies;
} scene_t;



bool isclose1(double d1, double d2) { return fabs(d1 - d2) < 1e-7; }

bool vec_isclose1(vector_t v1, vector_t v2) {
  return isclose1(v1.x, v2.x) && isclose1(v1.y, v2.y);
}


scene_t *scene_init(void) {

  // Allocate memory for empty scene
  scene_t *scene = malloc(sizeof(scene_t));

  // Set bodies for scene
  list_t *bods = list_init(INIT_NUM, (free_func_t)body_free);
  scene->bodies = bods;

  // Initialize force lists
  scene->force_creators = list_init(INIT_NUM, NULL);
  scene->force_info = list_init(INIT_NUM, NULL);
  scene->force_freers = list_init(INIT_NUM, NULL);
  scene->force_bodies = list_init(INIT_NUM, (free_func_t)list_free);


  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  for (size_t i = 0; i < list_size(scene->force_info); i++) {
    if ((free_func_t)list_get(scene->force_freers, i) != NULL) {
      ((free_func_t)list_get(scene->force_freers, i))(
          list_get(scene->force_info, i));
    }
  }
  list_free(scene->force_info);
  list_free(scene->force_creators);
  list_free(scene->force_freers);

  list_free(scene->force_bodies);

  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body_forces(scene_t *scene, body_t *body) {
  size_t forces_size = list_size(scene->force_creators);
  for (size_t i = 0; i < list_size(scene->force_info); i++) {
  }
  for (size_t i = 0; i < forces_size; i++) {
    // Get current bodies
    list_t *bodies = list_get(scene->force_bodies, i);
    assert(bodies != NULL);
    for (size_t j = 0; j < list_size(bodies); j++) {
      // If the body is the body we want to remove, we want to get rid of the
      // forces related to it
      if (list_get(bodies, j) == body) {
        // Want to remove from all force lists
        free_func_t freer = list_get(scene->force_freers, i);
        list_remove(scene->force_creators, i);
        if (freer == NULL)
          list_remove(scene->force_info, i);
        else
          freer(list_remove(scene->force_info, i));
        list_remove(scene->force_freers, i);
        // Free list of bodies, but don't free both bodies themselves
        list_free(list_remove(scene->force_bodies, i));
        i--;
        forces_size--;
        break;
      }
    }
  }
}
void scene_remove_body2(scene_t *scene, size_t index) {
  body_t *removed_body = scene_get_body(scene, index);

  // set forces associated with body to 0
  scene_remove_body_forces(scene, removed_body);
  // assert(list_size(scene->bodies) > 1);
  list_remove(scene->bodies, index);
  body_free(removed_body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_t *body_to_remove = scene_get_body(scene, index);
  body_remove(body_to_remove);
}

int scene_get_index(scene_t *scene, body_t *body) {
  size_t index;
  list_t *bodies = scene->bodies;
  for (size_t i = 0; i < list_size(bodies); i++)
    if (scene_get_body(scene, i) == body) {
      index = i;
    }
  return index;
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->force_creators); i++) {
    force_creator_t force_cr =
        (force_creator_t)list_get(scene->force_creators, i);
    force_cr(list_get(scene->force_info, i));
  }
  int num_bodies = list_size(scene->bodies);
  for (size_t i = 0; i < num_bodies; i++) {
    body_t *body = (body_t *)list_get(scene->bodies, i);
    assert(body != NULL);
    body_tick(body, dt);
    if (body_is_removed(body)) {
      scene_remove_body2(scene, i);
      i--;
      num_bodies--;
    }
  }
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

void scene_remove_last_force(scene_t *scene){
  size_t ind_to_remove = list_size(scene->force_creators)-1;
  list_remove(scene->force_creators, ind_to_remove);
  list_remove(scene->force_info, ind_to_remove);
  list_remove(scene->force_freers, ind_to_remove);
  list_remove(scene->force_bodies, ind_to_remove);

}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  list_add(scene->force_creators, forcer);
  list_add(scene->force_info, aux);
  list_add(scene->force_freers, freer);
  if (bodies != NULL)
    list_add(scene->force_bodies, bodies);
}
