#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// largely based on tsoding's nob.h

/* UTILS */
#define return_defer(value)                                                    \
    do {                                                                       \
        result = (value);                                                      \
        goto defer;                                                            \
    } while (0)

#define TODO(message, ...)                                                     \
    do {                                                                       \
        fprintf(stderr, "%s:%d: TODO: " message "\n", __FILE__,                \
                __LINE__ __VA_OPT__(, ) __VA_ARGS__);                          \
        abort();                                                               \
    } while (0)

typedef enum { INFO, WARN, ERROR } log_level;

void LOG(log_level level, const char *fmt, ...);

#define return_defer(value)                                                    \
    do {                                                                       \
        result = (value);                                                      \
        goto defer;                                                            \
    } while (0)
/* UTILS END */

/* ARRAYS */
#define DA_INIT_CAP 256

#define da_reserve(da, expected_capacity)                                      \
    do {                                                                       \
        if ((expected_capacity) > (da)->capacity) {                            \
            if ((da)->capacity == 0) {                                         \
                (da)->capacity = DA_INIT_CAP;                                  \
            }                                                                  \
            while ((expected_capacity) > (da)->capacity) {                     \
                (da)->capacity *= 2;                                           \
            }                                                                  \
            (da)->items =                                                      \
                realloc((da)->items, (da)->capacity * sizeof(*(da)->items));   \
            assert((da)->items != NULL);                                       \
        }                                                                      \
    } while (0)

// Append an item to a dynamic array
#define da_append(da, item)                                                    \
    do {                                                                       \
        da_reserve((da), (da)->count + 1);                                     \
        (da)->items[(da)->count++] = (item);                                   \
    } while (0)

#define da_free(da)                                                            \
    do {                                                                       \
        if ((da)->items != NULL)                                               \
            free((da)->items);                                                 \
        (da)->items = NULL;                                                    \
    } while (0)

// Append several items to a dynamic array
#define da_append_many(da, new_items, new_items_count)                         \
    do {                                                                       \
        da_reserve((da), (da)->count + (new_items_count));                     \
        memcpy((da)->items + (da)->count, (new_items),                         \
               (new_items_count) * sizeof(*(da)->items));                      \
        (da)->count += (new_items_count);                                      \
    } while (0)

#define da_resize(da, new_size)                                                \
    do {                                                                       \
        da_reserve((da), new_size);                                            \
        (da)->count = (new_size);                                              \
    } while (0)

#define da_pop(da) (da)->items[(assert((da)->count > 0), --(da)->count)]
#define da_first(da) (da)->items[(assert((da)->count > 0), 0)]
#define da_last(da) (da)->items[(assert((da)->count > 0), (da)->count - 1)]
#define da_remove_unordered(da, i)                                             \
    do {                                                                       \
        size_t j = (i);                                                        \
        assert(j < (da)->count);                                               \
        (da)->items[j] = (da)->items[--(da)->count];                           \
    } while (0)

// da_foreach(int, x, &xs) {
//     // `x` here is a pointer to the current element. You can get its index by
//     taking a difference
//     // between `x` and the start of the array which is `x.items`.
//     size_t index = x - xs.items;
//     log(INFO, "%zu: %d", index, *x);
// }
#define da_foreach(Type, it, da)                                               \
    for (Type *it = (da)->items; it < (da)->items + (da)->count; ++it)

/* ARRAYS END */

/* STRINGS */
typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} String;

#define string_append0(s) da_append((s), '\0')
#define string_append_cstr(s, s_new) da_append_many((s), (s_new), strlen(s_new))
#define string_cat(s, s_new) da_append_many((s), (s_new)->items, (s_new)->count)
#define string_print(s)                                                        \
    do {                                                                       \
        for (size_t i = 0; i < (s).count; i++)                                 \
            printf("%c", (s).items[i]);                                        \
    } while (0)
/* STRINGS END */

bool read_file(const char *filename, String *s);

#ifdef SMOL_IMPLEMENTATION
#include <errno.h>

void LOG(log_level level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char *prefix = "";
    switch (level) {
    case INFO:
        prefix = "[INFO] ";
        break;
    case WARN:
        prefix = "[WARNING] ";
        break;
    case ERROR:
        prefix = "[ERROR] ";
        break;
    }
    fprintf(stderr, "%s", prefix);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

bool read_file(const char *filename, String *s) {
    bool result = true;
    FILE *file = fopen(filename, "rb");
    size_t new_count = 0;
    long length = 0;
    if (!file)
        return_defer(false);
    if (fseek(file, 0, SEEK_END) < 0)
        return_defer(false);

    length = ftell(file);
    if (length < 0)
        return_defer(false);
    if (fseek(file, 0, SEEK_SET) < 0)
        return_defer(false);

    new_count = s->count + length;
    if (new_count > s->capacity) {
        s->items = realloc(s->items, new_count);
        assert(s->items);
        s->capacity = new_count;
    }

    fread(s->items + s->count, length, 1, file);
    if (ferror(file)) {
        return_defer(false);
    }
    s->count = new_count;

defer:
    if (!result)
        fprintf(stderr, "Failed reading file %s: %s\n", filename,
                strerror(errno));
    if (file)
        fclose(file);
    return result;
}
#endif
