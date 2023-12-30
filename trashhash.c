#include "trashhash.h"

#include <stdlib.h>
#include <string.h>

void trashmap_init(TrashMap *map, int bucket_count,
        uint64_t (*hash_func)(void *key),
        bool (*eq_func)(void *key1, void *key2)
        )
{
    memset(map, 0, sizeof(*map));
    map->bucket_count = bucket_count;
    map->buckets = calloc(bucket_count, sizeof(*map->buckets));
    map->hash_func = hash_func;
    map->eq_func = eq_func;
    ASSERT(((bucket_count & (bucket_count - 1)) == 0)); // power of two
}

bool trashmap_get(TrashMap *map, void *key, uintptr_t *result) {
    uint64_t hash = map->hash_func(key);
    uint64_t index = hash & (map->bucket_count - 1);

    for (TrashBucket *bucket = map->buckets[index];
            bucket;
            bucket = bucket->next)
    {
        if (map->eq_func(bucket->key, key)) {
            *result = (uintptr_t) bucket->value;
            return true;
        }
    }
    return false;
}

void trashmap_insert(TrashMap *map, void *key, uintptr_t value) {
    uint64_t hash = map->hash_func(key);
    uint64_t index = hash & (map->bucket_count - 1);

    for (TrashBucket *bucket = map->buckets[index];
            bucket;
            bucket = bucket->next)
    {
        if (map->eq_func(bucket->key, key)) {
            bucket->value = value;
            return;
        }
    }

    TrashBucket *bucket = calloc(1, sizeof(*bucket));
    bucket->next = map->buckets[index];
    bucket->key = key;
    bucket->value = value;
    map->buckets[index] = bucket;
}

void trashmap_delete(TrashMap *map, void *key) {
    uint64_t hash = map->hash_func(key);
    uint64_t index = hash & (map->bucket_count - 1);

    TrashBucket *prev = NULL;
    for (TrashBucket *bucket = map->buckets[index];
            bucket;
            prev = bucket, bucket = bucket->next)
    {
        if (map->eq_func(bucket->key, key)) {
            if (prev) {
                prev->next = bucket->next;
            } else {
                map->buckets[index] = bucket->next;
            }
            free(bucket);
            return;
        }
    }
}

void trashmap_iter(TrashMap *map,
        void (*callback)(void *key, uintptr_t value, void *context),
        void *context)
{
    for (int i = 0; i < map->bucket_count; i++) {
        for (TrashBucket *bucket = map->buckets[i];
                bucket;
                bucket = bucket->next)
        {
            callback(bucket->key, bucket->value, context);
        }
    }
}

void trashmap_clear(TrashMap *map) {
    for (int i = 0; i < map->bucket_count; i++) {
        for (TrashBucket *bucket = map->buckets[i]; bucket; ) {
            TrashBucket *to_delete = bucket;
            bucket = bucket->next;
            free(to_delete);
        }
        map->buckets[i] = NULL;
    }
}

void trashmap_free(TrashMap *map) {
    trashmap_clear(map);
}
