#define GSTD_STRING_IMPLEMENTATION
#include "gstd_string.h"
#define GSTD_LOG_IMPLEMENTATION
#include "gstd_log.h"

#include "advent.h"

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
bool str_parse_int64(str s, int64_t *out_result) {
    int64_t result = 0;
    int64_t sign = 1;
    size_t i = 0;
    if (s.len > 0 && s.data[0] == '-') {
        sign = -1;
        i++;
    }
    for (; i < s.len; i++) {
        uint8_t ch = s.data[i];
        if (isdigit(ch)) {
            result = 10 * (result) + (ch - '0');
        } else {
            return false;
        }
    }
    result *= sign;
    *out_result = result;
    return true;
}

void iv_init(IntVec *iv, size_t initial_capacity) {
    iv->count = 0;
    iv->capacity = initial_capacity;
    assert((iv->data = malloc(iv->capacity * sizeof(*iv->data))));
}

void iv_clear(IntVec *iv) {
    iv->count = 0;
}

void iv_push(IntVec *iv, int64_t value) {
    if (iv->count == iv->capacity) {
        iv->capacity = MIN(8, 2 * iv->capacity);
        assert((iv->data = realloc(iv->data, iv->capacity * sizeof(*iv->data))));
    }
    iv->data[iv->count++] = value;
}

bool iv_contains(IntVec *iv, int64_t value) {
    for (size_t i = 0; i < iv->count; i++) {
        if (iv->data[i] == value) {
            return true;
        }
    }
    return false;
}

void iv_free(IntVec *iv) {
    if (iv->data) {
        free(iv->data);
        iv->data = NULL;
        iv->count = 0;
        iv->capacity = 0;
    }
}

