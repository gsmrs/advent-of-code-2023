#include "advent.h"
#include <stdlib.h>

typedef struct {
    int x, y;
    int orig_x, orig_y;
} Galaxy;

int64_t abs_i64(int64_t i) {
    return (i >= 0) ? i : (-i);
}

int64_t solve(CharGrid cg, Galaxy *galaxies, int galaxy_count, int64_t expansion_factor) {
    // reset original positions of galaxies
    for (int i = 0; i < galaxy_count; i++) {
        galaxies[i].x = galaxies[i].orig_x;
        galaxies[i].y = galaxies[i].orig_y;
    }

    // scan for empty rows
    for (int y = 0; y < cg.height; y++) {
        bool is_empty = true;
        uint8_t *row = cg.cells + y * cg.width;
        for (int x = 0; x < cg.width; x++) {
            if (row[x] == '#') {
                is_empty = false;
                break;
            }
        }
        if (is_empty) {
            // Expand row to twice its size by shifting all galactic y-coordinates greater than
            // current y by +1.
            for (int galaxy_index = 0; galaxy_index < galaxy_count; galaxy_index++) {
                if (galaxies[galaxy_index].orig_y > y) {
                    galaxies[galaxy_index].y += expansion_factor - 1;
                }
            }
        }
    }

    // scan for empty columns
    for (int x = 0; x < cg.width; x++) {
        bool is_empty = true;
        for (int y = 0; y < cg.height; y++) {
            int index = y * cg.width + x;
            if (cg.cells[index] == '#') {
                is_empty = false;
                break;
            }
        }
        if (is_empty) {
            // expand column to twice its size by shifting all galactic x-coordiantes greater than
            // current x by +1.
            for (int galaxy_index = 0; galaxy_index < galaxy_count; galaxy_index++) {
                if (galaxies[galaxy_index].orig_x > x) {
                    galaxies[galaxy_index].x += expansion_factor - 1;
                }
            }
        }
    }

    // Consider pairwise distances between galaxies
    int64_t sum_of_distances = 0;
    for (int i = 0; i < galaxy_count; i++) {
        for (int j = 0; j < i; j++) {
            int64_t dist = abs_i64(galaxies[i].x - galaxies[j].x) + abs(galaxies[i].y - galaxies[j].y);
            /* printf("Distance between %d and %d is %d\n", i, j, dist); */
            sum_of_distances += dist;
        }
    }

    return sum_of_distances;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    enum { NUM_MAX_GALAXIES = 1024 };
    Galaxy galaxies[NUM_MAX_GALAXIES];
    int galaxy_count = 0;

    CharGrid cg = read_char_grid(input);

    for (int y = 0; y < cg.height; y++) {
        for (int x = 0; x < cg.width; x++) {
            if (cg.cells[y * cg.width + x] == '#') {
                assert(galaxy_count < NUM_MAX_GALAXIES);
                galaxies[galaxy_count++] = (Galaxy) { .x = x, .y = y, .orig_x = x, .orig_y = y };
            }
        }
    }
    printf("Found %d galaxies\n", galaxy_count);


    int64_t part_1 = solve(cg, galaxies, galaxy_count, 2);
    int64_t part_2 = solve(cg, galaxies, galaxy_count, 1000 * 1000);
    printf("%ld\n", part_1);
    printf("%ld\n", part_2);
}
