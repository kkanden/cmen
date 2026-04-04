#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define VEC_INIT_CAP 256

typedef struct {
    u64 size;
    u64 capacity;
} vecheader;

#define vec_get_header(vec) (vecheader *)(vec) - 1

#define vec_len(vec) (vec_get_header(vec))->size

// takes pointer to the start of the vector and value to append
// reallocs the memory if needed and updates the given pointer
#define vec_append(vec, val)                                                   \
    do {                                                                       \
        vecheader *header = vec_get_header(*(vec));                            \
        if (header->capacity < header->size + 1) {                             \
            header->capacity *= 2;                                             \
            header = realloc(header, sizeof(vecheader) +                       \
                                         sizeof(**(vec)) * header->capacity);  \
            *(vec) = (typeof(*(vec)))(header + 1);                             \
        }                                                                      \
        (*(vec))[header->size++] = val;                                        \
    } while (0)

#define vec_free(vec) free(vec_get_header(vec))

// return void pointer to the vector
void *vec_new(size_t size) {
    vecheader *header = malloc(sizeof(vecheader) + size * VEC_INIT_CAP);
    header->size = 0;
    header->capacity = VEC_INIT_CAP;
    return (void *)(header + 1);
}

int main(void) {
    float *vec = vec_new(sizeof(float));
    vec_append(&vec, 6);
    vec_append(&vec, 7);
    vec_append(&vec, 8);
    for (i64 i = 0; i < 1000; i++) {
        vec_append(&vec, i);
    }

    for (u64 i = 0; i < vec_len(vec); i++) {
        printf("%f\n", vec[i]);
    }
    vec_free(vec);

    return 0;
}
