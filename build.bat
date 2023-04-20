@ECHO off
SetLocal EnableDelayedExpansion
IF NOT EXIST bin mkdir bin
IF NOT EXIST bin\int mkdir bin\int

call vcvarsall.bat x64
SET cc=cl.exe

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


SET backend=BACKEND_D3D11


REM ==============
if %cc% == cl.exe (
  SET compiler_flags=/Zc:preprocessor /wd4090 /wd5105
  SET include_flags=/I.\source\ /I.\third_party\include\ /I.\third_party\source\
  SET linker_flags=/link /DEBUG /LIBPATH:.\third_party\lib shell32.lib user32.lib winmm.lib userenv.lib gdi32.lib
  SET output=/Fe.\bin\codebase /Fo.\bin\int\
  SET defines=/D_DEBUG /D_CRT_SECURE_NO_WARNINGS /D%backend%
)

if %cc% == clang (
  SET compiler_flags=-Wall -Wvarargs -Werror -Wno-unused-function -Wno-format-security -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unused-but-set-variable -Wno-int-to-void-pointer-cast
  SET include_flags=-Isource -Ithird_party/include -Ithird_party/source
  SET linker_flags=-g -lshell32 -luser32 -lwinmm -luserenv -lgdi32 -Lthird_party/lib
  SET output=-obin/codebase.exe
  SET defines=-D_DEBUG -D_CRT_SECURE_NO_WARNINGS -D%backend%
)

REM ==============


REM ==============
REM TODO(voxel): REMOVE BACKEND SPECIFIC LINKS
if %backend% == BACKEND_D3D11 (
  if %cc% == cl.exe (
    SET linker_flags=%linker_flags% dxguid.lib d3dcompiler.lib
  )
  if %cc% == clang (
    SET linker_flags=%linker_flags% -ldxguid -ld3dcompiler
  )
)
REM ==============

REM SET compiler_flags=!compiler_flags! -fsanitize=address

ECHO Building codebase.exe...
%cc% %compiler_flags% %c_filenames% %defines% %include_flags% %output% %linker_flags%
