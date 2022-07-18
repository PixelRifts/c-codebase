@ECHO off
SetLocal EnableDelayedExpansion

IF NOT EXIST bin mkdir bin

SET cc=clang

REM Gets list of all C files
SET c_filenames= 
FOR %%f in (source\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\base\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\impl\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\core\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\os\*.c) do SET c_filenames=!c_filenames! %%f

REM SET compiler_flags=
SET compiler_flags=-Wall -Wvarargs -Werror -Wno-unused-function -Wno-format-security -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unused-but-set-variable -Wno-int-to-void-pointer-cast

SET include_flags=-Isource -Ithird_party/include -Ithird_party/source
SET linker_flags=-g -lmsvcrt -lshell32 -luser32 -lwinmm -luserenv -lgdi32 -Lthird_party/lib
SET defines=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS
SET output=-obin/codebase.exe
SET backend=-DBACKEND_GL46

ECHO "Building codebase.exe..."
%cc% %c_filenames% %compiler_flags% %defines% %backend% %include_flags% %linker_flags% %output%
