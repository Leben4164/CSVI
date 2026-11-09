#include <stdio.h>
#include <string.h>

#define MAX_LINE 65536
#define SCHEMA_VERSION "1.0"

static const char *get_file_name(const char *path) {
    const char *name = path;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') name = p + 1;
    }
    return name;
}

static void print_json_string(const char *s) {
    putchar('"');
    for (; *s != '\0'; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"')       printf("\\\"");
        else if (c == '\\') printf("\\\\");
        else if (c == '\n') printf("\\n");
        else if (c == '\r') printf("\\r");
        else if (c == '\t') printf("\\t");
        else if (c < 0x20)  printf("\\u%04x", c);
        else                putchar(c);
    }
    putchar('"');
}

static void print_error(const char *file_name, const char *code, const char *message) {
    printf("{\"schema_version\": \"%s\", \"status\": \"error\", \"file_name\": ", SCHEMA_VERSION);
    if (file_name != NULL) print_json_string(file_name);
    else                   printf("null");
    printf(", \"summary\": null, \"columns\": [], \"error\": {\"code\": ");
    print_json_string(code);
    printf(", \"message\": ");
    print_json_string(message);
    printf("}}\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_error(NULL, "MISSING_ARGUMENT", "CSV file path was not given.");
        return 1;
    }

    const char *file_name = get_file_name(argv[1]);

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        print_error(file_name, "FILE_NOT_FOUND", "CSV file could not be found.");
        return 1;
    }

    static char line[MAX_LINE];
    int row_count = 0;
    int column_count = 0;

    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        print_error(file_name, "EMPTY_FILE", "CSV file is empty.");
        return 1;
    }
    column_count = 1;
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',') column_count++;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == '\n' || line[0] == '\r') continue;
        row_count++;
    }

    fclose(fp);

    printf("{\"schema_version\": \"%s\", \"status\": \"success\", \"file_name\": ", SCHEMA_VERSION);
    print_json_string(file_name);
    printf(", \"summary\": {\"row_count\": %d, \"column_count\": %d}", row_count, column_count);
    printf(", \"columns\": [], \"error\": null}\n");
    return 0;
}