#include "advent.h"
#include <stdlib.h>
#include <limits.h>

uint8_t determine_cell_type(CharGrid *cg, int x, int y) {
#define CELL(x, y) cg->cells[(y) * cg->width + (x)]
    bool right = false, up = false, left = false, down = false;
    char ch;
    if ((ch = CELL(x + 1, y)) != '.') {
        switch (ch) {
            case '-':
            case 'J':
            case '7':
                right = true;
                break;
        }
    }
    if ((ch = CELL(x, y - 1)) != '.') {
        switch (ch) {
            case '|':
            case 'F':
            case '7':
                up = true;
                break;
        }
    }
    if ((ch = CELL(x - 1, y)) != '.') {
        switch (ch) {
            case '-':
            case 'F':
            case 'L':
                left = true;
                break;
        }
    }
    if ((ch = CELL(x, y + 1)) != '.') {
        switch (ch) {
            case '|':
            case 'J':
            case 'L':
                down = true;
                break;
        }
    }

    if (right && up) return 'L';
    if (right && left) return '-';
    if (right && down) return 'F';
    if (up && down) return '|';
    if (left && up) return 'J';
    if (left && down) return '7';
    assert(0 && "not reachable");

#undef CELL
}

typedef struct Point {
    int x, y;
} Point;

void get_neighbor_pipes(CharGrid cg, Point p, Point *n1, Point *n2) {
#define CELL(x, y) cg.cells[(y) * cg.width + (x)]
    uint8_t cell = CELL(p.x, p.y);
    assert(cell != '.');
    switch (cell) {
        case '|':
            *n1 = (Point) { .x = p.x, .y = p.y - 1 };
            *n2 = (Point) { .x = p.x, .y = p.y + 1 };
            break;
        case '-':
            *n1 = (Point) { .x = p.x - 1, .y = p.y };
            *n2 = (Point) { .x = p.x + 1, .y = p.y };
            break;
        case 'L':
            *n1 = (Point) { .x = p.x, .y = p.y - 1 };
            *n2 = (Point) { .x = p.x + 1, .y = p.y };
            break;
        case 'J':
            *n1 = (Point) { .x = p.x, .y = p.y - 1 };
            *n2 = (Point) { .x = p.x - 1, .y = p.y };
            break;
        case '7':
            *n1 = (Point) { .x = p.x, .y = p.y + 1 };
            *n2 = (Point) { .x = p.x - 1, .y = p.y };
            break;
        case 'F':
            *n1 = (Point) { .x = p.x, .y = p.y + 1 };
            *n2 = (Point) { .x = p.x + 1, .y = p.y };
            break;
        default:
            fprintf(stderr, "Unknown cell type: %c\n", cell);
            assert(0 && "whoopsie");
            break;
    }
#undef CELL
}

bool in_bounds(CharGrid *grid, Point p) {
    return (p.x >= 0) && (p.y >= 0) && (p.x < grid->width) && (p.y < grid->height);
}

enum PointRelation {
    INSIDE, OUTSIDE, UNKNOWN
};

enum PointRelation raycast(CharGrid *grid, Point p, Point start, bool *boundary) {
    int x = start.x;
    int y = start.y;

    int dx, dy;
    char wall;
    if (start.x == p.x) {
        // vertical
        if (start.y < p.y) {
            dx = 0;
            dy = 1;
        }
        else {
            dx = 0;
            dy = -1;
        }
        wall = '-';
    } else {
        if (start.x < p.x) {
            dx = 1;
            dy = 0;
        } else {
            dx = -1;
            dy = 0;
        }
        wall = '|';
    }


    enum PointRelation state = OUTSIDE;
    bool last_on_boundary = false;
    for (; !((x == p.x + dx) && (y == p.y + dy)); x += dx, y += dy) {
        int index = y * grid->width + x;
        bool on_boundary = boundary[index];
        if (on_boundary) {
            if (grid->cells[index] == wall) {
                if (state == INSIDE) state = OUTSIDE;
                else if (state == OUTSIDE) state = INSIDE;
            } else {
                state = UNKNOWN;
            }
        }
        last_on_boundary = on_boundary;
    }
    return state;
}

bool is_point_inside(CharGrid *grid, Point p, bool *boundary) {
    const int width = grid->width;
    const int height = grid->height;
    if (grid->cells[p.y * width + p.x] != '.') {
        return false;
    }


    enum PointRelation result;

    result = raycast(grid, p, (Point) { .x = 0, .y = p.y }, boundary);
    if (result != UNKNOWN) return result == INSIDE;

    result = raycast(grid, p, (Point) { .x = grid->width - 1, .y = p.y }, boundary);
    if (result != UNKNOWN) return result == INSIDE;

    result = raycast(grid, p, (Point) { .x = p.x, .y = 0 }, boundary);
    if (result != UNKNOWN) return result == INSIDE;

    result = raycast(grid, p, (Point) { .x = p.x, .y = grid->height - 1 }, boundary);
    if (result != UNKNOWN) return result == INSIDE;

    printf("Could not determine whether (%d, %d) is inside\n", p.x, p.y);
    /* return false; */
    assert(0 && "not reachable");
}

void dump_boundary(bool *boundary, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            putchar(boundary[y * width + x] ? '#' : '.');
        }
        puts("");
    }
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    CharGrid grid = read_char_grid(input);
    int start_x = -1, start_y = -1;
    for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++) {
            if (grid.cells[y * grid.width + x] == 'S') {
                start_x = x;
                start_y = y;
            }
        }
    }

    printf("start_x = %d, start_y = %d\n", start_x, start_y);

    assert(start_x != -1);
    assert(start_y != -1);
    assert(grid.cells[start_y * grid.width + start_x] == 'S');

    uint8_t start_type = determine_cell_type(&grid, start_x, start_y);
    printf("start_type = %c\n", start_type);
    grid.cells[start_y * grid.width + start_x] = start_type;


    bool *loop_boundary = calloc(grid.width * grid.height, sizeof(bool));
    int *distance = calloc(grid.width * grid.height, sizeof(int));
    for (int i = 0; i < grid.width * grid.height; i++) {
        distance[i] = INT_MAX;
    }
    distance[start_y * grid.width + start_x] = 0;

    // Overall idea: Go around the loop twice, first in one direction and then the other.
    // Update the maximum distances to each point along the loop as we go.
    // Once done, every point should have its maximum distance set.

    Point p = { .x = start_x, .y = start_y };
    Point start_neighbor_1, start_neighbor_2;
    get_neighbor_pipes(grid, p, &start_neighbor_1, &start_neighbor_2);
    // start going into the direction of neighbor 2 in the first iteration
    Point p_last = start_neighbor_2;
    for (int run = 0; run < 2; run++) {
        do {
            if (grid.cells[p.y * grid.width + p.x] == '|') {
                Point left = { .x = p.x - 1, .y = p.y };
                Point right = { .x = p.x + 1, .y = p.y };
                if (in_bounds(&grid, left)) {
                    /* printf("Point attached left to curve: (%d, %d)\n", left.x, left.y); */
                }
                if (in_bounds(&grid, right)) {
                    /* printf("Point attached right to curve: (%d, %d)\n", right.x, right.y); */
                }
            }
            loop_boundary[p.y * grid.width + p.x] = true;
            Point n1, n2;
            get_neighbor_pipes(grid, p, &n1, &n2);
            if (n1.x == p_last.x && n1.y == p_last.y) {
                // Chose n2 as next
                p_last = p;
                p = n2;
            } else {
                p_last = p;
                p = n1;
            }
            distance[p.y * grid.width + p.x] = MIN(
                    distance[p.y * grid.width + p.x],
                    distance[p_last.y * grid.width + p_last.x] + 1
                    );
        } while (!(p.x == start_x && p.y == start_y));

        // for the second run, go in the other direction
        p_last = start_neighbor_1;
    }

    dump_boundary(loop_boundary, grid.width, grid.height);

    int inside_count = 0;
    for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++) {
            Point p = { x, y };
            if (is_point_inside(&grid, p, loop_boundary)) {
                printf("(%d, %d) is inside\n", p.x, p.y);
                inside_count++;
            }
        }
    }
    printf("inside_count = %d\n", inside_count);

    // Find maximum distance
    int max_d = 0;
    for (int i = 0; i < grid.width * grid.height; i++) {
        if (distance[i] > max_d && distance[i] < INT_MAX) {
            max_d = distance[i];
        }
    }
    int part_1 = max_d;
    printf("%d\n", part_1);

    free(distance);
    free(loop_boundary);
    free(grid.cells);
    free((void *) input.data);
}
