# You sourced the or1k-devenv-settings.sh vars, ya?

QEMU=/home/sean/lol/bin/qemu-system-or32

$QEMU -S -gdb tcp::60000 -nographic -kernel $1

