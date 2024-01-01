#include "advent.h"
#include <ctype.h>
#include <stdlib.h>


int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);

    str input = read_file(input_file);

    const str colon = cstr(":");
    const str semicolon = cstr(";");
    const str comma = cstr(", ");

    const str RED   = cstr("red");
    const str GREEN = cstr("green");
    const str BLUE  = cstr("blue");

    // inventory of available cube numbers
    const int INV_RED   = 12;
    const int INV_GREEN = 13;
    const int INV_BLUE  = 14;

    int part_1 = 0; // sum of IDs of possible games
    int part_2 = 0; // sum of products of numbers of minimal numbers of cubes to make game possible

    str line;
    FOREACH_LINE(input, line) {
        // parse game index
        int game_index;
        str game_name = str_next_token(&line, colon);
        str dummy, game_index_str;
        str_split(game_name, cstr(" "), &dummy, &game_index_str);
        assert(str_parse_int(game_index_str, &game_index));

        line = str_trim_left(line);

        str cubes;
        bool is_possible = true;
        int max_red   = 0;
        int max_green = 0;
        int max_blue  = 0;
        while (!str_empty(cubes = str_next_token(&line, semicolon))) {
            cubes = str_trim_left(cubes);

            str cube;
            int red   = 0;
            int green = 0;
            int blue  = 0;

            while (!str_empty(cube = str_next_token(&cubes, comma))) {
                str count_s, color;
                str_split(cube, cstr(" "), &count_s, &color);
                int count;
                assert(str_parse_int(count_s, &count));

                if (str_eq(color, RED)) {
                    red = count;
                } else if (str_eq(color, GREEN)) {
                    green = count;
                } else if (str_eq(color, BLUE)) {
                    blue = count;
                } else {
                    log_fatal("unknown color: " STR_FMT, STR_ARG(color));
                    exit(-1);
                }
            }
            max_red   = MAX(max_red,   red);
            max_green = MAX(max_green, green);
            max_blue  = MAX(max_blue,  blue);
            /* log_info("R = %d, G = %d, B = %d", red, green, blue); */

            is_possible &= ((red <= INV_RED) && (green <= INV_GREEN) && (blue <= INV_BLUE));
        }
        if (is_possible) {
            part_1 += game_index;
        }
        part_2 += max_red * max_green * max_blue;
    }

    printf("%d\n", part_1);
    printf("%d\n", part_2);
}
