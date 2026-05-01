#include "json.h"
#include <unistd.h>

#define shift(xs, x) (assert(x > 0), (x)--, *(xs)++)

void usage(char *program) {
    fprintf(stderr, "Usage: %s input.json\n", program);
}

int main(int argc, char **argv) {
    int result = 0;
    char *program = shift(argv, argc);
    String content = {0};

    if (!isatty(STDIN_FILENO)) {
        if (!read_stdin(&content))
            exit(1);
    } else if (argc >= 1) {
        char *input_file = shift(argv, argc);
        if (!read_file(input_file, &content))
            exit(1);

    } else {
        fprintf(stderr, "No argument provided.\n");
        usage(program);
        exit(1);
    }

    json_object object = {0};

    /* memory of content is owned by the lexer which is freed by parser_free in
       json_from_string */
    if (!json_from_string(content, &object)) {
        return_defer(1);
    }

    json_print(object, 0);

defer:
    json_object_free(&object);

    return result;
}
