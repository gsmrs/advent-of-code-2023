#include "advent.h"
#include <stdlib.h>

void tilt_north(CharGrid *grid) {
    for (int y = 0; y < grid->height; y++) {
        for (int x = 0; x < grid->width; x++) {
            if (grid->cells[y * grid->width + x] == 'O') {
                for (int yy = y - 1; yy >= 0 && grid->cells[yy * grid->width + x] == '.'; yy--) {
                    grid->cells[(yy + 1) * grid->width + x] = '.';
                    grid->cells[yy * grid->width + x] = 'O';
                }
            }
        }
    }
}

void print_grid(CharGrid *grid) {
    for (int y = 0; y < grid->height; y++) {
        printf("%.*s\n", grid->width, grid->cells + y * grid->width);
    }
}

int compute_load(CharGrid *grid) {
    int total = 0;
    for (int y = 0; y < grid->height; y++) {
        for (int x = 0; x < grid->width; x++) {
            if (grid->cells[y * grid->width + x] == 'O') {
                int load = grid->height - y;
                total += load;
            }
        }
    }
    return total;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    bool print = false;

    CharGrid grid = read_char_grid(input);
    if (print) printf("Have grid of size (%d, %d)\n", grid.width, grid.height);
    if (print) print_grid(&grid);
    tilt_north(&grid);
    if (print) printf("------------------------------------\n");
    if (print) print_grid(&grid);

    int part_1 = compute_load(&grid);
    printf("%d\n", part_1);

    free(grid.cells);
    free(input.data);
}
