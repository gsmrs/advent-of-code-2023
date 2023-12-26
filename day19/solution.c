#include "advent.h"
#include <stdlib.h>
#include <ctype.h>

#include "trashhash.h"

typedef struct Rule {
    struct Rule *next;
    enum {
        DEFAULT,
        LESS,
        GREATER
    } type;
    str property;
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

    c->property = property;
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
            printf(STR_FMT, STR(rule->property));
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

void parse_part(str line, TrashMap *part) {
    str content = line;
    content.data++;
    content.len -= 2;
    while (!str_empty(content)) {
        str token = str_next_token(&content, cstr(","));
        str *var = malloc(sizeof(*var)); // TODO this leaks memory :(
        str value_s;
        str_split(token, cstr("="), var, &value_s);
        int value;
        ASSERT(str_parse_int(value_s, &value));
        trashmap_insert(part, var, (uintptr_t) value);
    }
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

void part_add_visitor(void *key, uintptr_t value, void *total_ptr) {
    (void) key;
    int *total = total_ptr;
    *total += (int) value;
}

typedef enum Status {
    REJECT = 'R',
    ACCEPT = 'A'
} Status;

Status route_part(TrashMap *workflows, TrashMap *part) {
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
                uintptr_t value;
                ASSERT(trashmap_get(part, &rule->property, &value));
                bool matches = false;
                if (rule->type == LESS) {
                    matches = (int) value < rule->value;
                } else if (rule->type == GREATER) {
                    matches = (int) value > rule->value;
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

int sum_values(TrashMap *part) {
    int total = 0;
    trashmap_iter(part, part_add_visitor, &total);
    return total;
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
    trashmap_iter(&workflows, workflows_print, NULL);

    TrashMap part;
    trashmap_init(&part, 16, str_hash_func, str_eq_func);
    int accepted = 0;
    int rejected = 0;
    int part_1 = 0;
    while (!str_empty(lines)) {
        trashmap_clear(&part);
        str line = str_next_token(&lines, cstr("\n"));
        line = str_trim(line);
        if (str_empty(line)) {
            continue;
        }
        printf("part: " STR_FMT "\n", STR(line));
        parse_part(line, &part);

        Status status = route_part(&workflows, &part);
        if (status == ACCEPT) {
            part_1 += sum_values(&part);
        }
        part_count++;
    }
    /* printf("workflow_count = %d, part_count = %d\n", workflow_count, part_count); */
    /* printf("accepted = %d, rejected = %d\n", accepted, rejected); */
    printf("%d\n", part_1);


    trashmap_free(&workflows);
    trashmap_free(&part);
    free(input.data);
}

#include "trashhash.c"
