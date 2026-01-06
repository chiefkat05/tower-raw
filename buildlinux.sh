echo 'building for linux'
set -e
gcc -DDEBUG linux.c -o linux -std=c89 -lxcb -lxcb-xkb -lxcb-image -lopenal -lrt -Ofast
./linux