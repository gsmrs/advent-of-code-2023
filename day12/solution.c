#include "advent.h"
#include <stdlib.h>

bool parse_numbers(str nums_str, IntVec *result) {
    while (!str_empty(nums_str)) {
        str num_str = str_next_token(&nums_str, cstr(","));
        int64_t value;
        if (!str_parse_int64(num_str, &value)) {
            return false;
        }
        iv_push(result, value);
    }
    return true;
}

bool is_solution(str line, IntVec *numbers) {
    int run_length = 0;
    size_t number_index = 0;
    for (size_t i = 0; i < line.len; i++) {
        if (line.data[i] == '#') {
            run_length++;
        } else {
            if (run_length > 0) {
                if (number_index >= numbers->count) {
                    return false;
                }
                else if (run_length != numbers->data[number_index]) {
                    return false;
                }
                else {
                    number_index++;
                }
            }
            run_length = 0;
        }
    }
    if (number_index == numbers->count - 1) {
        return (numbers->data[number_index] == run_length);
    } else if (number_index == numbers->count) {
        return run_length == 0;
    } else {
        return false;
    }
}

int64_t count_solutions_helper(str pattern, IntVec *numbers, size_t index) {
    if (index == pattern.len) {
        bool ok = is_solution(pattern, numbers);
        /* printf("is_solution(" STR_FMT ") -> %d\n", STR(pattern), ok); */
        return (int64_t) ok;
    }

    uint8_t orig = pattern.data[index];
    if (orig == '?') {
        int64_t result = 0;

        pattern.data[index] = '.';
        result += count_solutions_helper(pattern, numbers, index + 1);

        pattern.data[index] = '#';
        result += count_solutions_helper(pattern, numbers, index + 1);

        pattern.data[index] = orig;

        return result;
    } else {
        return count_solutions_helper(pattern, numbers, index + 1);
    }
}

int64_t count_solutions(str pattern, IntVec *numbers) {
    return count_solutions_helper(pattern, numbers, 0);
}

static int FOO = 0;

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);
    str lines = input;
    IntVec numbers;
    iv_init(&numbers, 1024);
    int64_t part_1 = 0;

    while (!str_empty(lines)) {
        str line = str_next_token(&lines, cstr("\n"));

        str pattern, num_str;
        str_split(line, cstr(" "), &pattern, &num_str);
        iv_clear(&numbers);
        assert(parse_numbers(num_str, &numbers));

        int64_t solutions = count_solutions(pattern, &numbers);
        /* printf(STR_FMT " -> %ld\n", STR(line), solutions); */
        part_1 += solutions;

        FOO++;
    }
    printf("%ld\n", part_1);

    free((void *) input.data);
}
