#include "lexer.h"

typedef enum {
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_BOOLEAN,
    OBJECT_NULL,
    OBJECT_ARRAY,
    OBJECT_MAP,
    OBJECT_ILLEGAL
} json_object_kind;

extern char *object_lit[];

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

typedef struct {
    lexer lexer;
    token current;
} parser;

void parser_init(parser *parser, String content);
void parser_free(parser *parser);
bool parser_parse(parser *parser, json_object *object);
