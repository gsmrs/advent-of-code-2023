#include "advent.h"
#include <stdlib.h>
#include <string.h>

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

void tilt_south(CharGrid *grid) {
    for (int y = grid->height - 1; y >= 0; y--) {
        for (int x = 0; x < grid->width; x++) {
            if (grid->cells[y * grid->width + x] == 'O') {
                for (int yy = y + 1; yy < grid->height && grid->cells[yy * grid->width + x] == '.'; yy++) {
                    grid->cells[(yy - 1) * grid->width + x] = '.';
                    grid->cells[yy * grid->width + x] = 'O';
                }
            }
        }
    }
}

void tilt_west(CharGrid *grid) {
    for (int x = 0; x < grid->width; x++) {
        for (int y = 0; y < grid->height; y++) {
            if (grid->cells[y * grid->width + x] == 'O') {
                for (int xx = x - 1; xx >= 0 && grid->cells[y * grid->width + xx] == '.'; xx--) {
                    grid->cells[y * grid->width + (xx + 1)] = '.';
                    grid->cells[y * grid->width + xx] = 'O';
                }
            }
        }
    }
}

void tilt_east(CharGrid *grid) {
    for (int x = grid->width - 1; x >= 0; x--) {
        for (int y = 0; y < grid->height; y++) {
            if (grid->cells[y * grid->width + x] == 'O') {
                for (int xx = x + 1; xx < grid->width && grid->cells[y * grid->width + xx] == '.'; xx++) {
                    grid->cells[y * grid->width + (xx - 1)] = '.';
                    grid->cells[y * grid->width + xx] = 'O';
                }
            }
        }
    }
}

void tilt_cycle(CharGrid *grid) {
    tilt_north(grid);
    tilt_west(grid);
    tilt_south(grid);
    tilt_east(grid);
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

CharGrid copy_char_grid(CharGrid *grid) {
    uint8_t *cells = malloc(grid->width * grid->height);
    memcpy(cells, grid->cells, grid->width * grid->height);
    return (CharGrid) { .width = grid->width, .height = grid->height, .cells = cells };
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    bool print = false;

    CharGrid grid = read_char_grid(input);

    // Part 1
    CharGrid grid_part_1 = copy_char_grid(&grid);
    if (print) printf("Have grid of size (%d, %d)\n", grid.width, grid.height);
    if (print) print_grid(&grid_part_1);
    tilt_north(&grid_part_1);
    if (print) printf("------------------------------------\n");
    if (print) print_grid(&grid_part_1);

    int part_1 = compute_load(&grid_part_1);
    printf("%d\n", part_1);

    // Part 2: Keep track of past `HIST_LEN` grid states in a ring buffer (`history`).
    enum { HIST_LEN = 64 };
    // ring buffer of past grids
    struct {
        CharGrid grid;
        int iteration;
    } history[HIST_LEN] = {0};
    int head = 0;
    int tail = 0;

    for (int i = 0; i < 1000000000; i++) {
        // check if current state has already been seen
        bool seen = false;
        for (int j = tail; j != head; j = (j + 1) % HIST_LEN) {
            if (memcmp(history[j].grid.cells, grid.cells, grid.width * grid.height) == 0) {
                int cycle_len = i - history[j].iteration;
                // Since the current grid state will repeat every `cycle_len`, we can jump
                // ahead to the largest multiple of `cycle_len` below 1000000000.
                int n = (1000000000 - history[j].iteration) / cycle_len;
                i = history[j].iteration + n * cycle_len;
                seen = true;
                break;
            }
        }

        if (!seen) {
            // otherwise, if the state has not been seen yet, push it to the ring buffer.
            int next = (head + 1) % HIST_LEN;
            if (next != tail) {
                assert(!history[head].grid.cells);
                history[head].grid = copy_char_grid(&grid);
            } else {
                // ring buffer full, overwrite oldest entry
                if (history[head].grid.cells) {
                    memcpy(history[head].grid.cells, grid.cells, grid.width * grid.height);
                } else {
                    history[head].grid = copy_char_grid(&grid);
                }
                tail = (tail + 1) % HIST_LEN;
            }
            history[head].iteration = i;
            head = next;
        }

        tilt_cycle(&grid);
    }

    int part_2 = compute_load(&grid);
    printf("%d\n", part_2);

    for (int i = 0; i < HIST_LEN; i++) {
        if (history[i].grid.cells) {
            free(history[i].grid.cells);
        }
    }
    free(grid_part_1.cells);
    free(grid.cells);
    free(input.data);
}
