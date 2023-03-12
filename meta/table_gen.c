#include "defines.h"
#include <stdio.h>

#include "md/md.h"
#include "md.c"

#define MD_StringExpand(s) (int)s.size, (char*)s.str
#define Print(s) printf("%.*s\n", MD_StringExpand(s))

typedef struct Table Table;
struct Table {
	Table* next;
	
	MD_String8 name;
	MD_String8List properties;
	
	u32 element_count;
	u32 member_count;
	MD_String8List* members;
};

typedef struct Replacement Replacement;
struct Replacement {
	Replacement* next;
	MD_u64 start;
	MD_u64 end;
	MD_String8 member;
};

MD_String8 FindStringMemberFromTable(Table* table, MD_Node* member_node, MD_String8 member_name, u32 member_index) {
	MD_ArenaTemp scratch = MD_GetScratch(0, 0);
	
	u32 found_index = 0;
	MD_String8Node* curr;
	for (curr = table->properties.first;
		 curr != nullptr;
		 curr = curr->next) {
		if (MD_S8Match(curr->string, member_name, 0)) {
			break;
		}
		found_index += 1;
	}
	
	if (curr == nullptr) {
		MD_String8 error_str = MD_S8Fmt(scratch.arena, "Unknown member name %.*s for the table %.*s", MD_StringExpand(member_name), MD_StringExpand(table->name));
		MD_CodeLoc code_loc = MD_CodeLocFromNode(member_node);
		MD_PrintMessage(stderr, code_loc, MD_MessageKind_Error, error_str);
		MD_ReleaseScratch(scratch);
		return (MD_String8) { .str = nullptr, .size = 0 };
	}
	
	MD_String8List member_list = table->members[found_index];
	u32 i = 0;
	for (curr = member_list.first;
		 curr != nullptr;
		 curr = curr->next) {
		if (member_index == i) {
			MD_ReleaseScratch(scratch);
			return curr->string;
		}
		i += 1;
	}
	
	MD_String8 error_str = MD_S8Fmt(scratch.arena, "Unknown member name %.*s for the table %.*s", MD_StringExpand(member_name), MD_StringExpand(table->name));
	MD_CodeLoc code_loc = MD_CodeLocFromNode(member_node);
	MD_PrintMessage(stderr, code_loc, MD_MessageKind_Error, error_str);
	MD_ReleaseScratch(scratch);
	return (MD_String8) { .str = nullptr, .size = 0 };
}

Table* HandleTableNode(MD_Arena* arena, MD_Node* decl) {
	MD_Node* table_tag = MD_TagFromString(decl, MD_S8Lit("table"), 0);
	
	Table* ret = MD_ArenaPush(arena, sizeof(Table));
	MemoryZeroStruct(ret, Table);
	
	ret->name = decl->string;
	
	for (MD_EachNode(property, table_tag->first_child)) {
		MD_S8ListPush(arena, &ret->properties, property->string);
		ret->member_count += 1;
	}
	
	ret->members = MD_ArenaPush(arena, sizeof(MD_String8List) * ret->member_count);
	MemoryZero(ret->members, sizeof(MD_String8List) * ret->member_count);
	
	ret->element_count = 0;
	for (MD_Node* elem = decl->first_child;
		 elem != MD_NilNode();
		 elem = elem->next) {
		
		u32 i = 0;
		for (MD_Node* value = elem->first_child;
			 value != MD_NilNode();
			 value = value->next) {
			MD_S8ListPush(arena, &ret->members[i], value->string);
			i += 1;
		}
		ret->element_count += 1;
	}
	
	return ret;
}

MD_String8 ReduceEndingDoubleBackslashes(MD_String8 in) {
	// @awkward
	MD_String8 out = in;
	if (in.size > 2) {
		if (in.str[in.size-1] == in.str[in.size-2] &&
			in.str[in.size-2] == '\\') {
			out.size -= 1;
		}
	}
	return out;
}


MD_u64 MD_S8SubstrCount(MD_String8 str, MD_String8 needle) {
    MD_u32 ct = 0;
    MD_u64 idx = 0;
    while (true) {
        idx = MD_S8FindSubstring(str, needle, idx, 0);
        if (idx == str.size)
            break;
        ct++;
        idx++;
    }
    return ct;
}

MD_String8 MD_S8ReplaceAll(MD_Arena* arena, MD_String8 to_fix, MD_String8 needle, MD_String8 replacement) {
    if (needle.size == 0) return to_fix;
    MD_u64 replaceable = MD_S8SubstrCount(to_fix, needle);
    if (replaceable == 0) return to_fix;
    
    MD_String8 ret;
    ret.size = (to_fix.size - replaceable * needle.size) + (replaceable * replacement.size);
	ret.str = MD_ArenaPush(arena, ret.size + 1);
	ret.str[ret.size] = '\0';
    
    b8 replaced;
    MD_u64 o = 0;
    for (MD_u64 i = 0; i < to_fix.size;) {
        replaced = false;
        if (to_fix.str[i] == needle.str[0]) {
            if ((to_fix.size - i) >= needle.size) {
                MD_String8 sub = { .str = to_fix.str + i, .size = needle.size };
                if (MD_S8Match(sub, needle, 0)) {
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


MD_String8List HandleExpanderLine(MD_Arena* arena, MD_String8 expandme, MD_Node* curr, MD_Node* the_tag, Table* first_table) {
	MD_String8List to_ret_lines = {0};
	
	MD_String8 used_table_name = the_tag->first_child->string;
	Table* table_to_unroll = nullptr;
	for (Table* the_table = first_table;
		 the_table != nullptr;
		 the_table = the_table->next) {
		if (MD_S8Match(the_table->name, used_table_name, 0)) {
			table_to_unroll = the_table;
			break;
		}
	}
	
	if (table_to_unroll == nullptr) {
		MD_String8 error_str = MD_S8Fmt(arena, "Could not find the table %.*s",
										MD_StringExpand(used_table_name));
		MD_CodeLoc code_loc = MD_CodeLocFromNode(the_tag);
		MD_PrintMessage(stderr, code_loc, MD_MessageKind_FatalError, error_str);
		exit(1);
	}
	
	MD_ArenaTemp scratch = MD_GetScratch(&arena, 1);
	
	Replacement* first = nullptr;
	Replacement* last = nullptr;
	
	u32 finding_index = 0;
	while (finding_index != expandme.size) {
		MD_u64 loc = MD_S8FindSubstring(expandme, MD_S8Lit("$("), finding_index, 0);
		
		if (loc != expandme.size) {
			MD_u64 endloc = MD_S8FindSubstring(expandme, MD_S8Lit(")"), loc, 0);
			
			if (endloc == expandme.size) {
				MD_String8 error_str = MD_S8Lit("Unterminated $( in the expansion");
				MD_CodeLoc code_loc = MD_CodeLocFromNode(the_tag);
				MD_PrintMessage(stderr, code_loc, MD_MessageKind_FatalError, error_str);
				exit(1);
			}
			MD_String8 the_member = MD_S8Substring(expandme, loc+2, endloc);
			
			Replacement* new_replacement = MD_ArenaPush(scratch.arena, sizeof(Replacement));
			MemoryZeroStruct(new_replacement, Replacement);
			new_replacement->start = loc;
			new_replacement->end = endloc;
			new_replacement->member = the_member;
			
			MD_QueuePush(first, last, new_replacement);
			finding_index = endloc;
		} else {
			finding_index = loc;
		}
	}
	
	MD_String8 to_push = {0};
	for (u32 i = 0; i < table_to_unroll->element_count; i++) {
		MD_String8List chops = {0};
		
		u64 segment_start = 0;
		for (Replacement* rep = first;
			 rep != nullptr;
			 rep = rep->next) {
			MD_String8 unmodded_segment = MD_S8Substring(expandme, segment_start, rep->start);
			MD_S8ListPush(scratch.arena, &chops, unmodded_segment);
			
			MD_String8 modded_segment = FindStringMemberFromTable(table_to_unroll, curr, rep->member, i);
			MD_S8ListPush(scratch.arena, &chops, modded_segment);
			
			segment_start = rep->end+1;
		}
		MD_String8 final_segment = MD_S8Substring(expandme, segment_start, expandme.size);
		MD_S8ListPush(scratch.arena, &chops, final_segment);
		
		to_push = MD_S8ListJoin(arena, chops, 0);
		to_push = MD_S8ReplaceAll(arena, to_push, MD_S8Lit("\\\\"), MD_S8Lit("\\"));
		
		MD_S8ListPush(arena, &to_ret_lines, to_push);
	}
	MD_ReleaseScratch(scratch);
	return to_ret_lines;
}

MD_String8List HandleGeneratorNode(MD_Arena* arena, Table* first_table, Table* last_table,  MD_Node* decl) {
	MD_String8List generated = {0};
	
	//MD_PrintDebugDumpFromNode(stdout, decl, MD_GenerateFlags_All);
	for (MD_EachNode(curr, decl->first_child)) {
		if (MD_NodeHasTag(curr, MD_S8Lit("expander"), 0)) {
			
			MD_Node* the_tag = MD_TagFromString(curr, MD_S8Lit("expander"), 0);
			if (the_tag->first_child == MD_NilNode()) {
				MD_String8 error_str = MD_S8Lit("@expander note must have the table that it works with");
				MD_CodeLoc code_loc = MD_CodeLocFromNode(the_tag);
				MD_PrintMessage(stderr, code_loc, MD_MessageKind_FatalError, error_str);
				exit(1);
			}
			
			if (MD_NodeIsNil(curr->first_child)) {
				// Single Line expander
				MD_String8List expanded_lines = HandleExpanderLine(arena, curr->string, curr, the_tag, first_table);
				MD_S8ListConcat(&generated, &expanded_lines);
			} else {
				// Multi Line expander
				MD_ArenaTemp scratch = MD_GetScratch(&arena, 1);
				MD_String8List combined_multiline_chops = {0};
				for (MD_EachNode(subline, curr->first_child)) {
					MD_S8ListPush(scratch.arena, &combined_multiline_chops, subline->string);
				}
				MD_String8 to_expand = MD_S8ListJoinMid(arena, combined_multiline_chops, MD_S8Lit("\n"));
				MD_ReleaseScratch(scratch);
				
				MD_String8List expanded_lines = HandleExpanderLine(arena, to_expand, curr, the_tag, first_table);
				MD_S8ListConcat(&generated, &expanded_lines);
			}
		} else {
			MD_String8 to_push = curr->string;
			to_push = ReduceEndingDoubleBackslashes(to_push);
			
			MD_S8ListPush(arena, &generated, to_push);
		}
	}
	return generated;
}

int main(int argc, char** argv) {
	if (argc == 1) {
		printf("Please pass in a filepath to generate code from!\n");
		exit(1);
	} else if (argc > 2) {
		printf("Too many arguments! We only need from and to filepaths\n");
		exit(1);
	}
	
	MD_String8 fp = MD_S8CString(argv[1]);
	MD_u64 finaldotloc = MD_S8FindSubstring(fp, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
	MD_String8 fp_wo_ext = MD_S8Prefix(fp, finaldotloc);
	MD_String8List fp_chops = {0};
	
	MD_Arena* arena = MD_ArenaAlloc();
	
	MD_S8ListPush(arena, &fp_chops, fp_wo_ext);
	MD_S8ListPush(arena, &fp_chops, MD_S8Lit(".h"));
	MD_String8 header_fp = MD_S8ListJoin(arena, fp_chops, 0);
	fp_chops.last = fp_chops.first;
	MD_S8ListPush(arena, &fp_chops, MD_S8Lit(".c"));
	MD_String8 impl_fp = MD_S8ListJoin(arena, fp_chops, 0);
	
	
	MD_ParseResult result = MD_ParseWholeFile(arena, fp);
	
	if (result.errors.node_count != 0) {
		MD_Message* error = result.errors.first;
		while (error) {
			printf("%.*s\n", MD_StringExpand(error->string));
			error = error->next;
			exit(1);
		}
	}
	
	MD_Node* root = result.node;
	
	Table* table_list_first = nullptr;
	Table* table_list_last = nullptr;
	
	
	FILE* header_file = fopen((const char*)header_fp.str, "w");
	FILE* impl_file = fopen((const char*)impl_fp.str, "w");
	
	fwrite("// This file is autogenerated from the corresponding .mdesk file\n\n", sizeof(u8), strlen("// This file is autogenerated from the corresponding .mdesk file\n\n"), header_file);
	fwrite("// This file is autogenerated from the corresponding .mdesk file\n\n", sizeof(u8), strlen("// This file is autogenerated from the corresponding .mdesk file\n\n"), impl_file);
	
	for (MD_EachNode(top_level, root->first_child)) {
		if (MD_NodeHasTag(top_level, MD_S8Lit("table"), 0)) {
			Table* table = HandleTableNode(arena, top_level);
			MD_QueuePush(table_list_first, table_list_last, table);
			printf("Loaded table %.*s\n", MD_StringExpand(top_level->string));
			
		} else if (MD_NodeHasTag(top_level, MD_S8Lit("generator"), 0)) {
			MD_String8List generated_lines = HandleGeneratorNode(arena, table_list_first, table_list_last, top_level);
			MD_String8 generated = MD_S8ListJoinMid(arena, generated_lines, MD_S8Lit("\n"));
			printf("Generated %.*s\n", MD_StringExpand(top_level->string));
			
			MD_Node* generator_tag = MD_TagFromString(top_level, MD_S8Lit("generator"), 0);
			if (MD_NodeHasChild(generator_tag, MD_S8Lit("C"), 0)) {
				fwrite(generated.str, sizeof(MD_u8), generated.size, impl_file);
				fwrite("\n\n", sizeof(MD_u8), 2, impl_file);
			} else {
				fwrite(generated.str, sizeof(MD_u8), generated.size, header_file);
				fwrite("\n\n", sizeof(MD_u8), 2, header_file);
			}
			
		} else {
			MD_String8 error_str = MD_S8Lit("Top level node requires either @table or @generator");
			MD_CodeLoc code_loc = MD_CodeLocFromNode(top_level);
			MD_PrintMessage(stderr, code_loc, MD_MessageKind_FatalError, error_str);
			exit(1);
		}
	}
	
	fclose(impl_file);
	fclose(header_file);
	
	MD_ArenaRelease(arena);
	
}
