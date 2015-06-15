
QEMU + Support for the Open Reference Platform
===========================================================

This is a fork of the official Qemu repository with support for the Open Reference 
Platform.  

Build Instructions
------------------

To build the Qemu emulator for the Open Reference Platform, run the
following commands:

    ./configure --prefix=/path/to/install-dir --target-list=or32-softmmu
    make; make install

If you would like to debug Qemu itself, you will also need the `--enable-debug`
flag to the configure command.

If you want to enable ARM support, compile Qemu with `arm-softmmu` added to the
`--target-list` flag (use commas to separate different targets).  Note that the
Qemu version of ARM does not support any additional hardware components (TRNG,
AES, SHA, or faux filesystem).

Execution and Debugging
-----------------------

Qemu expects a disk image as its command-line argument.  To get around this, use
the -kernel flag; this may change in the future.  Qemu also starts a GUI by
default, which I normally disable with -nographic

    qemu-system-or32 -kernel path/to/kernel -nographic

During exection, you can press `C-a c` to drop into a command prompt environment
within Qemu; type help to see a list of available commands.  For example, to abort
the emulator process, type `C-a x`.

To debug the kernel, start qemu with the `-s -S` flags, which starts the kernel in
the 'halted' state, and initializes a gdb server on localhost:1234.
    
    qemu-system-or32 -kernel path/to/kernel -nographic -s -S

If you would like to have Qemu listen on a different port, you can use 
`-gdb tcp::<portnum>` in place of the `-s` flag.

Once you have started Qemu, run the following commands to attach the debugger to
your kernel process: 

    or1k-elf-gdb path/to/kernel
    target remote :<portnum>

where `<portnum>` is the specified port you started Qemu with.  You can now start
the kernel with the `continue` command.  You can also set breakpoints and 
single-step as normal.

If you want to debug Qemu itself with GDB, you will probably find it helpful to 
issue the following command in GDB before you start the kernel running:

    handle SIGUSR1 noprint

Additions to Qemu
-----------------

The source code has been modified to support the hardware TRNG, AES, and SHA-256 
cores, the E-521 elliptic curve montgomery ladder core, and the faux file system 
interface.  Most of the code changes are in `hw/openrisc`, but some modifications 
have been made to files in `target-openrisc` to support custom interrupts:

* Interrupt `0x10` fires when `wfile` is written to;
* Interrupt `0x11` fires when data from `rfile` is acknowledged

The Faux Filesystem Interface (FFS) is emulated by opening two TCP/IP sockets, one
for `rfile` and one for `wfile`.  Data is written to these sockets just like they
would be written to the actual filesystem.  The default ports are 9998 (`rfile`) 
and 9999 (`wfile`), on 127.0.0.1.  You can control the host IP address and
the socket port numbers with the following command-line options to qemu:

    -ffs host_ip=127.0.1.1 \
    -ffs rfile=7777 \
    -ffs wfile=7778 \

Original Qemu Information
-------------------------

Read the documentation in qemu-doc.html or on http://wiki.qemu-project.org

