#include "advent.h"
#include <stdlib.h>

typedef struct {
    enum {
        HORIZONTAL, VERTICAL
    } symmetry;
    int coordinate;
} MirrorAxis;

typedef struct {
    MirrorAxis *axes;
    int capacity;
    int count;
} MirrorAxes;

void axes_push(MirrorAxes *ma, MirrorAxis ax) {
    if (ma->count == ma->capacity) {
        ma->capacity = MAX(8, 2 * ma->capacity);
        ma->axes = realloc(ma->axes, ma->capacity * sizeof(*ma->axes));
    }
    ma->axes[ma->count++] = ax;
}

void axes_clear(MirrorAxes *ma) {
    ma->count = 0;
}

void find_mirror_axes(CharGrid grid, MirrorAxes *axes) {
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
        MirrorAxis axis = { .symmetry = HORIZONTAL, .coordinate = y_axis };
        axes_push(axes, axis);
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
        MirrorAxis axis = { .symmetry = VERTICAL, .coordinate = x_axis };
        axes_push(axes, axis);
next_x:
    }
}

MirrorAxis find_smudged_mirror_axis(CharGrid grid, MirrorAxis original_axis) {
    MirrorAxes axes = {0};

    for (int i = 0; i < grid.width * grid.height; i++) {
        uint8_t original = grid.cells[i];
        grid.cells[i] = (original == '.') ? '#' : '.';
        axes_clear(&axes);
        find_mirror_axes(grid, &axes);
        grid.cells[i] = original;

        for (int j = 0; j < axes.count; j++) {
            MirrorAxis axis = axes.axes[j];
            bool same_axis = (original_axis.symmetry == axis.symmetry)
                && (original_axis.coordinate == axis.coordinate);
            if (!same_axis) {
                free(axes.axes);
                return axis;
            }
        }
    }

    printf("Found no smudged axis :(\n");
    ASSERT(0 && "not reachable");
}

int score_axis(MirrorAxis axis) {
    if (axis.symmetry == VERTICAL) {
        return axis.coordinate + 1; // +1 for 1-based indexing
    } else if (axis.symmetry == HORIZONTAL) {
        return 100 * (axis.coordinate + 1); // +1 for 1-based indexing
    } else {
        return 0;
    }
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);
    void *input_ptr = input.data;

    int part_1 = 0;
    int part_2 = 0;

    MirrorAxes axes = {0};
    while (!str_empty(input)) {
        str block = str_next_token(&input, cstr("\n\n"));
        /* printf("Block:\n" STR_FMT "\n", STR(block)); */

        CharGrid grid = read_char_grid(block);
        axes_clear(&axes);
        find_mirror_axes(grid, &axes);
        ASSERT(axes.count > 0);
        part_1 += score_axis(axes.axes[0]);

        MirrorAxis axis = find_smudged_mirror_axis(grid, axes.axes[0]);
        part_2 += score_axis(axis);

        free(grid.cells);
    }
    free(axes.axes);
    free(input_ptr);
    printf("%d\n", part_1);
    printf("%d\n", part_2);

}
