#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define NDIGITS(n)                                                             \
    ({                                                                         \
        typeof(n) _n = n;                                                      \
        u64 len = 0;                                                           \
        do {                                                                   \
            _n /= 10;                                                          \
            len++;                                                             \
        } while (_n != 0);                                                     \
        len;                                                                   \
    })

//

void rle(FILE *input) {
    i32 ch = fgetc(input);
    while (ch != EOF) {
        int current = ch;
        u8 count = 1;
        while ((ch = fgetc(input)) == current && count < 255) {
            count++;
        }
        putchar(current);
        putchar(count);
    }
}

void unrle(FILE *input) {
    while (1) {
        i32 current = fgetc(input);
        if (current == EOF)
            break;
        i32 count = fgetc(input);
        if (count == EOF) {
            break;
        }
        for (i32 i = 0; i < count; i++)
            putchar(current);
    }
}

int main(int argc, char **argv) {
    bool decode = false;
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
        case 'd':
            decode = true;
            break;
        default:
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        // inline argument
        for (int i = optind; i < argc; i++) {
            FILE *f = fopen(argv[i], "rb");
            if (!f) {
                perror(argv[i]);
                exit(EXIT_FAILURE);
            }
            decode ? unrle(f) : rle(f);
        }
    } else if (!isatty(STDIN_FILENO)) {
        // data piped through
        decode ? unrle(stdin) : rle(stdin);
    } else {
        fprintf(stderr, "No input provided\n");
    }

    exit(EXIT_SUCCESS);
}
