#include "advent.h"

void get_digits_part1(str line, int * restrict first_digit, int * restrict last_digit)  {
    *first_digit = -1;
    *last_digit  = -1;
    for (size_t i = 0; i < line.len; i++) {
        int digit = -1;
        if (is_digit(line.data[i])) {
            digit = line.data[i] - '0';
            if (*first_digit == -1) *first_digit = digit;
            *last_digit = digit;
        }
    }
}

void get_digits_part2(str line, int * restrict first_digit, int * restrict last_digit)  {
    *first_digit = -1;
    *last_digit  = -1;

    str spelled_out[9] = {
        cstr("one"),
        cstr("two"),
        cstr("three"),
        cstr("four"),
        cstr("five"),
        cstr("six"),
        cstr("seven"),
        cstr("eight"),
        cstr("nine"),
    };

    for (size_t i = 0; i < line.len; i++) {
        int digit = -1;
        if (is_digit(line.data[i])) {
            digit = line.data[i] - '0';
        } else {
            str remainder = str_sub(line, i, line.len);
            for (int j = 0; j < 9; j++) {
                if (str_starts_with(remainder, spelled_out[j])) {
                    digit = j + 1;
                    break;
                }
            }
        }
        if (digit != -1) {
            if (*first_digit == -1) *first_digit = digit;
            *last_digit = digit;
        }
    }
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);

    str input = read_file(input_file);
    input = str_trim(input);

    int part_1 = 0, part_2 = 0;
    str line;
    int line_num = 1;
    FOREACH_LINE(input, line) {
        int first_digit = -1, last_digit = -1;

        get_digits_part1(line, &first_digit, &last_digit);
        part_1 += 10 * first_digit + last_digit;

        get_digits_part2(line, &first_digit, &last_digit);
        part_2 += 10 * first_digit + last_digit;

        line_num++;
    }

    printf("%d\n", part_1);
    printf("%d\n", part_2);
}
