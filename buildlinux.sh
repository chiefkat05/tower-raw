echo 'building for linux'
set -e
gcc -DDEBUG linux.c -o linux -L./ -std=c89 -lxcb -lxcb-xkb -lxcb-image -lopenal -lrt -l:./libgame.so
./linux