echo 'building for linux'
set -e
gcc -DDEBUG linux.c -o linux -lxcb -lxcb-xkb -lxcb-image -lopenal -lrt -lportaudio
./linux