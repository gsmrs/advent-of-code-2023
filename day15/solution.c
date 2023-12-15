#include "advent.h"
#include <stdlib.h>
#include <ctype.h>

typedef struct Lens {
    struct Lens *next;
    str label;
    int focal_length;
} Lens;

typedef struct {
    Lens *lens;
} Box;

typedef struct {
    Box boxes[256];
} Boxes;

uint8_t hash_string(str s) {
    uint8_t hash = 0;
    for (size_t i = 0; i < s.len; i++) {
        if (s.data[i] == '\n') {
            continue;
        }
        hash = (hash + s.data[i]) * 17;
    }
    return hash;
}

void boxes_remove(Boxes *boxes, str label) {
    uint8_t hash = hash_string(label);
    Lens *lens = boxes->boxes[hash].lens;
    Lens *last_lens = NULL;
    for (; lens; lens = lens->next) {
        if (str_eq(lens->label, label)) {
            if (last_lens) {
                last_lens->next = lens->next;
            }
            else {
                boxes->boxes[hash].lens = lens->next;
            }
            free(lens);
            return;
        }
        last_lens = lens;
    }
}

void boxes_insert(Boxes *boxes, str label, int focal_length) {
    uint8_t hash = hash_string(label);
    Lens *lens = boxes->boxes[hash].lens;
    Lens *last_lens = NULL;
    for (; lens; lens = lens->next) {
        if (str_eq(lens->label, label)) {
            lens->focal_length = focal_length;
            return;
        }
        last_lens = lens;
    }

    Lens *new_lens = calloc(1, sizeof(*new_lens));
    new_lens->label = label;
    new_lens->focal_length = focal_length;
    if (last_lens) {
        last_lens->next = new_lens;
    } else {
        boxes->boxes[hash].lens = new_lens;
    }
}

void boxes_free(Boxes *boxes) {
    for (int box_index = 0; box_index < 256; box_index++) {
        for (Lens *l = boxes->boxes[box_index].lens; l; ) {
            Lens *del = l;
            l = l->next;
            free(del);
        }
    }
}

int compute_focusing_power(Boxes *boxes) {
    int result = 0;
    for (int box_index = 0; box_index < 256; box_index++) {
        Lens *l;
        int li;
        for (li = 0, l = boxes->boxes[box_index].lens; l; li++, l = l->next) {
            result += (1 + box_index) * (1 + li) * l->focal_length;
        }
    }
    return result;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    ASSERT(hash_string(cstr("HASH")) == 52);

    Boxes boxes = {0};
    str tokens = input;
    int part_1 = 0;
    while (!str_empty(tokens)) {
        str token = str_next_token(&tokens, cstr(","));
        token = str_trim(token);
        if (str_empty(token)) {
            continue;
        }
        /* printf(STR_FMT "\n", STR(token)); */
        part_1 += (int) hash_string(token);

        size_t index = 0;
        while (index < token.len && isalpha(token.data[index])) {
            index++;
        }
        str label = str_sub(token, 0, index);
        if (token.data[index] == '-') {
            boxes_remove(&boxes, label);
        }
        else if (token.data[index] == '=') {
            str num_str = str_sub(token, index + 1, token.len);
            int focal_length;
            ASSERT(str_parse_int(num_str, &focal_length));
            boxes_insert(&boxes, label, focal_length);
        }
        else {
            ASSERT(0 && "not reachable");
        }
    }
    printf("%d\n", part_1);

    int part_2 = compute_focusing_power(&boxes);
    printf("%d\n", part_2);

    free(input.data);
    boxes_free(&boxes);
}
