/* date = February 16th 2022 0:27 pm */

#ifndef DS_H
#define DS_H

#include "defines.h"
#include "os/os.h"
#include <string.h>

#define DoubleCapacity(x) ((x) <= 0 ? 8 : x * 2)
#define DoubleCapacityBigInit(x) ((x) <= 0 ? 64 : x * 2)

#define darray(type) type##_array

#define darray_add(type, array, data) type##_array##_add(array, data)
#define darray_add_at(type, array, data, idx) type##_array##_add_at(array, data, idx)
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
void Data##_array##_add_at(Data##_array* array, Data data, u32 idx);\
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
void Data##_array##_add_at(Data##_array* array, Data data, u32 idx) {\
if (array->len + 1 > array->cap) {\
void* prev = array->elems;\
u32 new_cap = DoubleCapacity(array->cap);\
array->elems = calloc(new_cap, sizeof(Data));\
memmove(array->elems, prev, array->len * sizeof(Data));\
array->cap = new_cap;\
free(prev);\
}\
memmove(array->elems + idx + 1, array->elems + idx, sizeof(Data) * (array->len - idx));\
array->elems[idx] = data;\
array->len++;\
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

#define dstack_push(type, stack, data) type##_stack##_push(stack, data)
#define dstack_pop(type, stack) type##_stack##_pop(stack)
#define dstack_peek(type, stack) type##_stack##_peek(stack)
#define dstack_clear(type, stack) type##_stack##_clear(stack)
#define dstack_free(type, stack) type##_stack##_free(stack)

#define Stack_Prototype(Data)\
typedef struct Data##_stack {\
u32 cap;\
u32 len;\
Data* elems;\
} Data##_stack;\
void Data##_stack##_push(Data##_stack* stack, Data data);\
Data Data##_stack##_pop(Data##_stack* stack);\
Data Data##_stack##_peek(Data##_stack* stack);\
void Data##_stack##_clear(Data##_stack* stack);\
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
void Data##_stack##_clear(Data##_stack* stack) {\
stack->len = 0;\
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
#define hash_table_get_ptr(key_t, value_t, table, key, val) key_t##_##value_t##_hash_table_get_ptr(table, key, val)
#define hash_table_get_ptr_guarantee(key_t, value_t, table, key, val)\
key_t##_##value_t##_hash_table_get_ptr_guarantee(table, key, val)
#define hash_table_del(key_t, value_t, table, key) key_t##_##value_t##_hash_table_del(table, key)
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
u64 cap;\
u64 len;\
Key##_##Value##_hash_table_entry* elems;\
} Key##_##Value##_hash_table;\
void Key##_##Value##_hash_table_init(Key##_##Value##_hash_table* table);\
void Key##_##Value##_hash_table_free(Key##_##Value##_hash_table* table);\
b8 Key##_##Value##_hash_table_get(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value* val);\
b8 Key##_##Value##_hash_table_get_ptr(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value** val);\
void Key##_##Value##_hash_table_get_ptr_guarantee(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value** val);\
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
static Key##_##Value##_hash_table_entry* Key##_##Value##_hash_table_find_entry(Key##_##Value##_hash_table_entry* entries, u64 cap,  Key##_##Value##_hash_table_key key) {\
u64 index = HashKey(key) % cap;\
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
static void Key##_##Value##_hash_table_adjust_cap(Key##_##Value##_hash_table* table, u64 cap) {\
Key##_##Value##_hash_table_entry* entries = calloc(cap, sizeof(Key##_##Value##_hash_table_entry));\
table->len = 0;\
for (u64 i = 0; i < table->cap; i++) {\
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
b8 Key##_##Value##_hash_table_get_ptr(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value** val) {\
if (table->len == 0) return false;\
Key##_##Value##_hash_table_entry* entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) return false;\
if (val != nullptr) *val = &entry->value;\
return true;\
}\
void Key##_##Value##_hash_table_get_ptr_guarantee(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key, Key##_##Value##_hash_table_value** val) {\
if (table->len == 0) {\
Key##_##Value##_hash_table_set(table, key, (Key##_##Value##_hash_table_value){0});\
}\
Key##_##Value##_hash_table_entry* entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) {\
Key##_##Value##_hash_table_set(table, key, (Key##_##Value##_hash_table_value){0});\
entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
}\
if (val != nullptr) *val = &entry->value;\
}\
b8 Key##_##Value##_hash_table_del(Key##_##Value##_hash_table* table, Key##_##Value##_hash_table_key key) {\
if (table->len == 0) return false;\
Key##_##Value##_hash_table_entry* entry = Key##_##Value##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) return false;\
entry->key = (Key##_##Value##_hash_table_key) {0};\
entry->value = Key##_##Value##_hash_table_tombstone;\
return true;\
}

// More specialized form of hash table which doesn't grow and provides stable pointers
// It allows semi-infinite allocations using a Memory Pool
// Due to this, we have a few requirements for the value type
// @requirement a key member
// @requirement hash_next and hash_prev pointer fields

#define stable_table_key(key, value) key##_##value##_stable_table_key
#define stable_table_value(key, value) key##_##value##_stable_table_value
#define stable_table(key, value) key##_##value##_stable_table

#define stable_table_init(key_t, value_t, table, num_slots) key_t##_##value_t##_stable_table_init(table, num_slots)
#define stable_table_set(key_t, value_t, table, key, val) key_t##_##value_t##_stable_table_set(table, key, val)
#define stable_table_get(key_t, value_t, table, key, val) key_t##_##value_t##_stable_table_get(table, key, val)
#define stable_table_get_guarantee(key_t, value_t, table, key, val)\
key_t##_##value_t##_stable_table_get_guarantee(table, key, val)
#define stable_table_del(key_t, value_t, table, key) key_t##_##value_t##_stable_table_del(table, key)
#define stable_table_free(key_t, value_t, table) key_t##_##value_t##_stable_table_free(table)

#define StableTable_Prototype(Key, Value)\
typedef Key Key##_##Value##_stable_table_key;\
typedef Value Key##_##Value##_stable_table_value;\
typedef struct Key##_##Value##_stable_table {\
M_Pool element_pool;\
u64 len;\
Key##_##Value##_stable_table_value* elems;\
} Key##_##Value##_stable_table;\
void Key##_##Value##_stable_table_init(Key##_##Value##_stable_table* table, u64 num_slots);\
void Key##_##Value##_stable_table_free(Key##_##Value##_stable_table* table);\
b8 Key##_##Value##_stable_table_get(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key, Key##_##Value##_stable_table_value** val);\
void Key##_##Value##_stable_table_get_guarantee(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key, Key##_##Value##_stable_table_value** val);\
void Key##_##Value##_stable_table_set(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key, Key##_##Value##_stable_table_value val);\
b8 Key##_##Value##_stable_table_del(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key);\

#define StableTable_Impl(Key, Value, KeyIsNull, KeyIsEqual, HashKey)\
void Key##_##Value##_stable_table_init(Key##_##Value##_stable_table* table, u64 num_slots) {\
pool_init(&table->element_pool, sizeof(Key##_##Value##_stable_table_value));\
table->len = num_slots;\
table->elems = OS_MemoryReserve(sizeof(Key##_##Value##_stable_table_value) * num_slots);\
OS_MemoryCommit(table->elems, sizeof(Key##_##Value##_stable_table_value) * num_slots);\
}\
void Key##_##Value##_stable_table_free(Key##_##Value##_stable_table* table) {\
OS_MemoryRelease(table->elems, sizeof(Key##_##Value##_stable_table_value) * table->len);\
pool_free(&table->element_pool);\
}\
b8 Key##_##Value##_stable_table_get(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key, Key##_##Value##_stable_table_value** val) {\
u64 slot = HashKey(key) % table->len;\
Key##_##Value##_stable_table_value* curr = &table->elems[slot];\
if (KeyIsNull(curr->key)) return false;\
while (curr) {\
if (KeyIsEqual(curr->key, key)) {\
*val = curr;\
return true;\
}\
curr = curr->hash_next;\
}\
return false;\
}\
void Key##_##Value##_stable_table_get_guarantee(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key, Key##_##Value##_stable_table_value** val) {\
u64 slot = HashKey(key) % table->len;\
Key##_##Value##_stable_table_value* curr = &table->elems[slot];\
Key##_##Value##_stable_table_value* one_before_curr = curr;\
if (!KeyIsNull(curr->key)) {\
while (curr) {\
if (!KeyIsNull(curr->key)) {\
if (KeyIsEqual(curr->key, key)) {\
*val = curr;\
return;\
}\
}\
one_before_curr = curr;\
curr = curr->hash_next;\
}\
curr = pool_alloc(&table->element_pool);\
curr->hash_prev = one_before_curr;\
one_before_curr->hash_next = curr;\
curr->key = key;\
*val = curr;\
} else {\
MemoryZeroStruct(curr, Key##_##Value##_stable_table_value);\
curr->key = key;\
*val = curr;\
}\
}\
void Key##_##Value##_stable_table_set(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key, Key##_##Value##_stable_table_value val) {\
u64 slot = HashKey(key) % table->len;\
Key##_##Value##_stable_table_value* curr = &table->elems[slot];\
Key##_##Value##_stable_table_value* one_before_curr = curr;\
if (!KeyIsNull(curr->key)) {\
while (curr) {\
if (!KeyIsNull(curr->key)) {\
if (KeyIsEqual(curr->key, key)) {\
*curr = val;\
return;\
}\
}\
one_before_curr = curr;\
curr = curr->hash_next;\
}\
curr = pool_alloc(&table->element_pool);\
curr->hash_prev = one_before_curr;\
one_before_curr->hash_next = curr;\
*curr = val;\
curr->key = key;\
} else {\
MemoryZeroStruct(curr, Key##_##Value##_stable_table_value);\
*curr = val;\
curr->key = key;\
}\
}\
b8 Key##_##Value##_stable_table_del(Key##_##Value##_stable_table* table, Key##_##Value##_stable_table_key key) {\
u64 slot = HashKey(key) % table->len;\
Key##_##Value##_stable_table_value* curr = &table->elems[slot];\
Key##_##Value##_stable_table_value* one_before_curr = curr;\
if (KeyIsNull(curr->key)) return false;\
while (curr) {\
if (!KeyIsNull(curr->key)) {\
if (KeyIsEqual(curr->key, key)) {\
if (curr->hash_prev) {\
curr->hash_prev->hash_next = curr->hash_next;\
if (curr->hash_next) curr->hash_next->hash_prev = curr->hash_prev;\
return true;\
} else {\
if (curr->hash_next) {\
if (curr->hash_next->hash_next)\
curr->hash_next->hash_next->hash_prev = curr;\
Key##_##Value##_stable_table_value v = *curr->hash_next;\
pool_dealloc(&table->element_pool, curr->hash_next);\
*curr = v;\
return true;\
} else {\
MemoryZeroStruct(curr, Key##_##Value##_stable_table_value);\
return true;\
}\
}\
}\
one_before_curr = curr;\
}\
curr = curr->hash_next;\
}\
return false;\
}

#endif //DS_H
