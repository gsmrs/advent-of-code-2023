#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "gstd_string.h"
#include "gstd_log.h"

str read_file(const char *path);

static inline bool is_digit(uint8_t ch) { return ch >= '0' && ch <= '9'; }
const char *get_input(int argc, const char **argv);

extern str STR_NEWLINE;

#define FOREACH_LINE(input, line) \
        while (!str_empty(line = str_next_token(&input, STR_NEWLINE)))
