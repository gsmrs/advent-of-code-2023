#include "advent.h"

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    IntVec iv;
    iv_init(&iv, 1024);
    int64_t part_1 = 0;
    int64_t part_2 = 0;
    while (!str_empty(input)) {
        str line = str_next_token(&input, cstr("\n"));

        iv_clear(&iv);
        while (!str_empty(line)) {
            str num_str = str_next_token(&line, cstr(" "));
            int64_t value;
            assert(str_parse_int64(num_str, &value));
            iv_push(&iv, value);
            /* printf("%ld ", value); */
        }
        /* puts(""); */
        /* printf("%zu\n", iv.count); */

        int seq_len = iv.count;
        int count = iv.count;
        bool all_zero = true;
        size_t parent_offset = 0;
        do {
            all_zero = true;
            for (int i = 1; i < count; i++) {
                int64_t diff = iv.data[parent_offset + i] - iv.data[parent_offset + i - 1];
                all_zero &= diff == 0;
                /* printf("%ld ", diff); */
                iv_push(&iv, diff);
            }
            /* puts(""); */
            parent_offset += count;
            count--;
        } while (count > 0 && !all_zero);

        int last_row_len = count;

        int64_t new_value = 0;
        int64_t end_of_row_index = iv.count - 1;
        const int iterations = (int) seq_len - last_row_len;
        for (int i = 0; i < iterations; i++) {
            end_of_row_index -= count;
            new_value = iv.data[end_of_row_index] + new_value;
            count++;
        }
        part_1 += new_value;

        new_value = 0;
        int64_t start_of_row_index = (int64_t) iv.count - last_row_len;
        count = last_row_len;
        for (int i = 0; i < iterations; i++) {
            count++;
            start_of_row_index -= count;
            new_value = iv.data[start_of_row_index] - new_value;
        }
        part_2 += new_value;

        /* puts(""); */
    }
    printf("%ld\n", part_1);
    printf("%ld\n", part_2);

    iv_free(&iv);
}
