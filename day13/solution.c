#include "advent.h"
#include <stdlib.h>

typedef struct {
    enum {
        HORIZONTAL, VERTICAL, NO_SYMMETRY
    } symmetry;
    int coordinate;
} MirrorAxis;

MirrorAxis find_mirror_axis(CharGrid grid) {
    // first, try horizontal axis
    for (int y_axis = 0; y_axis < grid.height - 1; y_axis++) {
        for (
                int y_below = y_axis + 1, y_above = y_axis;
                y_above >= 0 && y_below < grid.height;
                y_above--, y_below++)
        {
            for (int x = 0; x < grid.width; x++) {
                if (grid.cells[y_below * grid.width + x] != grid.cells[y_above * grid.width + x]) {
                    goto next_y;
                }
            }
        }
        // if we reached here, this is a valid axis.
        return (MirrorAxis) { .symmetry = HORIZONTAL, .coordinate = y_axis };
next_y:
    }

    // next, try vertical axis
    for (int x_axis = 0; x_axis < grid.width - 1; x_axis++) {
        for (
                int x_left = x_axis, x_right = x_axis + 1;
                x_left >= 0 && x_right < grid.width;
                x_left--, x_right++
            )
        {
            for (int y = 0; y < grid.height; y++) {
                if (grid.cells[y * grid.width + x_left] != grid.cells[y * grid.width + x_right]) {
                    goto next_x;
                }
            }
        }
        // if we reached here, this is a valid axis.
        return (MirrorAxis) { .symmetry = VERTICAL, .coordinate = x_axis };
next_x:
    }

    return (MirrorAxis) { .symmetry = NO_SYMMETRY };
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    int part_1 = 0;
    while (!str_empty(input)) {
        str block = str_next_token(&input, cstr("\n\n"));
        /* printf("Block:\n" STR_FMT "\n", STR(block)); */

        CharGrid grid = read_char_grid(block);
        MirrorAxis axis = find_mirror_axis(grid);
        if (axis.symmetry == VERTICAL) {
            part_1 += axis.coordinate + 1; // +1 for 1-based indexing
        } else if (axis.symmetry == HORIZONTAL) {
            part_1 += 100 * (axis.coordinate + 1); // +1 for 1-based indexing
        } else {
            ASSERT(0 && "NOT ALLOWED");
        }
        free(grid.cells);


    }
    printf("%d\n", part_1);

}
