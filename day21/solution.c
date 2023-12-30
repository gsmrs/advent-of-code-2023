#include "advent.h"
#include <stdlib.h>

uint64_t int_hash_func(void *key) {
    int value = (int) (uintptr_t) key;
    return (uint64_t) value;
}

bool int_eq(void *key1, void *key2) {
    return (int) (uintptr_t) key1 == (int) (uintptr_t) key2;
}

typedef struct {
    TrashMap *next;
    CharGrid *grid;
} Context;

static const int OFFSETS[4][2] = {
    {-1, 0},
    {1, 0},
    {0, -1},
    {0, 1}
};

void step_callback(void *key, uintptr_t value, void *pcontext) {
    (void) value;
    Context *context = pcontext;
    int width = context->grid->width;
    int height = context->grid->height;
    int index = (int) (uintptr_t) key;
    int x = index % width;
    int y = index / width;

    for (int i = 0; i < 4; i++) {
        int dx = OFFSETS[i][0];
        int dy = OFFSETS[i][1];

        int x_ = x + dx;
        int y_ = y + dy;

        bool can_move = true;
        if (x_ >= 0 && y_ >= 0 && x_ < width && y_ < height) {
            if (context->grid->cells[y_ * width  + x_] == '#') {
                can_move = false;
            }
        }

        if (can_move) {
            trashmap_insert(context->next, (void *) (uintptr_t) (y_ * width + x_), 1);
        }
    }
}

void count_callback(void *key, uintptr_t value, void *pcontext) {
    (void) key;
    (void) value;
    int *square_count = pcontext;
    (*square_count)++;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);
    CharGrid grid = read_char_grid(input);

    int sx = -1, sy = -1;
    for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++) {
            if (grid.cells[y * grid.width + x] == 'S') {
                sx = x;
                sy = y;
                goto start_found;
            }
        }
    }
start_found:
    ASSERT(sx != -1);
    ASSERT(sy != -1);

    TrashMap current, next;
    trashmap_init(&current, 1024, int_hash_func, int_eq);
    trashmap_init(&next, 1024, int_hash_func, int_eq);
    TrashMap *pcurrent = &current, *pnext = &next;

    trashmap_insert(&current, (void *) (uintptr_t) (sy * grid.width + sx), 1);
    for (int iter = 0; iter < 64; iter++) {
        trashmap_clear(pnext);
        Context context = { .grid = &grid, .next = pnext };
        trashmap_iter(pcurrent, step_callback, &context);

        // swap buffers
        TrashMap *tmp = pnext;
        pnext = pcurrent;
        pcurrent = tmp;
    }
    int part_1 = 0;
    trashmap_iter(pnext, count_callback, &part_1);
    printf("%d\n", part_1);

    trashmap_free(&current);
    trashmap_free(&next);

    free(grid.cells);
    free(input.data);
}
