#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define STB_DS_IMPLEMENTATION
#include "array.h"
#include "stb_ds.h"

#define LOG 0

typedef unsigned char uchar;

typedef struct {
    uchar pair[2];
} Pair;

typedef struct {
    Pair key;
    size_t value;
} Freq;

typedef struct {
    uchar key;
    Pair value;
} TokenMap;

typedef struct {
    size_t count;
    size_t capacity;
    Freq *items;
} Freqs;

int compare_freq(const void *a, const void *b) {
    int x = (int)((Freq *)a)->value;
    int y = (int)((Freq *)b)->value;
    if (x < y)
        return 1;
    else if (x > y)
        return -1;
    else
        return 0;
}

char *strreplace(char *str, char *pattern, char *repl) {
    size_t len = strlen(str);
    size_t patlen = strlen(pattern);
    size_t repllen = strlen(repl);

    // count occurrences of pattern in str
    size_t count = 0;
    char *tmp = str;
    while ((tmp = strstr(tmp, pattern))) {
        count++;
        tmp += patlen;
    }

    if (count == 0)
        return strdup(str);

    char *ret = calloc(len - count * patlen + count * repllen + 1, 1);
    ////////////
    char *p;
    char *cursor = ret;
    while ((p = strstr(str, pattern))) {
        // copy original string
        memcpy(cursor, str, p - str);
        cursor += p - str;
        // copy replacement
        memcpy(cursor, repl, repllen);
        cursor += repllen;
        // adjust target pointer
        str = p + patlen;
    }
    // copy the remaining part
    memcpy(cursor, str, strlen(str));
    return ret;
}

char *readfile(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        exit(1);
    }

    // get length of stream
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    // reset stream
    rewind(file);

    char *buffer = malloc(len + 1);
    size_t ret = fread(buffer, 1, len, file);
    buffer[len] = '\0';
    if (ret != (size_t)len) {
        fprintf(stderr, "fread: failed reading file %s\n", filename);
        exit(1);
    }
    fclose(file);
    return buffer;
}

void writefile(char *filename, char *text) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror(filename);
        exit(1);
    }
    fwrite(text, 1, strlen(text), file);
    fclose(file);
};

void writetokens(char *filename, TokenMap *tokens) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror(filename);
        exit(1);
    }
    for (int i = 0; i < hmlen(tokens); i++) {
        char line[] = {
            tokens[i].key,
            tokens[i].value.pair[0],
            tokens[i].value.pair[1],
        };
        fwrite(line, 1, 3, file);
    }
}

TokenMap *readtokens(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        exit(1);
    }
    TokenMap *tokens = NULL;

    char line[3] = {0};
    while (fread(&line[0], 1, 3, file) == 3) {
        uchar key = line[0];
        Pair pair = {{line[1], line[2]}};
        hmput(tokens, key, pair);
    }
    fclose(file);
    return tokens;
}

#define shift(xs, x) (assert(x > 0), x--, *xs++)

void usage(char *prog) {
    fprintf(stderr, "Usage: %s <encode|decode> <input> [tknfile]\n", prog);
    exit(1);
}

char *encode(char *text, TokenMap **tokens) {
    for (uchar c = 128; c < 255; c++) {
        size_t len = strlen(text);
        Freq *hm = NULL;
        for (size_t i = 0; i < len - 1; i++) {
            Pair pair = {{text[i], text[i + 1]}};
            int index = hmgeti(hm, pair);
            if (index == -1)
                hmput(hm, pair, 1);
            else
                hm[index].value++;
        }

        // get most frequent pair
        Freqs sorted_freqs = {0};
        for (int i = 0; i < hmlen(hm); i++) {
            da_append(&sorted_freqs, hm[i]);
        }

        qsort(sorted_freqs.items, sorted_freqs.count,
              sizeof(*sorted_freqs.items), compare_freq);

        // most frequent pair appears once -- nothing left to replace
        if (sorted_freqs.items[0].value == 1)
            break;

        Pair most_frequent = sorted_freqs.items[0].key;

#if LOG == 1
        printf("---------\n");
        printf("Iteration %d\n", (int)c - 127);
        printf("most frequent: [%c%c]\n", most_frequent.pair[0],
               most_frequent.pair[1]);
        printf("frequency: %zu\n", sorted_freqs.items[0].value);
#endif

        hmput(*tokens, c, most_frequent);

        // replace in text
        char pattern[] = {most_frequent.pair[0], most_frequent.pair[1], '\0'};
        char repl[] = {c, '\0'};
        char *old = text;
        text = strreplace(text, pattern, repl);
        free(old);
    }
    return text;
}

char *decode(char *text, TokenMap *tokens) {
    char *text2 = strdup(text);
    for (int i = hmlen(tokens) - 1; i >= 0; i--) {
        char pattern[] = {tokens[i].key, '\0'};
        char repl[] = {tokens[i].value.pair[0], tokens[i].value.pair[1], '\0'};
        char *old = text2;
        text2 = strreplace(text2, pattern, repl);
        free(old);
    };
    return text2;
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
        fprintf(stderr, "Invalid argument\n");
        usage(program);
    }
    if (!do_decode) {
        TokenMap *tokens = NULL;
        char *input = shift(argv, argc);
        char *text = readfile(input);
        char *enc = encode(text, &tokens);

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
        char *text = readfile(bpefile);
        TokenMap *tokens = readtokens(tknfile);

        char *dec = decode(text, tokens);

        printf("%s", dec);
        free(text);
        free(dec);
    }
}
