#define SMOL_IMPLEMENTATION
#include "lexer.h"

#define shift(xs, x) (assert(x > 0), (x)--, *(xs)++)

void usage(char *program) { fprintf(stderr, "Usage: %s input\n", program); }

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

    lexer lexer = {0};
    lexer_init(&lexer, content);

    token token = lexer_get_token(&lexer);

    while (token.kind != TOKEN_EOF && token.kind != TOKEN_ILLEGAL) {
        string_append0(&token.value);
        line_col lc = calc_line_col(lexer.buffer, token.pos);
        printf("kind: %s, value: `%s`, pos: (%zu, %zu)\n",
               token_lit[token.kind], token.value.items, lc.line, lc.col);
        token = lexer_get_token(&lexer);
    }

    return 0;
}
