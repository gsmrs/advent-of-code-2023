#include "advent.h"
#include <stdlib.h>
#include <ctype.h>

#include "trashhash.h"

typedef enum {
    X = 0,
    M = 1,
    A = 2,
    S = 3
} Property;

Property property_from_str(str s) {
    if (str_eq(s, cstr("x"))) return X;
    if (str_eq(s, cstr("m"))) return M;
    if (str_eq(s, cstr("a"))) return A;
    if (str_eq(s, cstr("s"))) return S;
    else ASSERT(0);
}

const char *property_to_str(Property p) {
    switch (p) {
        case X: return "x";
        case M: return "m";
        case A: return "a";
        case S: return "s";
        default: ASSERT(0);
    }
}

typedef struct Rule {
    struct Rule *next;
    enum {
        DEFAULT,
        LESS,
        GREATER
    } type;
    Property property;
    int value;
    str destination;
} Rule;

typedef struct {
    str name;
    Rule *rule;
} Workflow;

Rule *make_default_rule(str destination) {
    Rule *c = calloc(1, sizeof(*c));
    c->type = DEFAULT;
    c->destination = destination;
    return c;
}

Rule *make_rule_rule(str rule, str destination) {
    Rule *c = calloc(1, sizeof(*c));
    c->destination = destination;
    size_t op_index = 0;
    while (op_index < rule.len && isalpha(rule.data[op_index])) {
        op_index++;
    }
    ASSERT(op_index < rule.len);
    uint8_t op = rule.data[op_index];
    /* printf("got op: %c\n", op); */

    switch (op) {
        case '<': c->type = LESS; break;
        case '>': c->type = GREATER; break;
        default: ASSERT(0);
    }

    str property = str_sub(rule, 0, op_index);
    str value_str = str_sub(rule, op_index + 1, rule.len);
    int value;
    assert(str_parse_int(value_str, &value));

    c->property = property_from_str(property);
    c->value = value;
    return c;
}

Workflow parse_workflow(str line) {
    str name = str_next_token(&line, cstr("{"));
    /* printf("Got name = " STR_FMT "\n", STR(name)); */
    str content = str_next_token(&line, cstr("}"));
    content.len--; // WHY?
    /* printf("content = " STR_FMT "\n", STR(content)); */
    Workflow workflow = { .name = name };
    Rule *rule = NULL;
    Rule *current = NULL;

    while (!str_empty(content)) {
        str block = str_next_token(&content, cstr(","));
        /* printf(STR_FMT "\n", STR(block)); */
        str_find_result colon = str_find_left(block, cstr(":"));
        Rule *next = NULL;
        if (colon.found) {
            // proper rule
            /* printf("-> proper rule\n"); */
            str rule_str, dest;
            str_split(block, cstr(":"), &rule_str, &dest);
            next = make_rule_rule(rule_str, dest);
        } else {
            // default rule
            str dest = block;
            next = make_default_rule(dest);
            /* printf("-> default rule\n"); */
        }
        if (!rule && current) rule = current;
        if (current) {
            current->next = next;
        }
        current = next;
    }

    workflow.rule = rule;
    return workflow;
}

void print_workflow(Workflow *w) {
    printf(STR_FMT "{", STR(w->name));
    for (Rule *rule = w->rule; rule; rule = rule->next) {
        if (rule->type == DEFAULT) {
            printf(STR_FMT, STR(rule->destination));
        } else {
            printf("%s\n", property_to_str(rule->property));
            switch (rule->type) {
                case LESS: putchar('<'); break;
                case GREATER: putchar('>'); break;
                default: ASSERT(0);
            }
            printf("%d:" STR_FMT, rule->value, STR(rule->destination));
        }
        if (rule->next) {
            printf(",");
        }
    }
    printf("}\n");
}

uint64_t str_hash_func(void *key) {
    uint64_t hash = 5381;
    str s = *(str *) key;

    for (size_t i = 0; i < s.len; i++) {
        hash = ((hash << 5) + hash) + s.data[i];
    }

    return hash;
}

bool str_eq_func(void *key1, void *key2) {
    return str_eq(*(str *) key1, *(str *) key2);
}

void workflows_print(void *key, uintptr_t ptr, void *ctx) {
    (void) key;
    (void) ctx;
    Workflow *w = (Workflow *) ptr;
    print_workflow(w);
}

typedef union Part {
    struct { int x, m, a, s; };
    int values[4];
} Part;

Part parse_part(str line) {
    str content = line;
    content.data++;
    content.len -= 2;
    int index = 0;
    Part part;
    while (!str_empty(content)) {
        ASSERT(index < 4);
        str token = str_next_token(&content, cstr(","));
        str *var = malloc(sizeof(*var)); // TODO this leaks memory :(
        str value_s;
        str_split(token, cstr("="), var, &value_s);
        int value;
        ASSERT(str_parse_int(value_s, &value));
        part.values[index++] = value;
    }
    return part;
}

void part_printer(void *key, uintptr_t value, void *ctx) {
    (void) ctx;
    printf(STR_FMT "=%d,", STR(*(str *) key), (int) value);
}

void print_part(TrashMap *part) {
    printf("{");
    trashmap_iter(part, part_printer, NULL);
    printf("}\n");
}

typedef enum Status {
    REJECT = 'R',
    ACCEPT = 'A'
} Status;

Status route_part(TrashMap *workflows, Part part) {
    str workflow_name = cstr("in");
    for (;;) {
        /* printf("Current workflow: " STR_FMT "\n", STR(workflow_name)); */
        Workflow *workflow;
        ASSERT(trashmap_get(workflows, &workflow_name, (uintptr_t *) &workflow));
        ASSERT(workflow);

        for (Rule *rule = workflow->rule; rule; rule = rule->next) {
            if (rule->type == DEFAULT) {
                workflow_name = rule->destination;
                break;
            } else {
                int value = part.values[rule->property];
                bool matches = false;
                if (rule->type == LESS) {
                    matches = value < rule->value;
                } else if (rule->type == GREATER) {
                    matches = value > rule->value;
                } else {
                    ASSERT(0 && "not reachable");
                }
                if (matches) {
                    workflow_name = rule->destination;
                    break;
                }
            }
        }
        if (str_eq(workflow_name, cstr("R"))) {
            return REJECT;
        } else if (str_eq(workflow_name, cstr("A"))) {
            return ACCEPT;
        }
    }
}

int sum_values(Part part) {
    return part.x + part.m + part.a + part.s;
}

typedef struct {
    int lo, hi;
} Range;

typedef struct {
    Range x, m, a, s;
} PartRange;

typedef struct {
    str workflow_name;
    PartRange range;
} WorkItem;

typedef struct {
    WorkItem *items;
    int cap;
    int count;
} WorkStack;

void ws_push(WorkStack *ws, WorkItem item) {
    if (ws->count == ws->cap) {
        ws->cap = MAX(8, ws->cap * 2);
        ws->items = realloc(ws->items, ws->cap * sizeof(*ws->items));
        ASSERT(ws->items);
    }
    ws->items[ws->count++] = item;
}

WorkItem ws_pop(WorkStack *ws) {
    ASSERT(ws->count > 0);
    return ws->items[--ws->count];
}

int64_t compute_range_product(PartRange pr) {
    return (int64_t) (pr.x.hi - pr.x.lo + 1)
        * (int64_t) (pr.m.hi - pr.m.lo + 1)
        * (int64_t) (pr.a.hi - pr.a.lo + 1)
        * (int64_t) (pr.s.hi - pr.s.lo + 1);
}

void split_range_less(Range range, int threshold, Range *a, Range *b) {
    a->lo = MIN(range.lo, threshold - 1);
    a->hi = MIN(range.hi, threshold - 1);

    b->lo = MAX(range.lo, threshold);
    b->hi = MAX(range.hi, threshold);

    ASSERT(a->lo < threshold);
    ASSERT(a->hi < threshold);
    ASSERT(b->lo >= threshold);
    ASSERT(b->hi >= threshold);
}

void split_range_greater(Range range, int threshold, Range *a, Range *b) {
    a->lo = MAX(range.lo, threshold + 1);
    a->hi = MAX(range.hi, threshold + 1);

    b->lo = MIN(range.lo, threshold);
    b->hi = MIN(range.hi, threshold);

    ASSERT(a->lo > threshold);
    ASSERT(a->hi > threshold);
    ASSERT(b->lo <= threshold);
    ASSERT(b->hi <= threshold);
}

void subdivide_part_range(PartRange range, Rule *rule, PartRange *a, PartRange *b) {
    *a = range;
    *b = range;
    if (rule->type == LESS) {
        if (rule->property == X) {
            split_range_less(range.x, rule->value, &a->x, &b->x);
        } else if (rule->property == M) {
            split_range_less(range.m, rule->value, &a->m, &b->m);
        } else if (rule->property == A) {
            split_range_less(range.a, rule->value, &a->a, &b->a);
        } else if (rule->property == S) {
            split_range_less(range.s, rule->value, &a->s, &b->s);
        } else ASSERT(0);
    }
    else if (rule->type == GREATER) {
        if (rule->property == X) {
            split_range_greater(range.x, rule->value, &a->x, &b->x);
        } else if (rule->property == M) {
            split_range_greater(range.m, rule->value, &a->m, &b->m);
        } else if (rule->property == A) {
            split_range_greater(range.a, rule->value, &a->a, &b->a);
        } else if (rule->property == S) {
            split_range_greater(range.s, rule->value, &a->s, &b->s);
        } else ASSERT(0);
    } else {
        ASSERT(0);
    }
}

bool range_empty(Range r) {
    return r.hi < r.lo;
}

bool part_range_empty(PartRange pr) {
    return range_empty(pr.x)
        || range_empty(pr.m)
        || range_empty(pr.a)
        || range_empty(pr.s);
}

void print_part_range(PartRange pr) {
    printf("PR { .x = [%d, %d], .m = [%d, %d], .a = [%d, %d], .s = [%d, %d] }",
            pr.x.lo, pr.x.hi,
            pr.m.lo, pr.m.hi,
            pr.a.lo, pr.a.hi,
            pr.s.lo, pr.s.hi);
}

int64_t solve_part_2(TrashMap *workflows) {
    PartRange start_range = {
        .x = { 1, 4000 },
        .m = { 1, 4000 },
        .a = { 1, 4000 },
        .s = { 1, 4000 },
    };

    WorkStack stack = {0};

    WorkItem initial = { .workflow_name = cstr("in"), .range = start_range };
    ws_push(&stack, initial);

    int64_t part_2 = 0;
    while (stack.count > 0) {
        /* printf("stack.count = %d\n", stack.count); */
        WorkItem item = ws_pop(&stack);

        if (str_eq(item.workflow_name, cstr("A"))) {
            part_2 += compute_range_product(item.range);
            continue;
        } else if (str_eq(item.workflow_name, cstr("R"))) {
            continue;
        }

        Workflow *workflow;
        ASSERT(trashmap_get(workflows, &item.workflow_name, (uintptr_t *) &workflow));

        PartRange range = item.range;
        for (Rule *rule = workflow->rule; rule; rule = rule->next) {
            if (rule->type == DEFAULT) {
                WorkItem new_item = {
                    .workflow_name = rule->destination,
                    .range = range
                };
                ws_push(&stack, new_item);
            }
            else {
                PartRange matching, non_matching;
                subdivide_part_range(range, rule, &matching, &non_matching);
                if (!part_range_empty(matching)) {
                    WorkItem matching_item = {
                        .workflow_name = rule->destination,
                        .range = matching
                    };
                    ws_push(&stack, matching_item);
                }
                if (!part_range_empty(non_matching)) {
                    range = non_matching;
                } else {
                    // nothing left
                    break;
                }
            }
        }
    }
    return part_2;
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    TrashMap workflows;
    trashmap_init(&workflows, 1024, str_hash_func, str_eq_func);

    str lines = input;
    int workflow_count = 0, part_count = 0;
    for (;;) {
        str line = str_next_token(&lines, cstr("\n"));
        if (str_empty(line)) {
            // end of rules section
            break;
        }
        Workflow *workflow = calloc(1, sizeof(*workflow));
        *workflow = parse_workflow(line);
        trashmap_insert(&workflows, &workflow->name, (uintptr_t) workflow);
        workflow_count++;
        /* print_workflow(&*workflow); */
    }
    /* trashmap_iter(&workflows, workflows_print, NULL); */

    int part_1 = 0;
    while (!str_empty(lines)) {
        str line = str_next_token(&lines, cstr("\n"));
        line = str_trim(line);
        if (str_empty(line)) {
            continue;
        }
        printf("part: " STR_FMT "\n", STR(line));
        Part part = parse_part(line);

        Status status = route_part(&workflows, part);
        if (status == ACCEPT) {
            part_1 += sum_values(part);
        }
        part_count++;
    }
    /* printf("workflow_count = %d, part_count = %d\n", workflow_count, part_count); */
    /* printf("accepted = %d, rejected = %d\n", accepted, rejected); */
    printf("%d\n", part_1);

    printf("%ld\n", solve_part_2(&workflows));


    trashmap_free(&workflows);
    free(input.data);
}

#include "trashhash.c"
