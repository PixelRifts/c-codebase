/* date = February 16th 2022 0:27 pm */

#ifndef DS_H
#define DS_H

#define DoubleCapacity(x) ((x) <= 0 ? 8 : x * 2)

#define Iterate(array, var) for (int var = 0; var < array.len; var++)
#define IteratePtr(array, var) for (int var = 0; var < array->len; var++)

#define Array_Prototype(Name, Data)\
typedef struct Name {\
u32 cap;\
u32 len;\
Data* elems;\
} Name;\
void Name##_add(Name* array, Data data);\
Data Name##_remove(Name* array, int idx);\
void Name##_free(Name* array);

#define Array_Impl(Name, Data)\
void Name##_add(Name* array, Data data) {\
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
Data Name##_remove(Name* array, int idx) {\
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
void Name##_free(Name* array) {\
array->cap = 0;\
array->len = 0;\
free(array->elems);\
}

#define Stack_Prototype(Name, Data)\
typedef struct Name {\
u32 cap;\
u32 len;\
Data* elems;\
} Name;\
void Name##_push(Name* stack, Data data);\
Data Name##_pop(Name* stack);\
Data Name##_peek(Name* stack);\
void Name##_free(Name* stack);

#define Stack_Impl(Name, Data)\
void Name##_push(Name* stack, Data data) {\
if (stack->len + 1 > stack->cap) {\
void* prev = stack->elems;\
u32 new_cap = DoubleCapacity(stack->cap);\
stack->elems = calloc(new_cap, sizeof(Data));\
memmove(stack->elems, prev, stack->len * sizeof(Data));\
free(prev);\
}\
stack->elems[stack->len++] = data;\
}\
Data Name##_pop(Name* stack) {\
if (stack->len == 0) return (Data){0};\
return stack->elems[--stack->len];\
}\
Data Name##_peek(Name* stack) {\
if (stack->len == 0) return (Data){0};\
return stack->elems[stack->len - 1];\
}\
void Name##_free(Name* stack) {\
stack->cap = 0;\
stack->len = 0;\
free(stack->elems);\
}

#define HashTable_MaxLoad 0.75

/* Name of table will be */
#define HashTable_Prototype(Name, Key, Value)\
typedef Key Name##_hash_table_key;\
typedef Value Name##_hash_table_value;\
typedef struct Name##_hash_table_entry {\
Name##_hash_table_key key;\
Name##_hash_table_value value;\
} Name##_hash_table_entry;\
typedef struct Name##_hash_table {\
u32 cap;\
u32 len;\
Name##_hash_table_entry* elems;\
} Name##_hash_table;\
void Name##_hash_table_init(Name##_hash_table* table);\
void Name##_hash_table_free(Name##_hash_table* table);\
b8 Name##_hash_table_get(Name##_hash_table* table, Name##_hash_table_key key, Name##_hash_table_value* val);\
b8 Name##_hash_table_set(Name##_hash_table* table, Name##_hash_table_key key, Name##_hash_table_value  val);\
b8 Name##_hash_table_del(Name##_hash_table* table, Name##_hash_table_key key);\
void Name##_hash_table_add_all(Name##_hash_table* to, Name##_hash_table* from);

#define HashTable_Impl(Name, KeyIsNull, KeyIsEqual, HashKey, Tombstone, ValIsNull, ValIsTombstone)\
static const Name##_hash_table_value Name##_hash_table_tombstone = Tombstone;\
void Name##_hash_table_init(Name##_hash_table* table) {\
table->cap = 0;\
table->len = 0;\
table->elems = nullptr;\
}\
void Name##_hash_table_free(Name##_hash_table* table) {\
free(table->elems);\
table->cap = 0;\
table->len = 0;\
table->elems = nullptr;\
}\
static Name##_hash_table_entry* Name##_hash_table_find_entry(Name##_hash_table_entry* entries, u32 cap,  Name##_hash_table_key key) {\
u32 index = HashKey(key) % cap;\
Name##_hash_table_entry* tombstone = nullptr;\
while (true) {\
Name##_hash_table_entry* entry = &entries[index];\
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
static void Name##_hash_table_adjust_cap(Name##_hash_table* table, u32 cap) {\
Name##_hash_table_entry* entries = calloc(cap, sizeof(Name##_hash_table_entry));\
table->len = 0;\
for (u32 i = 0; i < table->cap; i++) {\
Name##_hash_table_entry* curr = &table->elems[i];\
if (KeyIsNull(curr->key)) continue;\
Name##_hash_table_entry* dest = Name##_hash_table_find_entry(entries, cap, curr->key);\
dest->key = curr->key;\
dest->value = curr->value;\
table->len++;\
}\
free(table->elems);\
table->cap = cap;\
table->elems = entries;\
}\
b8 Name##_hash_table_set(Name##_hash_table* table, Name##_hash_table_key key, Name##_hash_table_value  val) {\
if (table->len + 1 > table->cap * HashTable_MaxLoad) {\
u32 cap = DoubleCapacity(table->cap);\
Name##_hash_table_adjust_cap(table, cap);\
}\
Name##_hash_table_entry* entry = Name##_hash_table_find_entry(table->elems, table->cap, key);\
b8 is_new_key = KeyIsNull(entry->key);\
if (is_new_key && ValIsNull(entry->value)) table->len++;\
entry->key = key;\
entry->value = val;\
return is_new_key;\
}\
void Name##_hash_table_add_all(Name##_hash_table* to, Name##_hash_table* from) {\
for (u32 i = 0; i < from->cap; i++) {\
Name##_hash_table_entry* e = &from->elems[i];\
if (KeyIsNull(e->key)) continue;\
Name##_hash_table_set(to, e->key, e->value);\
}\
}\
b8 Name##_hash_table_get(Name##_hash_table* table, Name##_hash_table_key key, Name##_hash_table_value* val) {\
if (table->len == 0) return false;\
Name##_hash_table_entry* entry = Name##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) return false;\
if (val != nullptr) *val = entry->value;\
return true;\
}\
b8 Name##_hash_table_del(Name##_hash_table* table, Name##_hash_table_key key) {\
if (table->len == 0) return false;\
Name##_hash_table_entry* entry = Name##_hash_table_find_entry(table->elems, table->cap, key);\
if (KeyIsNull(entry->key)) return false;\
entry->key = (Name##_hash_table_key) {0};\
entry->value = Name##_hash_table_tombstone;\
return true;\
}\

#endif //DS_H
