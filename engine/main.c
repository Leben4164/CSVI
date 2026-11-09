#include <stdio.h>

#define MAX_LINE 65536

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: csvi <file.csv>\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "cannot open file: %s\n", argv[1]);
        return 1;
    }

    static char line[MAX_LINE];
    int rows = 0;
    int columns = 0;

    if (fgets(line, sizeof(line), fp) != NULL) {
        columns = 1;
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == ',') columns++;
        }
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        rows++;
    }

    fclose(fp);

    printf("{\"rows\": %d, \"columns\": %d}\n", rows, columns);
    return 0;
}