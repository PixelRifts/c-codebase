if [ ! -d ./bin ]; then
   mkdir bin
fi

cc=clang

# ------------------
#      Options
# ------------------

Use_Render2D=false
Use_Physics2D=false
Use_UI=false

# ------------------
#    Main Project
# ------------------

# ==============
# Gets list of all C files
c_filenames= 
for entry in ./source/*.c
do
  c_filenames="$c_filenames $entry"
done

for entry in ./source/base/*.c
do
  c_filenames="$c_filenames $entry"
done

for entry in ./source/impl/*.c
do
  c_filenames="$c_filenames $entry"
done

for entry in ./source/core/*.c
do
  c_filenames="$c_filenames $entry"
done

for entry in ./source/os/*.c
do
  c_filenames="$c_filenames $entry"
done
# ==============

# ==============
# optional layers

if $Use_Render2D == true
then
  echo Optional Layer Selected: Render2D
  c_filenames="$c_filenames source/opt/render_2d.c"
fi

if $Use_Physics2D == true
then
  echo Optional Layer Selected: Physics2D
  c_filenames="$c_filenames source/opt/phys_2d.c"
fi

if $Use_UI == true
then
  echo Optional Layer Selected: UI
  c_filenames="$c_filenames source/opt/ui.c"
fi

# ==============



# ==============
compiler_flags="-Wall -Wvarargs -Werror -Wno-unused-function -Wno-format-security -Wno-incompatible-pointer-types-discards-qualifiers -Wno-unused-but-set-variable -Wno-int-to-void-pointer-cast"
include_flags="-Isource -Ithird_party/include -Ithird_party/source"
linker_flags="-g -lm -Lthird_party/lib"
defines="-D_DEBUG -D_CRT_SECURE_NO_WARNINGS"
output="-obin/codebase"
backend="-DBACKEND_GL33"
# ==============


# ==============
# TODO(voxel): REMOVE BACKEND SPECIFIC LINKS
if [ "$backend" -eq "-DBACKEND_D3D11" ]
then
  linker_flags="$linker_flags -ldxguid -ld3dcompiler"
fi
# ==============

# compiler_flags="$compiler_flags -fsanitize=address"

echo Building codebase.exe...
$cc $c_filenames $compiler_flags $defines $backend $include_flags $linker_flags $output
