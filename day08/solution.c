#include "advent.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    char name[3];
} Node;

typedef struct Rule {
    Node left, right;
} Rule;

typedef struct {
    struct NodeBucket {
        Node node;
        int instruction_index;
        int iteration;
        struct NodeBucket *next;
    } *nodes[1024];
} NodeCache;

void node_cache_insert(NodeCache *cache, int instruction_index, Node node, int iteration) {
    uint32_t hash = (instruction_index ^ ((node.name[1] << 16) | (node.name[1] << 8) | node.name[0]));
    uint32_t bucket_index = hash & 1023;
    for (struct NodeBucket *b = cache->nodes[bucket_index]; b; b = b->next) {
        if (b->instruction_index == instruction_index && memcmp(&b->node, &node, sizeof(Node)) == 0) {
            b->iteration = iteration;
            return;
        }
    }

    struct NodeBucket *bucket = malloc(sizeof(*bucket));
    bucket->node = node;
    bucket->instruction_index = instruction_index;
    bucket->iteration = iteration;
    bucket->next = cache->nodes[bucket_index];
    cache->nodes[bucket_index] = bucket;
}

struct NodeBucket *node_cache_get(NodeCache *cache, int instruction_index, Node node) {
    uint32_t hash = (instruction_index ^ ((node.name[1] << 16) | (node.name[1] << 8) | node.name[0]));
    uint32_t bucket_index = hash & 1023;
    for (struct NodeBucket *b = cache->nodes[bucket_index]; b; b = b->next) {
        if (b->instruction_index == instruction_index && memcmp(&b->node, &node, sizeof(Node)) == 0) {
            return b;
        }
    }
    return NULL;
}

void node_cache_free(NodeCache *cache) {
    for (int i = 0; i < 1024; i++) {
        for (struct NodeBucket *b = cache->nodes[i]; b; ) {
            struct NodeBucket *to_free = b;
            b = b->next;
            free(to_free);
        }
    }
}


static Rule map[26 * 26 * 26];

size_t INDEX(Node node) {
    return ((node.name[0] - 'A') * 26 * 26)
        + ((node.name[1] - 'A') * 26)
        + (node.name[2] - 'A');
}

size_t INDEX_STR(str s) {
    Node node;
    memcpy(node.name, s.data, 3);
    return INDEX(node);
}

Node node_from_str(str s) {
    assert(s.len == 3);
    Node node;
    memcpy(node.name, s.data, 3);
    return node;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    str instructions = str_next_token(&input, cstr("\n"));
    (void) str_next_token(&input, cstr("\n"));

    enum { NUM_MAX_START_NODES = 16 };
    Node start_nodes[NUM_MAX_START_NODES];
    int start_node_count = 0;

    while (!str_empty(input)) {
        str line = str_next_token(&input, cstr("\n"));
        if (str_empty(line = str_trim(line))) {
            continue;
        }

        str lhs, rhs;
        str_split(line, cstr(" = "), &lhs, &rhs);

        if (lhs.data[2] == 'A') {
            assert(start_node_count < NUM_MAX_START_NODES);
            start_nodes[start_node_count++] = node_from_str(lhs);
            printf("Have start node: " STR_FMT "\n", STR(lhs));
        }

        str left, right;
        str_split(rhs, cstr(", "), &left, &right);
        left = str_sub(left, 1, left.len);
        right = str_sub(right, 0, right.len - 1);

        map[INDEX_STR(lhs)].left = node_from_str(left);
        map[INDEX_STR(lhs)].right = node_from_str(right);
    }

    int instruction_index;
#if 0
    int part_1 = 0;
    Node current = { .name = "AAA" };
    instruction_index = 0;
    for (;;) {
        char instruction = instructions.data[instruction_index];
        /* printf("%.*s\n", 3, current); */

        Rule *rule = &map[INDEX(current)];
        if (instruction == 'L') {
            current = rule->left;
        } else if (instruction == 'R') {
            current = rule->right;
        } else {
            assert(0 && "not reachable");
        }
        part_1++;

        if (memcmp(current.name, "ZZZ", 3) == 0) {
            break;
        }
        instruction_index = (instruction_index + 1) % instructions.len;
    }
    printf("%d\n", part_1);
#endif

    NodeCache *caches = calloc(start_node_count, sizeof(NodeCache));

    // Part 2
    instruction_index = 0;
    int part_2 = 0;
    int iteration = 0;

    bool *cycles_detected = calloc(start_node_count, sizeof(bool));

    for (;;iteration++) {
        char instruction = instructions.data[instruction_index];

        int finish_count = 0;
        /* printf("\n"); */
        /* printf("Iteration %d -> %c\n", iteration, instruction); */
        for (int i = 0; i < start_node_count; i++) {
            /* printf("Ghost %d at %.*s\n", i, 3, start_nodes[i].name); */
            Rule *rule = &map[INDEX(start_nodes[i])];
            if (instruction == 'L') {
                start_nodes[i] = rule->left;
            } else if (instruction == 'R') {
                start_nodes[i] = rule->right;
            } else {
                assert(0 && "not reachable");
            }

            if (start_nodes[i].name[2] == 'Z') {
                finish_count++;

                struct NodeBucket *last_seen = node_cache_get(&caches[i], instruction_index, start_nodes[i]);
                if (last_seen) {
                    int cycle_len = iteration - last_seen->iteration;
                    printf("Ghost %d was already seen on iteration %d: %d + n * %d\n", i, last_seen->iteration, last_seen->iteration, cycle_len);
                    cycles_detected[i] = true;
                } else {
                    node_cache_insert(&caches[i], instruction_index, start_nodes[i], iteration);
                }
            }
        }
        instruction_index = (instruction_index + 1) % instructions.len;
        part_2++;

        bool all_detected = true;
        for (int i = 0; i < start_node_count; i++) all_detected &= cycles_detected[i];
        if (all_detected) {
            printf("all_detected\n");
            break;
        }

        if (finish_count == start_node_count) {
            break;
        }
    }
    printf("%d\n", part_2);

    for (int i = 0; i < start_node_count; i++) {
        node_cache_free(&caches[i]);
    }
    free(caches);

}
