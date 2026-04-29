#include <math.h>
#define SMOL_IMPLEMENTATION
#include "lexer.h"
#include "parser.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define shift(xs, x) (assert(x > 0), (x)--, *(xs)++)

void usage(char *program) {
    fprintf(stderr, "Usage: %s input.json\n", program);
}

void json_print(json_object object, int indent) {
    static int depth = 0;
    depth++;

    switch (object.kind) {
    case OBJECT_STRING:
        printf("\"%s\"", object.value.string);
        break;
    case OBJECT_NUMBER:
        printf("%.*f",
               (object.value.number == floorf(object.value.number)) ? 0 : 6,
               object.value.number);
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
    }

    if (depth == 1)
        printf("\n");
}

int main(int argc, char **argv) {
    int result = 0;
    char *program = shift(argv, argc);
    if (argc < 1) {
        fprintf(stderr, "No argument provided.\n");
        usage(program);
        exit(1);
    }
    char *input_file = shift(argv, argc);
    String content = {0};

    if (!read_file(input_file, &content))
        exit(1);

    parser parser = {0};
    parser_init(&parser, content);

    json_object object = {0};
    if (!parser_parse(&parser, &object)) {
        return_defer(1);
    }

    json_print(object, 0);

defer:
    parser_free(&parser);

    return result;
}
