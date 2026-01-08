echo 'building windows library'
clear
set -e
x86_64-w64-mingw32-gcc -shared game.c -o game.dll