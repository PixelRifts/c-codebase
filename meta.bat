@ECHO OFF

SET cc=clang

REM ------------------
REM    Metaprograms
REM ------------------

SET compiler_flags=-Wall -Wvarargs -Werror -Wno-unused-but-set-variable -Wno-unused-function
SET include_flags=-Isource -Ithird_party/include -Ithird_party/source
SET linker_flags=-g -Lthird_party/lib
SET defines=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS
SET output=-ometa/table_gen.exe

ECHO Building table_gen.exe...
%cc% meta/table_gen.c %compiler_flags% %defines% %include_flags% %linker_flags% %output%


ECHO Running Metaprogram on source\opt\meta\ui_stacks.mdesk
meta\table_gen.exe "source\opt\meta\ui_stacks.mdesk"
