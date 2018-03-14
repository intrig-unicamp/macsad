#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

static void vector_init_elements(vector_t* vector, int from, int to) {
    int i;
    for(i = from; i < to; i++) {
//TODO allocate from huge page
        //vector->data[i] = rte_malloc_socket("counter_array_element", vector->data_size, 0, vector->socketid);
        vector->data[i] = rte_malloc_socket(vector->data_size);
        vector->value_init(vector->data[i]);
    }
}

void vector_init(vector_t *vector, int size, int capacity, int data_size, void (*value_init)(void *), int socketid) {
    vector->size = size;
    vector->socketid = socketid;
    vector->data_size = data_size;
    vector->capacity = capacity;
    vector->value_init = value_init;
//TODO allocate from huge page
 //   vector->data = rte_malloc_socket("counter_array", sizeof(void*) * vector->capacity, 0, socketid);
    vector->data = malloc(sizeof(void*) * vector->capacity);
    vector_init_elements(vector, 0, size);
}

void vector_append(vector_t *vector, void* value) {
    vector_double_capacity_if_full(vector);
    vector->data[vector->size++] = value;
}

void* vector_get(vector_t *vector, int index) {
    if (index >= vector->size || index < 0) {
        printf("Index %d out of bounds for vector of size %d\n", index, vector->size);
        return NULL;
    }
    return vector->data[index];
}

void vector_set(vector_t *vector, int index, void* value) {
    while (index >= vector->size) {
        vector_append(vector, NULL);
    }
    vector->data[index] = value;
}

void vector_double_capacity_if_full(vector_t *vector) {
    if (vector->size >= vector->capacity) {
        int i = vector->capacity;
        vector->capacity *= 2;
        vector->data = rte_realloc(vector->data, vector->data_size * vector->capacity, 0);
        vector_init_elements(vector, i, vector->capacity);
    }
}

void vector_free(vector_t *vector) {
    free(vector->data);
}
