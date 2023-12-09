#define GSTD_STRING_IMPLEMENTATION
#include "gstd_string.h"
#define GSTD_LOG_IMPLEMENTATION
#include "gstd_log.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

str STR_NEWLINE = { .data = (uint8_t *) "\n", .len = 1 };

str read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        log_fatal("Could not open file: %s", path);
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = calloc(size + 1, 1);
    fread(data, 1, size, f);
    fclose(f);
    return make_str(data, size);
}

const char *get_input(int argc, const char **argv) {
    if (argc == 1) {
        fprintf(stderr, "Please provide the file containing the input: %s <input_file>\n", argv[0]);
        exit(-1);
    } else {
        return argv[1];
    }
}

bool str_parse_int(str s, int *result) {
    *result = 0;
    for (size_t i = 0; i < s.len; i++) {
        uint8_t ch = s.data[i];
        if (isdigit(ch)) {
            *result = 10 * (*result) + (ch - '0');
        } else {
            return false;
        }
    }
    return true;
}
bool str_parse_int64(str s, int64_t *result) {
    *result = 0;
    for (size_t i = 0; i < s.len; i++) {
        uint8_t ch = s.data[i];
        if (isdigit(ch)) {
            *result = 10 * (*result) + (ch - '0');
        } else {
            return false;
        }
    }
    return true;
}
