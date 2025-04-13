#!/bin/bash

if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

rm /etc/ld.so.preload
rm /usr/local/share/gcc.so

sudo rm /dev/shm/b.a

echo "[*] ElfDoor-gcc removed! [*]"
