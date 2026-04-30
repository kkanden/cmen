#define SMOL_IMPLEMENTATION
#include "json.h"
#include "lexer.h"
#include "parser.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define shift(xs, x) (assert(x > 0), (x)--, *(xs)++)

void usage(char *program) {
    fprintf(stderr, "Usage: %s input.json\n", program);
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

    // parser parser = {0};
    parser *parser = parser_init(content);

    json_object object = {0};
    if (!parser_parse(parser, &object)) {
        return_defer(1);
    }

    json_print(object, 0);

defer:
    parser_free(parser);
    json_object_free(&object);

    return result;
}
