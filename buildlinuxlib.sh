echo 'building for linux library'
set -e
gcc -DDEBUG -fpic -shared game.c -o libgame.so