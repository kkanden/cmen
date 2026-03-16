#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ANSI_RED "\e[31m"
#define ANSI_RESET "\e[0m"
#define ANSI_MAGENTA "\e[35m"

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct {
    u64 *start;
    u64 size;
} StrMatch;

// if s2 is contained in s1 return index of start of first match
// otherwise return -1
StrMatch strmatchcmp(char *s1, char *s2, bool (*cmp)(char, char)) {
    u64 capacity = 4; // initial guess for max matches
    u64 *start = malloc(capacity * sizeof(u64));
    u64 size = 0;
    u64 len1 = strlen(s1);
    u64 len2 = strlen(s2);

    for (u64 i = 0; i < len1; i++) {
        i64 j = 0;
        while (j < len2 && cmp(s1[i + j], s2[j]))
            j++;
        if (j == len2) {
            if (size == capacity) {
                capacity *= 2;
                start = realloc(start, capacity * sizeof(u64));
            }
            start[size] = i;
            size++;
        }
    }

    return (StrMatch){.start = start, .size = size};
}

bool cmpsens(char a, char b) { return a == b; }
StrMatch strmatch(char *s1, char *s2) { return strmatchcmp(s1, s2, cmpsens); }

bool cmpinsens(char a, char b) { return tolower(a) == tolower(b); }
StrMatch istrmatch(char *s1, char *s2) {
    return strmatchcmp(s1, s2, cmpinsens);
}

void print_usage(char *prog) {
    fprintf(stderr, "Usage: %s [-i] PATTERN FILE...\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    bool ignore_case = false;
    int opt;
    while ((opt = getopt(argc, argv, "i")) != -1) {
        switch (opt) {
        case 'i':
            ignore_case = true;
            break;
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    // optind points to the first non-option argument
    char *pattern = argv[optind];
    u64 patlen = strlen(pattern);
    bool multiple_files = argc - optind - 1 > 1;

    bool anymatch = false;
    for (int indf = optind + 1; indf < argc; indf++) {
        char *filename = argv[indf];
        char *prefix = multiple_files ? NULL : "";
        if (multiple_files)
            asprintf(&prefix, ANSI_MAGENTA "%s" ANSI_RESET ":", filename);

        FILE *file;
        file = fopen(filename, "r");
        if (file == NULL) {
            char *errprefix;
            asprintf(&errprefix, "%s: %s", argv[0], filename);
            perror(errprefix);
            free(errprefix);
            continue;
        }

        char *line = NULL;
        char *linehl;
        u64 size = 0;
        i64 nread;
        while ((nread = getline(&line, &size, file)) != -1) {
            StrMatch match = ignore_case ? istrmatch(line, pattern)
                                         : strmatch(line, pattern);
            u64 linelen = strlen(line);
            if (match.size > 0) {
                u64 cursor = 0;
                anymatch = true;
                u64 ansi_len = strlen(ANSI_RED) + strlen(ANSI_RESET);
                u64 total_ansi_len = match.size * ansi_len;
                linehl =
                    malloc(linelen + total_ansi_len +
                           1); // highlighted line needs extra space for ANSIs
                for (u64 ind = 0; ind < match.size; ind++) {
                    u64 seg_start =
                        (ind == 0) ? 0 : match.start[ind - 1] + patlen;
                    u64 seg_len = match.start[ind] - seg_start;
                    memcpy(linehl + cursor, line + seg_start, seg_len);
                    cursor += seg_len;

                    sprintf(linehl + cursor, ANSI_RED "%.*s" ANSI_RESET,
                            (int)patlen, line + match.start[ind]);
                    cursor += patlen + ansi_len;
                }
                u64 last_end = match.start[match.size - 1] + patlen;
                memcpy(linehl + cursor, line + last_end,
                       linelen - last_end); // copy line after last match
                linehl[cursor + linelen - last_end] = '\0';
                printf("%s%s", prefix, linehl);
                free(linehl);
            }
            free(match.start);
        }

        free(line);
        line = NULL;
        size = 0;
        fclose(file);
        if (multiple_files)
            free(prefix);
    }
    if (!anymatch) {
        exit(EXIT_FAILURE);
    }
    return 0;
}
