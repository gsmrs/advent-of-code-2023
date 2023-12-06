#include "advent.h"
#include <ctype.h>
#include <time.h>
#include <math.h>

// brute force solution
int64_t count_winning_options(int64_t target_time, int64_t target_distance) {
    int64_t num_options = 0;
    for (int64_t button_time = 0; button_time < target_time; button_time++) {
        int64_t velocity = button_time;
        int64_t race_time = target_time - button_time;
        int64_t distance_covered = velocity * race_time;
        if (distance_covered > target_distance) {
            num_options++;
        }
    }
    return num_options;
}

// 'mathematical solution'.
// We want to solve race_time * velocity > target_distance, i.e.
// (target_time - button_time) * button_time > target_distance.
// This equation has roots at button_time = target_time / 2 +- sqrt((target_time / 2)^2 - target_distance).
// Counting the options requires a bit of checking at the boundaries whether we hit `target_distance` exactly.
int64_t count_winning_options2(int64_t target_time, int64_t target_distance) {
    double disc = sqrt((double) target_time * (double) target_time / 4.0 - (double) target_distance);
    int64_t t_min = (int64_t) ceil((double) target_time / 2.0 - disc);
    int64_t t_max = (int64_t) floor((double) target_time / 2.0 + disc);
    int64_t result = t_max - t_min + 1;
    if ((target_time - t_min) * t_min == target_distance) result--;
    if ((target_time - t_max) * t_max == target_distance) result--;
    return result;
}

int64_t parse_num_part_2(str s) {
    int64_t result = 0;
    for (size_t i = 0; i < s.len; i++)  {
        if (isdigit(s.data[i])) {
            result = 10 * result + (s.data[i] - '0');
        }
    }
    return result;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    str time_line = str_next_token(&input, cstr("\n"));
    str dist_line = str_next_token(&input, cstr("\n"));

    str dummy;
    str time_str, dist_str;
    str_split(time_line, cstr(":"), &dummy, &time_str);
    str_split(dist_line, cstr(":"), &dummy, &dist_str);
    time_str = str_trim(time_str);
    dist_str = str_trim(dist_str);

    int64_t time_part_2 = parse_num_part_2(time_str);
    int64_t dist_part_2 = parse_num_part_2(dist_str);

    int64_t part_1 = 1; // product of options to beat the record
    while (!str_empty(time_str)) {
        str time_, dist_;
        do {
            time_ = str_trim(str_next_token(&time_str, cstr(" ")));
        } while(str_empty(time_));
        do {
            dist_ = str_trim(str_next_token(&dist_str, cstr(" ")));
        } while(str_empty(dist_));

        int64_t time, dist;
        assert(str_parse_int64(time_, &time));
        assert(str_parse_int64(dist_, &dist));

        int64_t options = count_winning_options2(time, dist);
        part_1 *= options;
    }
    printf("%ld\n", part_1);

    clock_t start = clock();
    int64_t part_2 = count_winning_options2(time_part_2, dist_part_2);
    double elapsed = ((double) clock() - (double) start) / (double) CLOCKS_PER_SEC;
    fprintf(stderr, "Part 2 took %lf seconds\n", elapsed);
    printf("%ld\n", part_2);
}
