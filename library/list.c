
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct list {
  void **data;
  size_t size;
  size_t capacity;
  free_func_t free_func;
} list_t;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list != NULL);
  list->data = malloc((initial_size) * sizeof(void *));
  assert(list->data != NULL);
  list->size = 0;
  list->capacity = initial_size;
  list->free_func = freer;
  return list;
}

void list_free(list_t *list) {
  free_func_t free_f = list->free_func;
  if (free_f != NULL) {
    for (size_t i = 0; i < list_size(list); i++) {
      free_f(list_get(list, i));
    }
  }
  list->size = 0;
  list->capacity = 0;

  free(list->data);
  free(list);
}

size_t list_size(list_t *list) {
  // assert(list->size != NULL);
  return list->size;
}

void *list_get(list_t *list, size_t index) {
  assert(index < list_size(list));
  return list->data[index];
}

void *list_remove(list_t *list, size_t index) {

  assert(list_size(list) >= 1);
  void *listRemoved = list_get(list, index);
  for (size_t i = index; i < list_size(list) - 1; i++) {
    list->data[i] = list->data[i + 1];
  }
  list->data[list->size - 1] = NULL;
  list->size--;
  return listRemoved;
}

void list_add(list_t *list, void *value) {
  assert(list != NULL);

  if (list->size == list->capacity) {
    if (list->capacity == 0)
      list->capacity = 1;
    list->capacity *= 2;
    list->data = realloc(list->data, list->capacity * sizeof(void *));
    assert(list->data != NULL);
  }

  assert(list->data != NULL);
  (list->data)[list->size] = value;
  list->size++;
}

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


free_func_t list_get_freer(list_t *list) { return list->free_func; }
