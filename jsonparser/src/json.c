#include "parser.h"
#include "smol.h"
#include "stb_ds.h"

#include <float.h>

char *object_lit[] = {"UNINITIALIZED", "STRING", "NUMBER", "BOOLEAN",
                      "NULL",          "ARRAY",  "MAP"};

void json_print(json_object object, int indent) {
    static int depth = 0;
    depth++;

    switch (object.kind) {
    case OBJECT_STRING:
        printf("\"%s\"", object.value.string);
        break;
    case OBJECT_NUMBER:
        printf("%g", object.value.number);
        break;
    case OBJECT_BOOLEAN:
        printf("%s", object.value.boolean ? "true" : "false");
        break;
    case OBJECT_NULL:
        printf("null");
        break;
    case OBJECT_MAP:
        printf("{\n");
        for (int i = 0; i < hmlen(object.value.map); i++) {
            for (int i = 0; i < indent; i++)
                printf(" ");

            printf("  \"%s\": ", object.value.map[i].key);

            json_print(*object.value.map[i].value, indent + 2);
            depth--;

            if (i < hmlen(object.value.map) - 1)
                printf(",\n");
        }
        printf("\n");
        for (int i = 0; i < indent; i++)
            printf(" ");
        printf("}");
        break;
    case OBJECT_ARRAY:
        printf("[");
        for (size_t i = 0; i < object.value.array.count; i++) {
            json_print(object.value.array.items[i], 0);
            depth--;
            if (i < object.value.array.count - 1)
                printf(", ");
        }
        printf("]");
        break;
    case OBJECT_UNINITIALIZED:
        break;
    }

    if (depth == 1)
        printf("\n");
}

void json_dump(json_object object) { json_print(object, 0); }

void json_object_free(json_object *object) {
    switch (object->kind) {
    case OBJECT_ARRAY:
        // free each object in the array
        for (size_t i = 0; i < object->value.array.count; i++) {
            json_object_free(&object->value.array.items[i]);
        }
        // free the array itself
        da_free(&object->value.array);
        break;
    case OBJECT_MAP:
        for (int i = 0; i < hmlen(object->value.map); i++) {
            json_object_free(object->value.map[i].value);

            // key is strdup'd so we own the memory
            free(object->value.map[i].key);

            /* value is malloced when parsed, so we need to free the pointer
             * itself */
            free(object->value.map[i].value);
        }
        hmfree(object->value.map);
        break;
    case OBJECT_STRING:
        free(object->value.string);
        object->value.string = NULL;
        break;
    default: // other objects are not heap-allocated or object is not
             // initialized
        break;
    }
}

// `file_content` does not necessarily have to be nul-terminated
bool json_from_string(char *file_content, json_object *object) {
    String content = {0};
    string_append_cstr(&content, file_content);
    bool result = true;
    parser *parser = parser_init(content);

    if (!parser_parse(parser, object)) {
        return_defer(false);
    }

defer:
    parser_free(parser);
    if (!result)
        json_object_free(object);

    return result;
}
