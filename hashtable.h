#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>

typedef struct {
    char* data;
    uint64_t hash;
} string_obj;

typedef enum {
    ENTRY_OCCUPIED,
    ENTRY_EMPTY,
    ENTRY_DELETED
} entry_status;

typedef struct {
    string_obj key;
    uint32_t value;
    entry_status status;
} entry;

typedef struct {
    entry* items;
    size_t count;
    size_t capacity;
} hash_table;

#define hash_init(ht)                           \
    do {                                        \
        (ht)->items = NULL;                     \
        (ht)->count = 0;                        \
        (ht)->capacity = 0;                     \
    } while(0)
void hash_set(hash_table* ht, char* key, uint32_t value);
bool hash_get(hash_table* ht, char* key, uint32_t* value);
bool hash_delete(hash_table* ht, char* key);
void hash_free(hash_table* ht);

#endif
