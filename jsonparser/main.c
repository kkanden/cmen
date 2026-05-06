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

    /* the content string is internally turned into a smol.h's `String` object
       whose memory is then owned by the parser object which is freed right
       after parsing. the input string ownership is left untouched. */
    if (!json_from_string(content.items, &object)) {
        return_defer(1);
    }

    json_print(object, 0);

    if (object.kind == OBJECT_ARRAY) {
        printf("it's an array\n");
        json_print(object.value.array.items[0], 0);
    } else {
        printf("not an array\n");
        printf("object kind: %s\n", object_lit[object.kind]);
    }

defer:
    json_object_free(&object);

    return result;
}
