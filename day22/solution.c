#include "advent.h"
#include <stdlib.h>
#include <limits.h>
#include <string.h>

typedef struct Point {
    int x, y, z;
} Point;

typedef struct Block {
    Point start, end;
    int id;
} Block;

Point parse_point(str s) {
    str xs = str_next_token(&s, cstr(","));
    str ys = str_next_token(&s, cstr(","));
    str zs = str_next_token(&s, cstr(","));
    Point p;
    ASSERT(str_parse_int(xs, &p.x));
    ASSERT(str_parse_int(ys, &p.y));
    ASSERT(str_parse_int(zs, &p.z));
    return p;
}

Block parse_block(str s) {
    static int id = 0;
    Block block = {
        .start = parse_point(str_next_token(&s, cstr("~"))),
        .end = parse_point(str_next_token(&s, cstr("~"))),
    };
    int ndims = 0;
    ndims += block.start.x != block.end.x;
    ndims += block.start.y != block.end.y;
    ndims += block.start.z != block.end.z;
    ASSERT(ndims <= 1);
    block.id = id++;
    return block;
}

typedef struct PosBucket {
    struct PosBucket *next;
    Point p;
    void *value;
} PosBucket;

typedef struct {
    PosBucket *buckets[1024];
} PosMap;

int hash_point(Point p) {
    return 1337 * p.x + 17 * p.y + p.z;
}

void pm_clear(PosMap *pm) {
    for (int i = 0; i < 1024; i++) {
        for (PosBucket *b = pm->buckets[i]; b; ) {
            PosBucket *to_delete = b;
            b = b->next;
            free(to_delete);
        }
        pm->buckets[i] = NULL;
    }
}

void pm_insert(PosMap *pm, Point p, void *value) {
    uint64_t index = hash_point(p) & 1023;
    for (PosBucket *b = pm->buckets[index]; b; b = b->next) {
        if (memcmp(&b->p, &p, sizeof(p)) == 0) {
            b->value = value;
            return;
        }
    }

    PosBucket *bucket = calloc(1, sizeof(*bucket));
    bucket->next = pm->buckets[index];
    bucket->p = p;
    bucket->value = value;
    pm->buckets[index] = bucket;
}

void pm_insert_block(PosMap *pm, Block block, void *value) {
    for (int x = block.start.x; x <= block.end.x; x++) {
        for (int y = block.start.y; y <= block.end.y; y++) {
            for (int z = block.start.z; z <= block.end.z; z++) {
                pm_insert(pm, (Point) { x, y, z }, value);
            }
        }
    }
}

void *pm_get(PosMap *pm, Point p) {
    uint64_t index = hash_point(p) & 1023;
    for (PosBucket *b = pm->buckets[index]; b; b = b->next) {
        if (memcmp(&b->p, &p, sizeof(p)) == 0) {
            return b->value;
        }
    }
    return NULL;
}

void debug_print_xz(PosMap *pm, Point min, Point max) {
    // Print projection onto XZ plane
    for (int z = max.z; z >= min.z; z--) {
        for (int x = min.x; x <= max.x; x++) {
            Block *block = NULL;
            for (int y = min.y; y <= max.y; y++) {
                Point p = { x, y, z };
                if ((block = pm_get(pm, p))) {
                    break;
                }
            }
            if (block) {
                putchar('A' + block->id);
            } else {
                putchar('.');
            }
        }
        puts("");
    }
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    str lines = input;
    PosMap pm_front = {0}, pm_back = {0};

    PosMap *front = &pm_front;
    PosMap *back = &pm_back;

    enum { NUM_MAX_BLOCKS = 2048 };
    Block blocks[NUM_MAX_BLOCKS];
    int block_count = 0;

    Point min = { INT_MAX, INT_MAX, INT_MAX };
    Point max = { INT_MIN, INT_MIN, INT_MIN };
    while (!str_empty(lines)) {
        str line = str_next_token(&lines, cstr("\n"));
        if (str_empty(line)) {
            continue;
        }

        ASSERT(block_count < NUM_MAX_BLOCKS);
        Block block = blocks[block_count] = parse_block(line);
        for (int x = block.start.x; x <= block.end.x; x++) {
            for (int y = block.start.y; y <= block.end.y; y++) {
                for (int z = block.start.z; z <= block.end.z; z++) {
                    min.x = MIN(min.x, x);
                    min.y = MIN(min.y, y);
                    min.z = MIN(min.z, z);

                    max.x = MAX(max.x, x);
                    max.y = MAX(max.y, y);
                    max.z = MAX(max.z, z);

                    Point p = { x, y, z };
                    pm_insert(front, p, &blocks[block_count]);
                }
            }
        }
        block_count++;
    }
    /* printf("min = { %d, %d, %d }\n", min.x, min.y, min.z); */
    /* printf("max = { %d, %d, %d }\n", max.x, max.y, max.z); */

    /* debug_print_xz(front, min, max); */

    // support[i * block_count + j] is true iff block i supports block j
    bool *support = calloc(block_count * block_count, sizeof(*support));

    for (;;) {
        pm_clear(back);

        memset(support, 0, block_count * block_count * sizeof(*support));

        int blocks_updated = 0;
        for (int i = 0; i < block_count; i++) {
            Block *b = &blocks[i];
            // check if block can move downwards
            bool can_fall = true;
            for (int x = b->start.x; x <= b->end.x; x++) {
                for (int y = b->start.y; y <= b->end.y; y++) {
                    for (int z = b->start.z; z <= b->end.z; z++) {
                        Point point_below = { x, y, z - 1};
                        Block *below = pm_get(front, point_below);
                        can_fall &= z > 1;
                        if (below && (below->id != b->id)) {
                            support[below->id * block_count + b->id] = true;
                            can_fall = false;
                        }
                    }
                }
            }
            if (can_fall) {
                b->start.z--;
                b->end.z--;
                /* printf("Block %d can fall.\n", b->id); */
                blocks_updated++;
            }
            pm_insert_block(back, *b, b);
        }

        // switch buffers
        PosMap *tmp = back;
        back = front;
        front = tmp;

        if (blocks_updated == 0) {
            break;
        }
    }

    int part_1 = 0;
    for (int i = 0; i < block_count; i++) {
        // Check if block i can be disintegrated.
        // This is the case iff all children of i are supported by at least one other block apart from i.
        bool can_disintegrate = true;
        for (int child_index = 0; child_index < block_count; child_index++) {
            if (support[i * block_count + child_index]) {
                int num_parents = 0;
                for (int parent_index = 0; parent_index < block_count; parent_index++) {
                    num_parents += support[parent_index * block_count + child_index];
                }
                ASSERT(num_parents >= 1);
                if (num_parents == 1) {
                    can_disintegrate = false;
                }
            }
        }
        part_1 += can_disintegrate;
        /* if (can_disintegrate) { */
        /*     printf("Can disintegrate block %d\n", blocks[i].id); */
        /* } */
    }
    printf("%d\n", part_1);

    /* debug_print_xz(front, min, max); */

    free(support);
    pm_clear(front);
    pm_clear(back);
    free(input.data);
}
