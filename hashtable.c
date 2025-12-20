#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "hashtable.h"

#define internal static
#define THRESHOLD(capacity) ((capacity*3)/4)

internal
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
    entry* new_entries = (entry *)malloc(sizeof(*ht->items)*capacity);
    for (size_t i=0; i<capacity; i++)
        new_entries[i].status = ENTRY_EMPTY;

    for (size_t i=0; i<ht->capacity; i++) {
        if (ht->items[i].status != ENTRY_OCCUPIED)
            continue;
        size_t index = (ht->items[i].key.hash)%(capacity);
        while(new_entries[index].status == ENTRY_OCCUPIED)
            index = (index+1)%(capacity);
        new_entries[index] = ht->items[i];
    }

    free(ht->items);
    ht->items = new_entries;
    ht->capacity = capacity;
}

internal
entry* hash_find_entry(char* key, uint64_t hash,
                       entry* entries, size_t capacity)
{
    size_t index = hash % capacity;
    entry* last_deleted_entry = NULL;

    for (size_t i=0; i<capacity; i++) {
        entry* curr_entry = &entries[index];
        if (curr_entry->status == ENTRY_OCCUPIED
            && curr_entry->key.hash == hash) {
            size_t j = 0;
            for (;
                 key[j]
                     && key[j] == curr_entry->key.data[j];
                 j++);
            if (key[j] == curr_entry->key.data[j])
                return curr_entry;
        }
        else if (curr_entry->status == ENTRY_EMPTY) return curr_entry;
        else if (curr_entry->status == ENTRY_DELETED) last_deleted_entry = curr_entry;
        index = (index+1)%capacity;
    }
    return last_deleted_entry;
}

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

    uint64_t hash  = fnv1a((uint8_t *)key, key_len);
    entry* ht_entry = hash_find_entry(key, hash, ht->items, ht->capacity);

    assert(ht_entry != NULL && "hash_find_entry must not return null");

    if (ht_entry->status == ENTRY_OCCUPIED) {
        ht_entry->value = value;
    }
    else {
        char* data = (char *)malloc(sizeof(char)*(key_len+1));
        for (size_t i=0; i<key_len; i++)
            data[i] = key[i];
        data[key_len] = '\0';

        ht_entry->key = (string_obj){
            .data = data,
            .hash = hash
        };
        ht_entry->value = value;

        if (ht_entry->status == ENTRY_EMPTY)
            ht->count++;
        ht_entry->status = ENTRY_OCCUPIED;
    }
}

bool hash_get(hash_table* ht, char* key, uint32_t* value)
{
    size_t key_len = 0;
    for (; key[key_len] != '\0'; key_len++);
    uint64_t hash = fnv1a((uint8_t *)key, key_len);
    entry* ht_entry = hash_find_entry(key, hash, ht->items, ht->capacity);

    assert(ht_entry != NULL && "hash_find_entry must not return null");

    if (ht_entry->status != ENTRY_OCCUPIED)
        return false;
    *value = ht_entry->value;
    return true;
}

bool hash_delete(hash_table* ht, char* key)
{
    size_t key_len = 0;
    for (; key[key_len] != '\0'; key_len++);
    uint64_t hash = fnv1a((uint8_t *)key, key_len);
    entry* ht_entry = hash_find_entry(key, hash, ht->items, ht->capacity);

    assert(ht_entry != NULL && "hash_find_entry must not return null");

    if (ht_entry->status != ENTRY_OCCUPIED)
        return false;
    ht_entry->status = ENTRY_DELETED;
    return true;
}

void hash_free(hash_table* ht)
{
    for (int i=0; i<ht->capacity; i++) {
        if (ht->items[i].status == ENTRY_EMPTY)
            continue;
        free(ht->items[i].key.data);
    }
    free(ht->items);
    ht->items = NULL;
    ht->count = 0;
    ht->capacity = 0;
}
