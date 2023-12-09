#include "advent.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIRS,
    THREE_OF_A_KIND,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    FIVE_OF_A_KIND,
} Type;

typedef struct {
    char hand[5];
    int bid;
    Type type;
} Hand;

const char *get_type_str(Type type) {
    switch (type) {
        case FIVE_OF_A_KIND: return "FIVE_OF_A_KIND";
        case FOUR_OF_A_KIND: return "FOUR_OF_A_KIND";
        case FULL_HOUSE: return "FULL_HOUSE";
        case THREE_OF_A_KIND: return "THREE_OF_A_KIND";
        case TWO_PAIRS: return "TWO_PAIRS";
        case ONE_PAIR: return "ONE_PAIR";
        case HIGH_CARD: return "HIGH_CARD";
        default: return "UNKNOWN";
    }
}

int compare_cards_char(const void *p1, const void *p2) {
    char c1 = *(const char *)p1;
    char c2 = *(const char *)p2;
    if (c1 < c2) return -1;
    if (c1 > c2) return 1;
    return 0;
}

Hand sort_hand(Hand hand) {
    qsort(hand.hand, 5, sizeof(char), compare_cards_char);
    return hand;
}

int n_of_a_kind(Hand sorted_hand) {
    int max_count = 1;
    int count = 1;
    for (int i = 1; i < 5; i++) {
        if (sorted_hand.hand[i] == sorted_hand.hand[i - 1]) {
            count++;
        }
         else {
             if (count > max_count) {
                 max_count = count;
                 count = 1;
             }
         }
    }
    if (count > max_count) {
        max_count = count;
    }
    return max_count;
}

bool is_full_house(Hand sh) {
    return ((sh.hand[0] == sh.hand[1]) && (sh.hand[2] == sh.hand[3]) && (sh.hand[3] == sh.hand[4]))
        | ((sh.hand[0] == sh.hand[1]) && (sh.hand[1] == sh.hand[2]) && (sh.hand[3] == sh.hand[4]));
}

int count_pairs(Hand sh) {
    int pairs = 0;
    for (int i = 1; i < 5; i++) {
        if (sh.hand[i] == sh.hand[i - 1]) {
            pairs++;
        }
    }
    return pairs;
}

Type get_type(Hand hand) {
    if ((hand.hand[0] == hand.hand[1])
        && (hand.hand[0] == hand.hand[2])
        && (hand.hand[0] == hand.hand[3])
        && (hand.hand[0] == hand.hand[4]))
    {
        return FIVE_OF_A_KIND;
    }

    Hand sorted_hand = sort_hand(hand);

    int n = n_of_a_kind(sorted_hand);
    if (n == 4) {
        return FOUR_OF_A_KIND;
    }

    if (is_full_house(sorted_hand)) {
        return FULL_HOUSE;
    }

    if (n == 3) {
        return THREE_OF_A_KIND;
    }

    int n_pairs = count_pairs(sorted_hand);
    if (n_pairs == 2) {
        return TWO_PAIRS;
    }
    else if (n_pairs == 1) {
        return ONE_PAIR;
    }

    return HIGH_CARD;
}

static const char CARD_SYMBOLS[] = {
    'J',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'T',
    'Q',
    'K',
    'A',
};

#define NUM_CARD_SYMBOLS sizeof(CARD_SYMBOLS)

Type get_type_with_jokers_helper(Hand hand, int index) {
    if (index == 5) {
        return get_type(hand);
    }

    if (hand.hand[index] == 'J') {
        Type best_type = HIGH_CARD;
        for (size_t i = 0; i < NUM_CARD_SYMBOLS; i++) {
            hand.hand[index] = CARD_SYMBOLS[i];
            Type t = get_type_with_jokers_helper(hand, index + 1);
            if (t > best_type) {
                best_type = t;
            }
        }
        hand.hand[index] = 'J';
        return best_type;
    } else {
        return get_type_with_jokers_helper(hand, index + 1);
    }
}

Type get_type_with_jokers(Hand hand) {
    return get_type_with_jokers_helper(hand, 0);
}

int get_card_value(char card) {
    switch (card) {
        case 'A': return 14;
        case 'K': return 13;
        case 'Q': return 12;
        case 'J': return 11;
        case 'T': return 10;
        default: assert(isdigit(card)); return (card - '0');
    }
}

int get_card_value_with_jokers(char card) {
    switch (card) {
        case 'A': return 14;
        case 'K': return 13;
        case 'Q': return 12;
        case 'J': return 1;
        case 'T': return 10;
        default: assert(isdigit(card)); return (card - '0');
    }
}

int compare_cards(char a, char b) {
    int a_value = get_card_value(a);
    int b_value = get_card_value(b);
    if (a_value < b_value) return -1;
    if (a_value > b_value) return 1;
    return 0;
}

int compare_cards_with_jokers(char a, char b) {
    int a_value = get_card_value_with_jokers(a);
    int b_value = get_card_value_with_jokers(b);
    if (a_value < b_value) return -1;
    if (a_value > b_value) return 1;
    return 0;
}

int compare_hands(const void *p1, const void *p2, int (*compare_func)(char a, char b)) {
    const Hand *h1 = p1;
    const Hand *h2 = p2;
    if (h1->type < h2->type) return -1;
    if (h1->type > h2->type) return 1;
    // equal type; determine order by stronger card in order
    for (int i = 0; i < 5; i++) {
        /* int c = compare_cards(h1->hand[i], h2->hand[i]); */
        int c = compare_func(h1->hand[i], h2->hand[i]);
        if (c != 0) {
            return c;
        }
    }
    return 0;
}

int compare_hands_without_jokers(const void *p1, const void *p2) {
    return compare_hands(p1, p2, compare_cards);
}

int compare_hands_with_jokers(const void *p1, const void *p2) {
    return compare_hands(p1, p2, compare_cards_with_jokers);
}

int main(int argc, const char **argv) {
    const char *input_file = get_input(argc, argv);
    str input = read_file(input_file);

    enum { NUM_MAX_HANDS = 1024 };
    Hand hands[NUM_MAX_HANDS];
    int hand_count = 0;

    while (!str_empty(input)) {
        assert(hand_count < NUM_MAX_HANDS);
        str line = str_next_token(&input, cstr("\n"));

        str hand_str, bid_str;
        str_split(line, cstr(" "), &hand_str, &bid_str);

        Hand *hand = &hands[hand_count];
        assert(str_parse_int(bid_str, &hand->bid));
        memcpy(hand->hand, hand_str.data, 5);

        hand->type = get_type(*hand);
#if 0
        printf("Hand: %.*s (%d) -> %s\n", 5, hand->hand, hand->bid, get_type_str(hand->type));
#endif

        hand_count++;
    }

    qsort(hands, hand_count, sizeof(hands[0]), compare_hands_without_jokers);
    int64_t part_1 = 0;
    for (int i = 0; i < hand_count; i++) {
        int64_t rank = i + 1;
        part_1 += rank * (int64_t) hands[i].bid;
    }
    printf("%ld\n", part_1);

    // Part 2
    for (int i = 0; i < hand_count; i++) {
        hands[i].type = get_type_with_jokers(hands[i]);
    }
    qsort(hands, hand_count, sizeof(hands[0]), compare_hands_with_jokers);
    int64_t part_2 = 0;
    for (int i = 0; i < hand_count; i++) {
        int64_t rank = i + 1;
        part_2 += rank * (int64_t) hands[i].bid;
    }
    printf("%ld\n", part_2);
}
