echo 'building for windows'
set -e
x86_64-w64-mingw32-gcc -DDEBUG win.c -o win.exe -L./ -std=c89 -luser32 -lkernel32 -lgdi32 -lgame
wine win.exe