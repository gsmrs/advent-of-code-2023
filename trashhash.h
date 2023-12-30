#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct TrashBucket {
    void *key;
    uintptr_t value;
    struct TrashBucket *next;
} TrashBucket;

typedef struct {
    TrashBucket **buckets;
    int bucket_count;

    uint64_t (*hash_func)(void *key);
    bool (*eq_func)(void *key1, void *key2);
} TrashMap;

void trashmap_init(TrashMap *map, int bucket_count,
        uint64_t (*hash_func)(void *key),
        bool (*eq_func)(void *key1, void *key2)
        );

bool trashmap_get(TrashMap *map, void *key, uintptr_t *result);
void trashmap_insert(TrashMap *map, void *key, uintptr_t value);
void trashmap_delete(TrashMap *map, void *key);
void trashmap_iter(TrashMap *map, void (*callback)(void *key, uintptr_t value, void *context), void *context);
void trashmap_clear(TrashMap *map);
void trashmap_free(TrashMap *map);
