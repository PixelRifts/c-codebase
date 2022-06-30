#include "str.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

string_const str_alloc(M_Arena* arena, u64 size) {
    string_const str = {0};
    str.str = (u8*)arena_alloc(arena, size + 1);
    str.str[size] = '\0';
    str.size = size;
    return str;
}

string_const str_copy(M_Arena* arena, string_const other) {
    string_const str = {0};
    str.str = (u8*)arena_alloc(arena, other.size + 1);
    str.size = other.size;
    memcpy(str.str, other.str, other.size);
    str.str[str.size] = '\0';
    return str;
}

string_const str_cat(M_Arena* arena, string_const a, string_const b) {
    string_const final = {0};
    final.size = a.size + b.size;
    final.str = (u8*)arena_alloc(arena, final.size + 1);
    memcpy(final.str, a.str, a.size);
    memcpy(final.str + a.size, b.str, b.size);
    final.str[final.size] = '\0';
    return final;
}

string_const str_from_format(M_Arena* arena, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const char buf[8092];
    vsnprintf(buf, 8092, format, args);
    va_end(args);
    u64 size = strlen(buf);
    string_const s = str_alloc(arena, size);
    memmove(s.str, buf, size);
    return s;
}

b8 str_eq(string_const a, string_const b) {
    if (a.size != b.size) return false;
    return memcmp(a.str, b.str, b.size) == 0;
}

string_const str_replace_all(M_Arena* arena, string_const to_fix, string_const needle, string_const replacement) {
    if (needle.size == 0) return to_fix;
    u64 replaceable = str_substr_count(to_fix, needle);
    if (replaceable == 0) return to_fix;
    
    u64 new_size = (to_fix.size - replaceable * needle.size) + (replaceable * replacement.size);
    string_const ret = str_alloc(arena, new_size);
    
    b8 replaced;
    u64 o = 0;
    for (u64 i = 0; i < to_fix.size;) {
        replaced = false;
        if (to_fix.str[i] == needle.str[0]) {
            if ((to_fix.size - i) >= needle.size) {
                string_const sub = { .str = to_fix.str + i, .size = needle.size };
                if (str_eq(sub, needle)) {
                    // replace this one
                    memmove(ret.str + o, replacement.str, replacement.size);
                    replaced = true;
                }
            }
        }
        
        if (replaced) {
            o += replacement.size;
            i += needle.size;
            continue;
        }
        
        ret.str[o] = to_fix.str[i];
        o++; i++;
    }
    
    return ret;
}

u64 str_substr_count(string_const str, string_const needle) {
    u32 ct = 0;
    u64 idx = 0;
    while (true) {
        idx = str_find_first(str, needle, idx);
        if (idx == str.size)
            break;
        ct++;
        idx++;
    }
    return ct;
}

u64 str_find_first(string_const str, string_const needle, u32 offset) {
    u64 i = 0;
    if (needle.size > 0) {
        i = str.size;
        if (str.size >= needle.size) {
            i = offset;
            i8 c = needle.str[0];
            u64 one_past_last = str.size - needle.size + 1;
            for (; i < one_past_last; i++) {
                if (str.str[i] == c) {
                    if ((str.size - i) >= needle.size) {
                        string_const sub = { .str = str.str + i, .size = needle.size };
                        if (str_eq(sub, needle)) break;
                    }
                }
            }
            if (i == one_past_last) {
                i = str.size;
            }
        }
    }
    return i;
}

u64 str_find_last(string_const str, string_const needle, u32 offset) {
    u64 prev = str.size;
    if (offset == 0)
        offset = str.size;
    u64 idx = 0;
    while (true) {
        prev = idx;
        idx = str_find_first(str, needle, idx);
        if (idx >= offset) break;
        idx++;
    }
    return prev;
}

u32 str_hash(string_const str) {
    u32 hash = 2166136261u;
    for (int i = 0; i < str.size; i++) {
        hash ^= str.str[i];
        hash *= 16777619;
    }
    return hash;
}

void string_list_push_node(string_const_list* list, string_const_list_node* node) {
    if (!list->first && !list->last) {
        list->first = node;
        list->last = node;
    } else {
        list->last->next = node;
        list->last = node;
    }
    list->node_count += 1;
    list->total_size += node->str.size;
}

void string_list_push(M_Arena* arena, string_const_list* list, string_const str) {
    string_const_list_node* node = (string_const_list_node*) arena_alloc(arena, sizeof(string_const_list_node));
    node->str = str;
    string_list_push_node(list, node);
}

b8 string_list_equals(string_const_list* a, string_const_list* b) {
    if (a->total_size != b->total_size) return false;
    if (a->node_count != b->node_count) return false;
    string_const_list_node* curr_a = a->first;
    string_const_list_node* curr_b = b->first;
    
    while (curr_a != nullptr || curr_b != nullptr) {
        if (!str_eq(curr_a->str, curr_b->str)) return false;
        
        curr_a = curr_a->next;
        curr_b = curr_b->next;
    }
    return true;
}

b8 string_list_contains(string_const_list* a, string_const needle) {
    string_const_list_node* curr = a->first;
    while (curr != nullptr) {
        if (str_eq(needle, curr->str))
            return true;
        curr = curr->next;
    }
    return false;
}

string_const string_list_flatten(M_Arena* arena, string_const_list* list) {
    string_const final = str_alloc(arena, list->total_size);
    u64 current_offset = 0;
    for (string_const_list_node* node = list->first; node != nullptr; node = node->next) {
        memcpy(final.str + current_offset, node->str.str, node->str.size);
        current_offset += node->str.size;
    }
    return final;
}

void string_array_add(string_const_array* array, string data) {
    if (array->len + 1 > array->cap) {
        void* prev = array->elems;
        u32 new_cap = array->cap == 0 ? 8 : array->cap * 2;
        array->elems = calloc(new_cap, sizeof(string));
        memmove(array->elems, prev, array->len * sizeof(string));
        free(prev);
    }
    array->elems[array->len++] = data;
}

string string_array_remove(string_const_array* array, int idx) {
    if (idx >= array->len || idx < 0) return (string) {0};
    string value = array->elems[idx];
    if (idx == array->len - 1) {
        array->len--;
        return value;
    }
    string* from = array->elems + idx + 1;
    string* to = array->elems + idx;
    memmove(to, from, sizeof(string) * (array->len - idx - 1));
    array->len--;
    return value;
}

void string_array_free(string_const_array* array) {
    array->cap = 0;
    array->len = 0;
    free(array->elems);
}

//~ Encoding stuff

typedef struct str_decode {
    u32 codepoint;
    u32 size;
} str_decode;

string_utf16_const str16_cstring(u16 *cstr){
    u16 *ptr = cstr;
    for (;*ptr != 0; ptr += 1);
    string_utf16_const result = { cstr, (u64) (ptr - cstr) };
    return result;
}

static str_decode str_decode_utf8(u8 *str, u32 cap){
    u8 length[] = {
        1, 1, 1, 1, // 000xx
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        0, 0, 0, 0, // 100xx
        0, 0, 0, 0,
        2, 2, 2, 2, // 110xx
        3, 3,       // 1110x
        4,          // 11110
        0,          // 11111
    };
    u8 first_byte_mask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
    u8 final_shift[] = { 0, 18, 12, 6, 0 };
    
    str_decode result = {};
    if (cap > 0){
        result.codepoint = '#';
        result.size = 1;
        
        u8 byte = str[0];
        u8 l = length[byte >> 3];
        if (0 < l && l <= cap){
            u32 cp = (byte & first_byte_mask[l]) << 18;
            switch (l){
                case 4: cp |= ((str[3] & 0x3F) << 0);
                case 3: cp |= ((str[2] & 0x3F) << 6);
                case 2: cp |= ((str[1] & 0x3F) << 12);
                default: break;
            }
            cp >>= final_shift[l];
            
            result.codepoint = cp;
            result.size = l;
        }
    }
    
    return result;
}

static u32 str_encode_utf8(u8 *dst, u32 codepoint){
    u32 size = 0;
    if (codepoint < (1 << 8)){
        dst[0] = codepoint;
        size = 1;
    }
    else if (codepoint < (1 << 11)){
        dst[0] = 0xC0 | (codepoint >> 6);
        dst[1] = 0x80 | (codepoint & 0x3F);
        size = 2;
    }
    else if (codepoint < (1 << 16)){
        dst[0] = 0xE0 | (codepoint >> 12);
        dst[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[2] = 0x80 | (codepoint & 0x3F);
        size = 3;
    }
    else if (codepoint < (1 << 21)){
        dst[0] = 0xF0 | (codepoint >> 18);
        dst[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        dst[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[3] = 0x80 | (codepoint & 0x3F);
        size = 4;
    }
    else{
        dst[0] = '#';
        size = 1;
    }
    return size;
}

static str_decode str_decode_utf16(u16 *str, u32 cap){
    str_decode result = {'#', 1};
    u16 x = str[0];
    if (x < 0xD800 || 0xDFFF < x){
        result.codepoint = x;
    }
    else if (cap >= 2){
        u16 y = str[1];
        if (0xD800 <= x && x < 0xDC00 &&
            0xDC00 <= y && y < 0xE000){
            u16 xj = x - 0xD800;
            u16 yj = y - 0xDc00;
            u32 xy = (xj << 10) | yj;
            result.codepoint = xy + 0x10000;
            result.size = 2;
        }
    }
    return result;
}

static u32 str_encode_utf16(u16 *dst, u32 codepoint){
    u32 size = 0;
    if (codepoint < 0x10000){
        dst[0] = codepoint;
        size = 1;
    }
    else{
        u32 cpj = codepoint - 0x10000;
        dst[0] = (cpj >> 10) + 0xD800;
        dst[1] = (cpj & 0x3FF) + 0xDC00;
        size = 2;
    }
    return(size);
}

string_utf16_const str16_from_str8(M_Arena *arena, string str) {
    u16* memory = arena_alloc_array(arena, u16, str.size * 2 + 1);
    
    u16* dptr = memory;
    u8* ptr = str.str;
    u8* opl = str.str + str.size;
    for (; ptr < opl;){
        str_decode decode = str_decode_utf8(ptr, (u64)(opl - ptr));
        u32 enc_size = str_encode_utf16(dptr, decode.codepoint);
        ptr += decode.size;
        dptr += enc_size;
    }
    
    *dptr = 0;
    
    u64 alloc_count = str.size*2 + 1;
    u64 string_count = (u64)(dptr - memory);
    u64 unused_count = alloc_count - string_count - 1;
    arena_dealloc(arena, unused_count * sizeof(*memory));
    
    string_utf16_const result = { memory, string_count };
    return result;
}

string_const str8_from_str16(M_Arena *arena, string_utf16_const str) {
    u8 *memory = arena_alloc_array(arena, u8, str.size * 3 + 1);
    
    u8 *dptr = memory;
    u16 *ptr = str.str;
    u16 *opl = str.str + str.size;
    for (; ptr < opl;){
        str_decode decode = str_decode_utf16(ptr, (u64)(opl - ptr));
        u16 enc_size = str_encode_utf8(dptr, decode.codepoint);
        ptr += decode.size;
        dptr += enc_size;
    }
    
    *dptr = 0;
    
    u64 alloc_count = str.size*3 + 1;
    u64 string_count = (u64)(dptr - memory);
    u64 unused_count = alloc_count - string_count - 1;
    arena_dealloc(arena, unused_count * sizeof(*memory));
    
    string result = { memory, string_count };
    return result;
}
