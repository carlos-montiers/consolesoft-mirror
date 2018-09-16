@Echo Off
gcc -Wl,-e,__start -nostartfiles -m32 -Os -s bg.c -lwinmm -o BG.EXE -fno-omit-frame-pointer -flto -Wall
gcc -Wl,-e,__start -nostartfiles -m32 -O3 -s sprite.c assoc-array.c -o SPRITE.EXE -fno-omit-frame-pointer -Wall
gcc -Wl,-e,__start -nostartfiles -m32 -O3 -s sound.c -lwinmm -o SOUND.EXE -Wall
gcc -Wl,-e,__start -nostartfiles -m32 -O3 -s pixelfnt.c memory-mapped-file.c -lgdi32 -o PIXELFNT.EXE -fno-omit-frame-pointer -Wall
gcc -Wl,-e,__start -nostartfiles -m32 -Os editor.c -o EDITOR.EXE
