#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 127
#define MIN 33

int main(int argc, char *argv[]) {
    srand(time(0));

    if (argc < 2) {
        fprintf(stderr,
                "Provide the length of the password (a positive integer)\n");
        return EXIT_FAILURE;
    }

    int passlen = atoi(argv[1]);
    if (passlen < 1) {
        fprintf(stderr, "Length must be a positive integer\n");
        return EXIT_FAILURE;
    }

    char *passwd;
    passwd = malloc(passlen + 1);
    for (int i = 0; i < passlen; i++) {
        char ascii_code = random() % (MAX - MIN + 1) + MIN;
        passwd[i] = ascii_code;
    }
    passwd[passlen] = 0;
    printf("%s\n", passwd);
    free(passwd);
    return EXIT_SUCCESS;
}
