#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "advent.h"


typedef struct {
    char direction;
    int distance;
    // TODO color
} Command;

void dump_grid(char *grid, int width, int height, const char *name) {
    FILE *f = fopen(name, "wb");
    ASSERT(f);
    for (int y = 0; y < height; y++) {
        fprintf(f, "%.*s\n", width, grid + y * width);
    }
    fclose(f);
}

void dump_grid_ppm(char *grid, int width, int height, const char *name) {
    FILE *f = fopen(name, "wb");
    ASSERT(f);
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (int i = 0; i < width * height; i++) {
        uint8_t v;
        switch (grid[i]) {
            case '.': v = 0; break;
            case 'i': v = 127; break;
            case '#': v = 255; break;
            default: ASSERT(0);
        }
        uint8_t pixel[3] = { v, v, v };
        fwrite(pixel, 3, 1, f);
    }
    fclose(f);
}

bool find_point_inside(char *grid, int width, int height, int *px, int *py) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width - 2; x++) {
            int index = y * width + x;
            if (grid[index] == '.'
                    && grid[index + 1] == '#'
                    && grid[index + 2] == '.') {
                *px = x + 2;
                *py = y;
                return true;
            }
        }
    }
    return false;
}

static const int OFFSETS[4][2] = {
    {-1, 0},
    {1, 0},
    {0, -1},
    {0, 1}
};

void flood_fill(char *grid, int width, int height, int x, int y) {
    if (grid[y * width + x] == '.') {
        grid[y * width + x] = 'i';
    } else {
        return;
    }

    for (int i = 0; i < 4; i++) {
        int xx = x + OFFSETS[i][0];
        int yy = y + OFFSETS[i][1];
        if (xx >= 0 && yy >= 0 && xx < width && yy < height
                && grid[yy * width + xx] == '.') {
            flood_fill(grid, width, height, xx, yy);
        }
    }
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    enum { NUM_MAX_CMDS = 1024 };
    int cmd_count = 0;
    Command cmds[NUM_MAX_CMDS];

    str lines = input;
    while (!str_empty(lines)) {
        str line = str_next_token(&lines, cstr("\n"));
        if (str_empty(line)) {
            continue;
        }

        str direction_s = str_next_token(&line, cstr(" "));
        char direction = direction_s.data[0];
        str distance_s = str_next_token(&line, cstr(" "));
        int distance;
        ASSERT(str_parse_int(distance_s, &distance));

        ASSERT(cmd_count < NUM_MAX_CMDS);
        cmds[cmd_count++] = (Command) {
            .direction = direction,
            .distance = distance
        };
    }

    // find bounds
    int x_min = INT_MAX, y_min = INT_MAX;
    int x_max = INT_MIN, y_max = INT_MIN;
    int x = 0, y = 0;
    for (int i = 0; i < cmd_count; i++) {
        int dx, dy;
        switch (cmds[i].direction) {
            case 'R': dx = 1;  dy = 0;  break;
            case 'D': dx = 0;  dy = 1;  break;
            case 'L': dx = -1; dy = 0;  break;
            case 'U': dx = 0;  dy = -1; break;
            default: ASSERT(0);
        }

        x += dx * cmds[i].distance;
        y += dy * cmds[i].distance;
        x_min = MIN(x_min, x);
        y_min = MIN(y_min, y);
        x_max = MAX(x_max, x);
        y_max = MAX(y_max, y);
    }

    int width = x_max - x_min + 1;
    int height = y_max - y_min + 1;

    char *grid = malloc(width * height);
    memset(grid, '.', width * height);

    x = 0;
    y = 0;
    for (int i = 0; i < cmd_count; i++) {
        int dx, dy;
        switch (cmds[i].direction) {
            case 'R': dx = 1;  dy = 0;  break;
            case 'D': dx = 0;  dy = 1;  break;
            case 'L': dx = -1; dy = 0;  break;
            case 'U': dx = 0;  dy = -1; break;
            default: ASSERT(0);
        }

        for (int j = 0; j < cmds[i].distance; j++) {
            x += dx;
            y += dy;
            grid[(y - y_min) * width + (x - x_min)] = '#';
        }
    }

    int px, py; // any point inside the boundary.
    ASSERT(find_point_inside(grid, width, height, &px, &py));
    flood_fill(grid, width, height, px, py);

    char *end = grid + width * height;
    int part_1 = 0;
    for (char *ptr = grid; ptr < end; ptr++) {
        part_1 += (*ptr != '.');
    }
    printf("%d\n", part_1);
    dump_grid(grid, width, height, "grid.txt");
    dump_grid_ppm(grid, width, height, "grid.ppm");

    free(input.data);
    free(grid);
}
