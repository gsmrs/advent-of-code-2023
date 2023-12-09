#include "advent.h"
#include <limits.h>
#include <stdlib.h>

typedef struct Range {
    int64_t start;
    int64_t end;
} Range;

typedef struct Ranges {
    size_t count;
    size_t capacity;
    Range *ranges;
} Ranges;

void ranges_init(Ranges *ranges) {
    ranges->count = 0;
    ranges->capacity = 1024;
    ranges->ranges = malloc(ranges->capacity * sizeof(*ranges->ranges));
}

void ranges_clear(Ranges *ranges)  {
    ranges->count = 0;
}

void ranges_push(Ranges *ranges, Range range) {
    if (ranges->count == ranges->capacity) {
        ranges->capacity = MIN(ranges->capacity * 2, 8);
        ranges->ranges = realloc(ranges->ranges, ranges->capacity * sizeof(*ranges->ranges));
    }
    ranges->ranges[ranges->count++] = range;
}

void ranges_free(Ranges *ranges) {
    ranges->count = 0;
    ranges->capacity = 0;
    free(ranges->ranges);
}

bool range_empty(Range r) {
    return r.start >= r.end;
}

typedef struct {
    Range source;
    int64_t dst_start;
} Mapping;

int cmp_mapping(const void *pa, const void *pb) {
    const Mapping *a = pa;
    const Mapping *b = pb;
    if (a->source.start < b->source.start) return -1;
    if (a->source.start > b->source.start) return 1;
    return 0;
}

Mapping read_mapping(str line) {
    Mapping mapping;

    str delim = cstr(" ");
    assert(str_parse_int64(str_trim(str_next_token(&line, delim)), &mapping.dst_start));
    assert(str_parse_int64(str_trim(str_next_token(&line, delim)), &mapping.source.start));
    int64_t len;
    assert(str_parse_int64(str_trim(str_next_token(&line, delim)), &len));
    mapping.source.end = mapping.source.start + len;

    return mapping;
}

enum { NUM_MAX_MAPPINGS = 256 };

typedef struct {
    int count;
    Mapping mappings[NUM_MAX_MAPPINGS];
} Map;

int64_t map_lookup(const Map *map, int64_t input) {
    for (int i = 0; i < map->count; i++) {
        Range r = map->mappings[i].source;
        if (r.start <= input && input < r.end) {
            return map->mappings[i].dst_start + (input - r.start);
        }
    }
    return input;
}

void map_lookup_range(const Map *map, Range range, Ranges *result) {
    // We need to subdivide `range`.
    // We can assume that `map` contains mappings whose source ranges are sorted ascendingly.
    for (int i = 0; i < map->count; i++) {
        const Mapping *mapping = &map->mappings[i];

        Range below = { .start = range.start, .end = MIN(range.end, mapping->source.start) };
        if (!range_empty(below)) {
            // since `below` by construction is not inside any interval, it is identically mapped.
            Range mapped = below;
            ranges_push(result, mapped);
        }

        Range inside = {
            .start = MAX(range.start, mapping->source.start),
            .end = MIN(range.end, mapping->source.end)
        };
        if (!range_empty(inside)) {
            Range mapped = {
                .start = mapping->dst_start + (inside.start - mapping->source.start),
                .end = mapping->dst_start + (inside.end - mapping->source.start),
            };
            ranges_push(result, mapped);
        }

        Range remainder = {
            .start = MAX(range.start, mapping->source.end),
            .end = range.end
        };
        if (range_empty(remainder)) {
            // fully mapped
            return;
        } else {
            range = remainder;
        }
    }

    if (!range_empty(range)) {
        // remainder is outside any interval and therefore identically mapped.
        ranges_push(result, range);
    }
}

void map_compose(Map *a, Map *b, Map *result) {
    Ranges output;
    ranges_init(&output);
    result->count = 0;
    for (int i = 0; i < a->count; i++) {
        Range source = a->mappings[i].source;
        Range dest = {
            .start = a->mappings[i].dst_start,
            .end = a->mappings[i].dst_start + (source.end - source.start)
        };
        ranges_clear(&output);
        map_lookup_range(b, dest, &output);
        int64_t start = source.start;
        for (size_t j = 0; j < output.count; j++) {
            Range target = output.ranges[j];
            int64_t target_len = target.end - target.start;
            result->mappings[result->count++] = (Mapping) {
                .source = { .start = start, .end = start + target_len },
                .dst_start = target.start
            };
            start += target_len;
        }
        ASSERT(start == source.end);
    }
    ranges_free(&output);
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    str delim = cstr("\n\n");

    str seeds_line = str_next_token(&input, delim);
    str nothing, seed_values;
    str_split(seeds_line, cstr(": "), &nothing, &seed_values);

    enum { NUM_MAPS = 7 };
    Map maps[NUM_MAPS] = {0};

    Map total_map = {0};
    Map total_map_tmp = {0};

    // parse maps
    for (int map_index = 0; !str_empty(input) && map_index < NUM_MAPS; map_index++) {
        str block = str_next_token(&input, delim);

        str newline = cstr("\n");
        // skip first line
        (void) str_next_token(&block, newline);

        Map *map = &maps[map_index];
        while (!str_empty(block)) {
            str mapping_line = str_next_token(&block, newline);
            assert(map->count < NUM_MAX_MAPPINGS);
            map->mappings[map->count++] = read_mapping(mapping_line);
        }

        // sort mappings by start
        qsort(map->mappings, map->count, sizeof(Mapping), cmp_mapping);
    }

    // compose maps
    Map *a = &total_map, *b = &total_map_tmp;
    for (int i = 0; i < NUM_MAPS; i++) {
        map_compose(a, &maps[i], b);
        Map *tmp = a;
        a = b;
        b = tmp;
    }

    // Part 1:
    // iterate over seed values and map them to location
    int64_t part_1 = INT64_MAX;
    str seed_iter = seed_values;
    while (!str_empty(seed_iter)) {
        str seed_value_s = str_next_token(&seed_iter, cstr(" "));
        int64_t seed_value;
        assert(str_parse_int64(seed_value_s, &seed_value));
        /* printf("Hvae seed value %d\n", seed_value); */

        int64_t value = seed_value;
        for (int map_index = 0; map_index < NUM_MAPS; map_index++) {
            value = map_lookup(&maps[map_index], value);
        }
        /* printf("Location %d\n", value); */
        part_1 = MIN(part_1, value);
    }
    printf("%ld\n", part_1);

    // Part 2:
    // Use r1, r2 as double buffers (switching roles between input_ranges and output_ranges)
    Ranges r1, r2;
    ranges_init(&r1);
    ranges_init(&r2);
    Ranges *input_ranges = &r1, *output_ranges = &r2;

    // parse input seed ranges
    seed_iter = seed_values;
    while (!str_empty(seed_iter)) {
        str start_s = str_next_token(&seed_iter, cstr(" "));
        str len_s = str_next_token(&seed_iter, cstr(" "));

        int64_t start, len;
        assert(str_parse_int64(start_s, &start));
        assert(str_parse_int64(len_s, &len));

        Range range = { .start = start, .end = start + len };
        ranges_push(input_ranges, range);
    }

#if 0
    ranges_clear(output_ranges);
    for (size_t range_index = 0; range_index < input_ranges->count; range_index++) {
        Range range = input_ranges->ranges[range_index];
        map_lookup_range(a, range, output_ranges);
    }
#else
    for (int map_index = 0; map_index < NUM_MAPS; map_index++) {
        ranges_clear(output_ranges);
        for (size_t range_index = 0; range_index < input_ranges->count; range_index++) {
            Range range = input_ranges->ranges[range_index];
            map_lookup_range(&maps[map_index], range, output_ranges);
        }
        // outputs of previous map become input to next map (flip buffers)
        Ranges *tmp = input_ranges;
        input_ranges = output_ranges;
        output_ranges = tmp;
    }
#endif
    ASSERT(input_ranges->ranges != output_ranges->ranges);

    for (size_t i = 0; i < output_ranges->count; i++) {
        ASSERT(!range_empty(output_ranges->ranges[i]));
    }

    // iterate over output ranges and find minimum location
    int64_t part_2 = INT64_MAX;
    for (size_t i = 0; i < output_ranges->count; i++) {
        part_2 = MIN(part_2, output_ranges->ranges[i].start);
    }

    printf("%ld\n", part_2);
}
