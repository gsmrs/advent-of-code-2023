#include "advent.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Grid {
    int rows, cols;
    uint8_t *cells;
} Grid;

Grid read_grid(str input) {
    str tmp = input;
    str line = str_next_token(&tmp, cstr("\n"));
    size_t ncols = line.len;
    size_t nrows = 0;

    tmp = input;
    FOREACH_LINE(tmp, line) {
        nrows++;
    }

    uint8_t *cells = calloc(ncols * nrows, 1);

    tmp = input;
    int row = 0;
    FOREACH_LINE(tmp, line) {
        assert(line.len == ncols);
        memcpy(cells + row * ncols, line.data, line.len);
        row++;
    }

    return (Grid) {
        .rows = nrows,
        .cols = ncols,
        .cells = cells
    };
}

static const int offsets[8][2] = {
    {-1, -1},
    {-1,  0},
    {-1,  1},
    { 0, -1},
    { 0,  1},
    { 1, -1},
    { 1,  0},
    { 1,  1},
};

typedef struct {
    int position; // y * cols + x
    int ratio;
    int count;
} Gear;

typedef struct {
    int count;
    int capacity;
    Gear *gears;
} Gears;

void gears_init(Gears *gears, int capacity) {
    gears->count = 0;
    gears->capacity = capacity;
    gears->gears = calloc(gears->capacity, sizeof(Gear));
}

Gear *gear_at(Gears *gears, int position) {
    for (int i = 0; i < gears->count; i++) {
        if (gears->gears[i].position == position) {
            return &gears->gears[i];
        }
    }
    return NULL;
}

Gear *gear_add(Gears *gears, int position) {
    if (gears->count == gears->capacity) {
        gears->capacity *= 2;
        gears->gears = realloc(gears->gears, gears->capacity * sizeof(Gear));
    }
    gears->gears[gears->count++] = (Gear) { .position = position, .ratio = 1, .count = 0 };
    return &gears->gears[gears->count - 1];
}

int main(int argc, const char **argv) {
    str input = read_file(get_input(argc, argv));
    Grid grid = read_grid(input);

    int part_1 = 0; // sum of part numbers

    Gears gears;
    gears_init(&gears, 32);

    // scan for all symbols
    for (int y = 0; y < grid.rows; y++) {
        uint8_t *row = grid.cells + y * grid.cols;
        for (int x = 0; x < grid.cols; x++) {
            // check if we have a number
            if (isdigit(row[x])) {
                // if so, read the number...
                int number = 0;
                bool is_part_number = false;
                Gear *gear = NULL;
                for (; x < grid.cols && isdigit(row[x]); x++) {
                    number = 10 * number + (row[x] - '0');
                    if (!is_part_number) {
                        // ...and scan surrounding area of each digit for whether they are attached to a part.
                        for (int offset_index = 0; offset_index < 8; offset_index++) {
                            int dx = offsets[offset_index][0];
                            int dy = offsets[offset_index][1];

                            int x_ = x + dx;
                            int y_ = y + dy;
                            // check if within grid
                            if ((x_ >= 0) && (y_ >= 0) && (x_ < grid.cols) && (y_ < grid.rows)) {
                                // if so, check if we have non-empty adjacent cell which is not a number
                                uint8_t adjacent = grid.cells[y_ * grid.cols + x_];
                                if (!isdigit(adjacent)) {
                                    if (adjacent == '*') {
                                        int position = y_ * grid.cols + x_;
                                        gear = gear_at(&gears, position);
                                        if (!gear) {
                                            gear = gear_add(&gears, position);
                                        }
                                    }
                                    is_part_number |= (adjacent != '.');
                                }
                            }
                        }
                    }
                }
                if (gear) {
                    gear->count++;
                    gear->ratio *= number;
                }

                if (is_part_number) {
                    part_1 += number;
                }
            }
        }
    }

    int part_2 = 0; // sum of gear ratios
    for (int i = 0; i < gears.count; i++) {
        if (gears.gears[i].count >= 2) {
            part_2 += gears.gears[i].ratio;
        }
    }

    printf("%d\n", part_1);
    printf("%d\n", part_2);

    free(grid.cells);
    free(gears.gears);
}
