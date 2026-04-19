#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define STB_DS_IMPLEMENTATION
#include "array.h"
#include "stb_ds.h"

#define LOG 1

typedef unsigned char uchar;
typedef uint16_t token_t;

#define shift(xs, x) (assert(x > 0), (x)--, *(xs)++)

typedef struct {
    token_t pair[2];
} Pair;

typedef struct {
    Pair key;
    size_t value;
} Freq;

typedef struct {
    token_t key;
    Pair value;
} TokenMap;

typedef struct {
    size_t count;
    size_t capacity;
    Freq *items;
} Freqs;

typedef struct {
    size_t count;
    size_t capacity;
    token_t *items;
} Text;

Text pair_replace(Text text, Pair pair, token_t token) {
    Text new_text = {0};
    for (size_t i = 0; i < text.count; i++) {
        // if we got to the last item, then it was not a replacable pair
        // just insert it and break
        if (i == text.count - 1) {
            da_append(&new_text, text.items[i]);
            break;
        }
        token_t a = text.items[i];
        token_t b = text.items[i + 1];
        if (a == pair.pair[0] && b == pair.pair[1]) {
            da_append(&new_text, token);
            i++;
        } else {
            da_append(&new_text, a);
        }
    }
    return new_text;
}

Text token_replace(Text text, token_t token, Pair pair) {
    Text new_text = {0};
    for (size_t i = 0; i < text.count; i++) {
        uint16_t c = text.items[i];
        if (c == token) {
            da_append(&new_text, pair.pair[0]);
            da_append(&new_text, pair.pair[1]);
        } else {
            da_append(&new_text, c);
        }
    }
    return new_text;
}

// input file is read byte-by-byte
Text readfile(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        exit(1);
    }

    Text text = {0};
    uchar byte;
    while (fread(&byte, 1, 1, file) == 1) {
        da_append(&text, (token_t)byte);
    }
    fclose(file);
    return text;
}

void writefile(char *filename, Text text) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror(filename);
        exit(1);
    }
    da_foreach(token_t, x, &text) { fwrite(x, 1, sizeof(uint16_t), file); }
    fclose(file);
};

void writetokens(char *filename, TokenMap *tokens) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror(filename);
        exit(1);
    }
    for (int i = 0; i < hmlen(tokens); i++) {
        fwrite(&tokens[i].key, 1, sizeof(token_t), file);
        fwrite(&tokens[i].value.pair, 1, 2 * sizeof(token_t), file);
    }
}

// bpe file is read using token_t
Text readbpe(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        exit(1);
    }

    Text text = {0};
    token_t token;
    while (fread(&token, sizeof(token_t), 1, file) == 1) {
        da_append(&text, token);
    }
    return text;
}

TokenMap *readtokens(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        exit(1);
    }
    TokenMap *tokens = NULL;

    token_t line[3] = {0};
    while (fread(&line[0], sizeof(token_t), 3, file) == 3) {
        token_t key = line[0];
        Pair pair = {{line[1], line[2]}};
        hmput(tokens, key, pair);
    }
    fclose(file);
    return tokens;
}

Text encode(Text text, TokenMap **tokens) {
    for (token_t c = 256; c < UINT16_MAX; c++) {
        Freq *hm = NULL;
        for (size_t i = 0; i < text.count - 1; i++) {
            Pair pair = {{text.items[i], text.items[i + 1]}};
            int index = hmgeti(hm, pair);
            if (index == -1)
                hmput(hm, pair, 1);
            else
                hm[index].value++;
        }

        // get most frequent pair
        Freq most_frequent = hm[0];
        for (int i = 1; i < hmlen(hm); i++) {
            if (hm[i].value > most_frequent.value) {
                most_frequent = hm[i];
            }
        }
        // most frequent pair appears once -- nothing left to replace
        if (most_frequent.value == 1)
            break;

#if LOG == 1
        if (c > 256) {
            for (int i = 0; i < 3; i++) {
                printf("\033[1A");
                printf("\033[2K");
            }
        }
        printf("Iteration %d\n", (int)c - 255);
        printf("most frequent: [%d,%d]\n", most_frequent.key.pair[0],
               most_frequent.key.pair[1]);
        printf("frequency: %zu\n", most_frequent.value);
#endif

        hmput(*tokens, c, most_frequent.key);

        // replace in text
        Text old = text;
        text = pair_replace(text, most_frequent.key, c);
        da_free(old);
        hmfree(hm);
    }
    return text;
}

Text decode(Text text, TokenMap *tokens) {
    for (int i = hmlen(tokens) - 1; i >= 0; i--) {
        Text old = text;
        text = token_replace(text, tokens[i].key, tokens[i].value);
        da_free(old);
    };
    return text;
}

void usage(char *prog) {
    fprintf(stderr, "Usage: %s <encode|decode> <input> [tknfile]\n", prog);
    exit(1);
}

int main(int argc, char *argv[]) {
    char *program = shift(argv, argc);
    if (argc == 0) {
        fprintf(stderr, "No input provided\n");
        usage(program);
    }
    char *what = shift(argv, argc);
    int do_decode = 0;
    if (strcmp(what, "decode") == 0) {
        if (argc < 2) {
            fprintf(stderr, "Too few arguments provided\n");
            usage(program);
        }
        do_decode = 1;
    } else if (strcmp(what, "encode") == 0) {
        if (argc < 1) {
            fprintf(stderr, "Too few arguments provided\n");
            usage(program);
        }
    } else {
        fprintf(stderr, "Invalid argument: %s\n", what);
        usage(program);
    }
    if (!do_decode) {
        TokenMap *tokens = NULL;
        char *input = shift(argv, argc);
        Text text = readfile(input);
        Text enc = encode(text, &tokens);

        // output
        mkdir("output", 0755);
        char *outname_bpe;
        asprintf(&outname_bpe, "output/%s.bpe", input);
        char *outname_tkn;
        asprintf(&outname_tkn, "output/%s.tkn", input);

        writefile(outname_bpe, enc);
        writetokens(outname_tkn, tokens);
        free(outname_bpe);
        free(outname_tkn);
    } else {
        char *bpefile = shift(argv, argc);
        char *tknfile = shift(argv, argc);
        Text text = readbpe(bpefile);
        TokenMap *tokens = readtokens(tknfile);

        Text dec = decode(text, tokens);

        // print decoded to stdout
        da_foreach(token_t, x, &dec) { putchar((uchar)*x); }

        da_free(dec);
    }
}
