#define SMOL_IMPLEMENTATION
#include "lexer.h"
#include "parser.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define shift(xs, x) (assert(x > 0), (x)--, *(xs)++)

void usage(char *program) { fprintf(stderr, "Usage: %s input\n", program); }

void json_print(json_object *object, int indent) {
    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (object->kind) {
    case OBJECT_STRING:
        printf("string: %s\n", object->value.string);
        break;
    case OBJECT_NUMBER:
        printf("number: %f\n", object->value.number);
        break;
    case OBJECT_BOOLEAN:
        printf("boolean: %s\n", object->value.boolean ? "true" : "false");
        break;
    case OBJECT_NULL:
        printf("null\n");
        break;
    case OBJECT_ARRAY:
        printf("array:\n");
        da_foreach(json_object, element, &object->value.array) {
            json_print(element, indent + 2);
        }
        break;
    case OBJECT_MAP:
        printf("map:\n");
        for (int i = 0; i < hmlen(object->value.map); i++) {
            printf("  key: %s\n", object->value.map[i].key);
            printf("  value: ");
            json_print(object->value.map[i].value, indent + 2);
        }
        break;
    case OBJECT_ILLEGAL:
        printf("illegal\n");
        break;
    }
}

int main(int argc, char **argv) {
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
    parser_parse(&parser, &object);
    json_print(&object, 0);

    return 0;
}
