#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "trashhash.h"
#include "gstd_string.h"
#include "gstd_log.h"

#define ASSERT(cond) if (!(cond)) { __builtin_trap(); }

str read_file(const char *path);

static inline bool is_digit(uint8_t ch) { return ch >= '0' && ch <= '9'; }
const char *get_input(int argc, const char **argv);
bool str_parse_int(str s, int *result);
bool str_parse_int64(str s, int64_t *result);

extern str STR_NEWLINE;

#define MIN(a, b) (((a) <  (b)) ? (a) : (b))
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))

#define FOREACH_TOKEN(input, delim, token) \
    for (str token = str_next_token(&input, delim); !str_empty(input); token = str_next_token(&input, delim))

#define FOREACH_LINE(input, line) \
        while (!str_empty(line = str_next_token(&input, STR_NEWLINE)))


typedef struct {
    size_t count;
    size_t capacity;
    int64_t *data;
} IntVec;

void iv_init(IntVec *iv, size_t initial_capacity);
void iv_clear(IntVec *iv);
void iv_push(IntVec *iv, int64_t value);
bool iv_contains(IntVec *iv, int64_t value);
void iv_free(IntVec *iv);

typedef struct {
    uint8_t *cells;
    int width, height;
} CharGrid;

CharGrid read_char_grid(str input);
