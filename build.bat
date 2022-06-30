@ECHO off
SetLocal EnableDelayedExpansion

IF NOT EXIST bin mkdir bin

REM Copy over dlls
IF NOT EXIST bin\glfw3.dll (
	ECHO INFO: glfw3.dll not found in bin folder: Copying it over from third_party\lib
	COPY "third_party\lib\glfw3.dll" "bin"
)

SET cc=clang

REM Get's list of all C files
SET c_filenames= 

FOR %%f in (source\*.c) do (
	SET c_filenames=!c_filenames! %%f
)
FOR %%f in (source\base\*.c) do (
	SET c_filenames=!c_filenames! %%f
)
FOR %%f in (source\impl\*.c) do (
	SET c_filenames=!c_filenames! %%f
)
SET c_filenames=!c_filenames! source\os\os.c

SET compiler_flags=-Wall -Wvarargs -Werror -Wno-unused-function -Wno-format-security -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unused-but-set-variable
SET include_flags=-Isource -Ithird_party/include
SET linker_flags=-g -lshell32 -luser32 -lwinmm -luserenv -lgdi32 -lopengl32 -Lthird_party/lib -lglfw3dll
SET defines=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS
SET output=-o bin/codebase.exe

ECHO "Building codebase.exe..."
%cc% %c_filenames% %compiler_flags% %defines% %include_flags% %linker_flags% %output%
