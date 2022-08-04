#include "png.h"

#include <stdlib.h>
#include <string.h>

typedef struct file {
	u32 size;
	u8* stream;
} file;

static file load_file(string s) {
	file f = {0};
	FILE* in = fopen((const char*) s.str, "rb");
	if (in) {
		fseek(in, 0, SEEK_END);
		f.size = ftell(in);
		fseek(in, 0, SEEK_SET);
		
		f.stream = malloc(f.size);
		fread(f.stream, f.size, 1, in);
		fclose(in);
	} else {
		LogError("PNG File Loading Failed %.*s", str_expand(s));
	}
}

static void free_file(file f) {
	free(f.stream);
}

void* A_LoadPNG(string path) {
	file f = load_file(path);
	free_file(f);
}
