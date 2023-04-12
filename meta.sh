#!/bin/sh

cc=gcc

# ------------------
#    Metaprograms
# ------------------

compiler_flags="-Wall -Wvarargs -Werror -Wno-unused-but-set-variable -Wno-unused-function"
include_flags="-Isource -Ithird_party/include -Ithird_party/source"
linker_flags="-g -Lthird_party/lib"
defines="-D_DEBUG -D_CRT_SECURE_NO_WARNINGS"
output="-ometa/table_gen"

echo Building table_gen.exe...
$cc meta/table_gen.c $compiler_flags $defines $include_flags $linker_flags $output


echo Running Metaprogram on source/opt/meta/ui_stacks.mdesk
meta/table_gen "source/opt/meta/ui_stacks.mdesk"
