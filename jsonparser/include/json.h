#ifndef JSON_H
#define JSON_H
#include "smol.h"

extern char *object_lit[];

typedef enum {
    OBJECT_UNINITIALIZED,
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_BOOLEAN,
    OBJECT_NULL,
    OBJECT_ARRAY,
    OBJECT_MAP,
} json_object_kind;

typedef struct json_object json_object;

typedef struct {
    json_object *items;
    size_t count;
    size_t capacity;
} json_array;

// to be used by stb_ds hashmaps
typedef struct {
    char *key;
    json_object *value;
} json_map;

struct json_object {
    json_object_kind kind;
    union {
        char *string;
        double number;
        bool boolean;
        json_array array;
        json_map *map; // used with stb_ds.h
    } value;
};

void json_print(json_object object, int indent);
void json_dump(json_object object);
void json_object_free(json_object *object);
bool json_from_string(String file_content, json_object *object);

#endif
