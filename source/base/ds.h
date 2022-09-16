/* date = February 16th 2022 0:27 pm */

#ifndef DS_H
#define DS_H

#include "defines.h"
#include <string.h>

#define DoubleCapacity(x) ((x) <= 0 ? 8 : x * 2)

#define darray(type) type##_array

#define darray_add(type, array, data) type##_array##_add(array, data)
#define darray_reserve(type, array, count) type##_array##_reserve(array, count)
#define darray_remove(type, array, idx) type##_array##_remove(array, idx)
#define darray_free(type, array) type##_array##_free(array)

#define DArray_Prototype(Data)\
typedef struct Data##_array {\
u32 cap;\
u32 len;\
Data* elems;\
} Data##_array;\
void Data##_array##_add(Data##_array* array, Data data);\
void Data##_array##_reserve(Data##_array* array, u32 count);\
Data Data##_array##_remove(Data##_array* array, int idx);\
void Data##_array##_free(Data##_array* array);

#define DArray_Impl(Data)\
void Data##_array##_add(Data##_array* array, Data data) {\
if (array->len + 1 > array->cap) {\
void* prev = array->elems;\
u32 new_cap = DoubleCapacity(array->cap);\
array->elems = calloc(new_cap, sizeof(Data));\
memmove(array->elems, prev, array->len * sizeof(Data));\
array->cap = new_cap;\
free(prev);\
}\
array->elems[array->len++] = data;\
}\
void Data##_array##_reserve(Data##_array* array, u32 count) {\
void* prev = array->elems;\
u32 new_cap = count;\
array->elems = calloc(new_cap, sizeof(Data));\
memmove(array->elems, prev, array->len * sizeof(Data));\
array->cap = new_cap;\
if (prev) free(prev);\
}\
Data Data##_array##_remove(Data##_array* array, int idx) {\
if (idx >= array->len || idx < 0) return (Data){0};\
Data value = array->elems[idx];\
if (idx == array->len - 1) {\
array->len--;\
return value;\
}\
Data* from = array->elems + idx + 1;\
Data* to = array->elems + idx;\
memmove(to, from, sizeof(Data) * (array->len - idx - 1));\
array->len--;\
return value;\
}\
void Data##_array##_free(Data##_array* array) {\
array->cap = 0;\
array->len = 0;\
free(array->elems);\
}

#define dstack(type) type##_stack

#define dstack_push(type, stack, data) Data##_stack##_push(stack, data)
#define dstack_pop(type, stack, data) Data##_stack##_pop(stack)
#define dstack_peek(type, stack, data) Data##_stack##_peek(stack)
#define dstack_free(type, stack, data) Data##_stack##_free(stack)

#define Stack_Prototype(Data)\
typedef struct Data##_stack {\
u32 cap;\
u32 len;\
Data* elems;\
} Data##_stack;\
void Data##_stack##_push(Data##_stack* stack, Data data);\
Data Data##_stack##_pop(Data##_stack* stack);\
Data Data##_stack##_peek(Data##_stack* stack);\
void Data##_stack##_free(Data##_stack* stack);

#define Stack_Impl(Data)\
void Data##_stack##_push(Data##_stack* stack, Data data) {\
if (stack->len + 1 > stack->cap) {\
void* prev = stack->elems;\
u32 new_cap = DoubleCapacity(stack->cap);\
stack->elems = calloc(new_cap, sizeof(Data));\
memmove(stack->elems, prev, stack->len * sizeof(Data));\
free(prev);\
}\
stack->elems[stack->len++] = data;\
}\
Data Data##_stack##_pop(Data##_stack* stack) {\
if (stack->len == 0) return (Data){0};\
return stack->elems[--stack->len];\
}\
Data Data##_stack##_peek(Data##_stack* stack) {\
if (stack->len == 0) return (Data){0};\
return stack->elems[stack->len - 1];\
}\
void Data##_stack##_free(Data##_stack* stack) {\
stack->cap = 0;\
stack->len = 0;\
free(stack->elems);\
}

#define HashTable_MaxLoad 0.75

#define hash_table_key(key, value) key##_##value##_hash_table_key
#define hash_table_value(key, value) key##_##value##_hash_table_value
#define hash_table_entry(key, value) key##_##value##_hash_table_entry
#define hash_table(key, value) key##_##value##_hash_table

#define hash_table_init(key_t, value_t, table) key_t##_##value_t##_hash_table_init(table)
#define hash_table_set(key_t, value_t, table, key, val) key_t##_##value_t##_hash_table_set(table, key, val)
#define hash_table_get(key_t, value_t, table, key, val) key_t##_##value_t##_hash_table_get(table, key, val)
#define hash_table_del(key_t, value_t, table, key, val) key_t##_##value_t##_hash_table_del(table, key)
#define hash_table_add_all(key_t, value_t, to, from) key_t##_##value_t##_hash_table_del(to, from)
#define hash_table_free(key_t, value_t, table) key_t##_##value_t##_hash_table_free(table)

#define HashTable_Prototype(Key, Value)\
typedef Key Key##_##Value##_hash_table_key;\
typedef Value Key##_##Value##_hash_table_value;\
typedef struct Key##_##Value##_hash_table_entry {\
Key##_##Value##_hash_table_key key;\
Key##_##Value##_hash_table_value value;\
} Key##_##Value##_hash_table_entry;\
typedef struct Key##_##Value##_hash_table {\
u32 cap;\
u32 len;\
Key##_##Value##_hash_table_entry* elems;\
} Key##_##Value##_hash_table;\
void Key##_##Value##_hash_table_init(Key##_##Value##_hash_table* table);\
void Key##_##Value##_hash_table_free(Key##_##Value##_hash_table* table);\
b8 Key##_##Value##_hash_table_get(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value* val);\
b8 Key##_##Value##_hash_table_set(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value  val);\
b8 Key##_##Value##_hash_table_del(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key);\
void Key##_##Value##_hash_table_add_all(Key##_##Value##_hash_table* to, Key##_##Value##_hash_table* from);

#define HashTable_Impl(Key, Value, KeyIsNull, KeyIsEqual, HashKey, Tombstone, ValIsNull, ValIsTombstone)\
static const Key##_##Value##_hash_table_value Key##_##Value##_hash_table_tombstone = Tombstone;\
void Key##_##Value##_hash_table_init(Key##_##Value##_hash_table* table) {\
table->cap = 0;\
table->len = 0;\
table->elems = nullptr;\
}\
void Key##_##Value##_hash_table_free(Key##_##Value##_hash_table* table) {\
free(table->elems);\
table->cap = 0;\
table->len = 0;\
table->elems = nullptr;\
}\
static Key##_##Value##_hash_table_entry* Key##_##Value##_hash_table_find_entry(Key##_##Value##_hash_table_entry* entries, u32 cap,  Key##_##Value##_hash_table_key key) {\
u32 index = HashKey(key) % cap;\
Key##_##Value##_hash_table_entry* tombstone = nullptr;\
while (true) {\
Key##_##Value##_hash_table_entry* entry = &entries[index];\
if (KeyIsNull(entry->key)) {\
if (ValIsNull(entry->value))\
return tombstone != nullptr ? tombstone : entry;\
else {\
if (tombstone == nullptr) tombstone = entry;\
}\
} else if (KeyIsEqual(entry->key, key))\
return entry;\
index = (index + 1) % cap;\
}\
}\
static void Key##_##Value##_hash_table_adjust_cap(Key##_##Value##_hash_table* table, u32 cap) {\
Key##_##Value##_hash_table_entry* entries = calloc(cap, sizeof(Key##_##Value##_hash_table_entry));\
table->len = 0;\
for (u32 i = 0; i < table->cap; i++) {\
Key##_##Value##_hash_table_entry* curr = &table->elems[i];\
if (KeyIsNull(curr->key)) continue;\
Key##_##Value##_hash_table_entry* dest = Key##_##Value##_hash_table_find_entry(entries, cap, curr->key);\
dest->key = curr->key;\
dest->value = curr->value;\
table->len++;\
}\
free(table->elems);\
table->cap = cap;\
table->elems = entries;\
}\
b8 Key##_##Value##_hash_table_set(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value  val) {\
if (table->len + 1 > table->cap * HashTable_MaxLoad) {\
u32 cap = DoubleCapacity(table->cap);\
Key##_##Value##_hash_table_adjust_cap(table, cap);\
}\
Key##_##Value##_hash_table_entry* entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
b8 is_new_key = KeyIsNull(entry->key);\
if (is_new_key && ValIsNull(entry->value)) table->len++;\
entry->key = key;\
entry->value = val;\
return is_new_key;\
}\
void Key##_##Value##_hash_table_add_all(Key##_##Value##_hash_table* to, Key##_##Value##_hash_table* from) {\
for (u32 i = 0; i < from->cap; i++) {\
Key##_##Value##_hash_table_entry* e = &from->elems[i];\
if (KeyIsNull(e->key)) continue;\
Key##_##Value##_hash_table_set(to, e->key, e->value);\
}\
}\
b8 Key##_##Value##_hash_table_get(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value* val) {\
if (table->len == 0) return false;\
Key##_##Value##_hash_table_entry* entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) return false;\
if (val != nullptr) *val = entry->value;\
return true;\
}\
b8 Key##_##Value##_hash_table_del(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key) {\
if (table->len == 0) return false;\
Key##_##Value##_hash_table_entry* entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) return false;\
entry->key = (Key##_##Value##_hash_table_key) {0};\
entry->value = Key##_##Value##_hash_table_tombstone;\
return true;\
}\

#endif //DS_H
