#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    char *str;
    u64 size;
} string;

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

#define IS_NUM(c) (((c) >= 48 && (c) <= 57) ? true : false)
#define IS_ALPHA(c)                                                            \
    ((((c) >= 32 && (c) <= 47) || ((c) >= 58 && (c) <= 126)) ? true : false)

string rle(string data) {
    u64 capacity = data.size * 2;
    string encoded = {0};
    encoded.str = malloc(capacity);
    char current = data.str[0];
    if (!IS_ALPHA(current)) {
        fprintf(stderr, "Invalid character at position 0: %c\n", current);
        exit(EXIT_FAILURE);
    }
    u64 offset = 0;
    u64 count = 1;
    for (u64 i = 1; i <= data.size; i++) {
        if (i < data.size && !IS_ALPHA(data.str[i])) {
            fprintf(stderr, "Invalid character at position %lu: %c\n", i,
                    data.str[i]);
            exit(EXIT_FAILURE);
        }
        if (current == (i < data.size ? data.str[i] : '\0')) {
            count++;
            continue;
        }
        u64 count_nchar = NDIGITS(count); // number of digits
        while (offset + 1 + count_nchar >= capacity) {
            capacity *= 2;
            encoded.str = realloc(encoded.str, capacity);
        }
        offset += sprintf(encoded.str + offset, "%c%lu", current, count);
        encoded.size = offset; // 1 for letter
        current = data.str[i];
        count = 1;
    }
    encoded.str = realloc(encoded.str, encoded.size + 1);

    return encoded;
}

string unrle(string data) {
    u64 capacity = data.size * 10;
    string decoded = {0};
    decoded.str = malloc(capacity);
    char current;
    u64 offset = 0;
    u64 count;
    u64 count_cap = 3; // max 3 digits for start
    char *count_c = malloc(count_cap);
    u64 count_len = 0;
    char letter = data.str[0];
    for (u64 i = 1; i <= data.size; i++) {
        current =
            (i < data.size ? data.str[i] : '\0'); // null byte for final flush
        if (IS_ALPHA(current) || current == '\0') {
            char *endptr;
            count_c[count_len] = '\0';
            count = strtoull(count_c, &endptr, 10);
            while (offset + count + 1 >= capacity) {
                capacity *= 2;
                decoded.str = realloc(decoded.str, capacity);
            }
            for (u64 ind = 0; ind < count; ind++) {
                decoded.str[offset + ind] = letter;
            }
            offset += count;
            count_len = 0;
            letter = current;
        } else if (IS_NUM(current)) {
            count_len++;
            while (count_len >= count_cap) {
                count_cap *= 2;
                count_c = realloc(count_c, count_cap);
            }
            count_c[count_len - 1] = current;
        } else {
            fprintf(stderr, "invalid character: %c", current);
            exit(EXIT_FAILURE);
        }
    }
    free(count_c);
    decoded.size = offset;
    decoded.str[offset] = '\0';

    return decoded;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "No arguments provided\n");
        exit(EXIT_FAILURE);
    }
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
    for (int i = 1 + (int)decode; i < argc; i++) {
        string data;
        data.str = argv[i];
        data.size = strlen(data.str);
        string out = decode ? unrle(data) : rle(data);
        printf("%s\n", out.str);
        free(out.str);
    }
    exit(EXIT_SUCCESS);
}
