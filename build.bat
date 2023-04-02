@ECHO off
SetLocal EnableDelayedExpansion
IF NOT EXIST bin mkdir bin

SET cc=clang

REM ------------------
REM      Options
REM ------------------

SET Use_Render2D=false
SET Use_Physics2D=false
SET Use_UI=true

REM ------------------
REM    Main Project
REM ------------------

REM ==============
REM Gets list of all C files
SET c_filenames= 
FOR %%f in (source\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\base\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\impl\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\core\*.c) do SET c_filenames=!c_filenames! %%f
FOR %%f in (source\os\*.c) do SET c_filenames=!c_filenames! %%f
REM ==============

REM ==============
REM optional layers

if %Use_Render2D% == true (
  ECHO Optional Layer Selected: Render2D
  SET c_filenames=!c_filenames! source\opt\render_2d.c
)

if %Use_Physics2D% == true (
  ECHO Optional Layer Selected: Physics2D
  SET c_filenames=!c_filenames! source\opt\phys_2d.c
)

if %Use_UI% == true (
  ECHO Optional Layer Selected: UI
  SET c_filenames=!c_filenames! source\opt\ui.c
)

REM ==============



REM ==============
SET compiler_flags=-Wall -Wvarargs -Werror -Wno-unused-function -Wno-format-security -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unused-but-set-variable -Wno-int-to-void-pointer-cast
SET include_flags=-Isource -Ithird_party/include -Ithird_party/source
SET linker_flags=-g -lshell32 -luser32 -lwinmm -luserenv -lgdi32 -Lthird_party/lib
SET defines=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS
SET output=-obin/codebase.exe
SET backend=-DBACKEND_D3D11
REM ==============


REM ==============
REM TODO(voxel): REMOVE BACKEND SPECIFIC LINKS
if %backend% == -DBACKEND_D3D11 (
  SET linker_flags=%linker_flags% -ldxguid -ld3dcompiler
)
REM ==============

REM SET compiler_flags=!compiler_flags! -fsanitize=address

ECHO Building codebase.exe...
%cc% %c_filenames% %compiler_flags% %defines% %backend% %include_flags% %linker_flags% %output%
