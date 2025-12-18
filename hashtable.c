#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define internal static
#define THRESHOLD(capacity) ((capacity*3)/4)

typedef struct {
    char* data;
    size_t count;
    uint64_t hash;
} string_obj;

typedef struct {
    string_obj key;
    uint32_t value;
    bool occupied;
} entry;

typedef struct {
    entry* items;
    size_t count;
    size_t capacity;
} hash_table;

uint64_t fnv1a(uint8_t* buffer, size_t buffer_size)
{
    uint64_t hash = 0xcbf29ce484222325;
    uint64_t FNV_prime =  0x100000001b3;
    for (size_t i=0; i<buffer_size; i++) {
        hash = hash ^ buffer[i];
        hash = hash * FNV_prime;
    }

    return hash;
}

internal
void resize_hash_table(hash_table* ht, size_t capacity)
{
    entry* new_items = (entry *)malloc(sizeof(*ht->items)*capacity);
    for (size_t i=0; i<ht->capacity; i++)
        new_items[i].occupied = false;

    for (size_t i=0; i<ht->capacity; i++) {
        if (!(ht->items[i].occupied))
            continue;
        size_t index = (new_items[i].key.hash) % (ht->capacity);
        new_items[index] = ht->items[i];
    }

    ht->capacity = capacity;
    free(ht->items);
    ht->items = new_items;
}

#define hash_init() (hash_table){0}

void hash_set(hash_table* ht, char* key, uint32_t value)
{
    if (ht->count >= THRESHOLD(ht->capacity)) {
        if (ht->capacity == 0)
            resize_hash_table(ht, 256);
        else
            resize_hash_table(ht, ht->capacity*2);
    }

    size_t key_len = 0;
    for (; key[key_len] != '\0'; key_len++);

    string_obj key_obj = (string_obj){
        .data = (char *)malloc(key_len * sizeof(char)),
        .count = key_len,
        .hash = fnv1a((uint8_t *)key, key_len)
    };

    size_t index = key_obj.hash % ht->capacity;
    while(ht->items[index].occupied)
        index = (index+1)%(ht->capacity);

    ht->items[index] = (entry){
        .key = key_obj,
        .value = value,
        .occupied = true
    };

    ht->count++;
}

bool hash_get(hash_table* ht, char* key, uint32_t* value)
{
    size_t key_len = 0;
    for (; key[key_len] != '\0'; key_len++);

    uint64_t hash = fnv1a((uint8_t *)key, key_len);
    size_t index = hash % ht->capacity;

    if(!ht->items[index].occupied)
        return false;

    while (ht->items[index].key.hash
           != hash)
        index = (index+1)%(ht->capacity);

    *value = ht->items[index].value;
    return true;
}

int main()
{
    return 0;
}
