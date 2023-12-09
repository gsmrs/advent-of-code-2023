#include "advent.h"

#include <stdlib.h>

void parse_ints(str input, IntVec *iv) {
    str delim = cstr(" ");
    while (!str_empty(input)) {
        str token = str_trim(str_next_token(&input, delim));
        if (str_empty(token)) {
            continue;
        }
        int value;
        assert(str_parse_int(token, &value));
        iv_push(iv, value);
    }
}

int64_t count_cards_part_2_helper(IntVec matches, size_t index, IntVec *cache) {
    if (cache->data[index] != -1) {
        int64_t result = cache->data[index];
        return result;
    } else {
        int64_t result = 1;
        int64_t win_count = matches.data[index];
        if (win_count > 0) {
            for (int64_t i = 1; i <= win_count; i++) {
                result += count_cards_part_2_helper(matches, index + i, cache);
            }
        }
        cache->data[index] = result;
        return result;
    }
}

int64_t count_cards_part_2(IntVec matches) {
    IntVec cache;
    iv_init(&cache, matches.count);
    for (size_t i = 0; i < matches.count; i++) {
        iv_push(&cache, -1);
    }

    int64_t result = 0;
    for (size_t i = 0; i < matches.count; i++) {
        result += count_cards_part_2_helper(matches, i, &cache);
    }
    iv_free(&cache);

    return result;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    str line;

    IntVec winning, have;
    iv_init(&winning, 64);
    iv_init(&have,    64);

    IntVec matches;
    iv_init(&matches, 256);

    int part_1 = 0; // sum of scores
    FOREACH_LINE(input, line) {
        str dummy, numbers;
        str_split(line, cstr(":"), &dummy, &numbers);

        str winning_s, have_s;
        str_split(numbers, cstr(" | "), &winning_s, &have_s);
        winning_s = str_trim(winning_s);
        have_s = str_trim(have_s);
        iv_clear(&winning);
        parse_ints(winning_s, &winning);

        iv_clear(&have);
        parse_ints(have_s, &have);

        int score = 0;
        int match_count = 0;
        for (size_t i = 0; i < have.count; i++) {
            if (iv_contains(&winning, have.data[i])) {
                match_count++;
                score = (score == 0) ? 1 : (score * 2);
            }
        }
        iv_push(&matches, match_count);
        part_1 += score;
    }
    iv_free(&winning);
    iv_free(&have);

    printf("%d\n", part_1);

    int64_t part_2 = count_cards_part_2(matches);
    printf("%ld\n", part_2);

    iv_free(&matches);
}
