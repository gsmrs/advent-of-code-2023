#include "advent.h"
#include <string.h>

typedef struct {
    char name[3];
} Node;

typedef struct Rule {
    Node left, right;
} Rule;

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
            printf("Have start node: " STR_FMT "\n", STR_ARG(lhs));
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

    // Part 2
    instruction_index = 0;
    int part_2 = 0;
    for (;;) {
        char instruction = instructions.data[instruction_index];

        int finish_count = 0;
        for (int i = 0; i < start_node_count; i++) {
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
            }
        }
        instruction_index = (instruction_index + 1) % instructions.len;
        part_2++;

        if (finish_count == start_node_count) {
            break;
        }
    }
    printf("%d\n", part_2);
}
