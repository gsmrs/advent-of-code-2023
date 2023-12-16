#include "advent.h"
#include <stdlib.h>
#include <string.h>

typedef struct Cell {
    bool energized;
    bool visited[4]; // indexed by `Direction`
    uint8_t ch;
} Cell;

typedef struct {
    int width, height;
    Cell *cells;
} Grid;

typedef enum {
    RIGHT,
    UP,
    LEFT,
    DOWN
} Direction;

int offset_x(int x, Direction direction) {
    switch (direction) {
        case RIGHT: return x + 1;
        case LEFT: return x - 1;
        default: return x;
    }
}

int offset_y(int y, Direction direction) {
    switch (direction) {
        case UP: return y - 1;
        case DOWN: return y + 1;
        default: return y;
    }
}

void trace_beam(Grid *grid, int x, int y, Direction direction) {
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return;
    }
    Cell *cell = &grid->cells[y * grid->width + x];
    if (cell->visited[direction]) {
        // path already taken, nothing more to discover..
        return;
    }
    cell->energized = true;
    cell->visited[direction] = true;

    switch (cell->ch) {
        case '.':
            trace_beam(grid,
                    offset_x(x, direction),
                    offset_y(y, direction),
                    direction);
            break;
        case '/':
            switch (direction) {
                case RIGHT:
                    trace_beam(grid, x, y - 1, UP);
                    break;
                case UP:
                    trace_beam(grid, x + 1, y, RIGHT);
                    break;
                case LEFT:
                    trace_beam(grid, x, y + 1, DOWN);
                    break;
                case DOWN:
                    trace_beam(grid, x - 1, y, LEFT);
                    break;
            }
            break;
        case '\\':
            switch (direction) {
                case RIGHT:
                    trace_beam(grid, x, y + 1, DOWN);
                    break;
                case UP:
                    trace_beam(grid, x - 1, y, LEFT);
                    break;
                case LEFT:
                    trace_beam(grid, x, y - 1, UP);
                    break;
                case DOWN:
                    trace_beam(grid, x + 1, y, RIGHT);
                    break;
            }
            break;
        case '|':
            switch (direction) {
                case DOWN:
                case UP:
                    trace_beam(grid,
                            offset_x(x, direction),
                            offset_y(y, direction),
                            direction);
                    break;
                case RIGHT:
                case LEFT:
                    trace_beam(grid, x, y - 1, UP);
                    trace_beam(grid, x, y + 1, DOWN);
                    break;
            }
            break;
        case '-':
            switch (direction) {
                case LEFT:
                case RIGHT:
                    trace_beam(grid,
                            offset_x(x, direction),
                            offset_y(y, direction),
                            direction);
                    break;
                case UP:
                case DOWN:
                    trace_beam(grid, x - 1, y, LEFT);
                    trace_beam(grid, x + 1, y, RIGHT);
                    break;
            }
            break;
        default:
            ASSERT(0 && "not reachable");
    }
}

void reset_grid(Grid *g) {
    for (int i = 0; i < g->width * g->height; i++) {
        memset(g->cells[i].visited, 0, sizeof(g->cells[i].visited));
        g->cells[i].energized = false;
    }
}

int count_energized(const Grid *grid) {
    int result = 0;
    for (int i = 0; i < grid->width * grid->height; i++) {
        result += (int) grid->cells[i].energized;
    }
    return result;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    CharGrid char_grid = read_char_grid(input);

    Grid grid = {
        .width = char_grid.width,
        .height = char_grid.height,
        .cells = calloc(char_grid.width * char_grid.height, sizeof(Cell))
    };
    for (int i = 0; i < char_grid.width * char_grid.height; i++) {
        grid.cells[i].ch = char_grid.cells[i];
    }

    trace_beam(&grid, 0, 0, RIGHT);

    // count energized tiles
    int part_1 = count_energized(&grid);
    printf("%d\n", part_1);

    // Part 2
    int part_2 = 0;

    for (int x = 0; x < grid.width; x++) {
        // top edge
        reset_grid(&grid);
        trace_beam(&grid, x, 0, DOWN);
        part_2 = MAX(part_2, count_energized(&grid));
        // bottom edge
        reset_grid(&grid);
        trace_beam(&grid, x, grid.height - 1, UP);
        part_2 = MAX(part_2, count_energized(&grid));
    }

    for (int y = 0; y < grid.height; y++) {
        // left edge
        reset_grid(&grid);
        trace_beam(&grid, 0, y, RIGHT);
        part_2 = MAX(part_2, count_energized(&grid));
        // right edge
        reset_grid(&grid);
        trace_beam(&grid, grid.width - 1, y, LEFT);
        part_2 = MAX(part_2, count_energized(&grid));
    }
    printf("%d\n", part_2);

    free(char_grid.cells);
    free(grid.cells);
    free(input.data);
}
