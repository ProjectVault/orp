ORP System-on-chip
==================

Programming the Geophyte
------------------------

Before anything else,

1. Power-up and turn-on your Geophyte.
2. Once the board is turned-on, connect the Geophyte to your computer using the Geophyte's USB2-micro port.

Now we can run `urjtag`. To load the ORP system-on-chip onto the Geophyte, run `jtag` (which may require root):

    [ORP/hardware]$ su
    Password: 
    [ORP/hardware]# jtag
    UrJTAG 0.10 #1502
    Copyright (C) 2002, 2003 ETC s.r.o.
    Copyright (C) 2007, 2008, 2009 Kolja Waschk and the respective authors

    UrJTAG is free software, covered by the GNU General Public License, and you are
    welcome to change it and/or distribute copies of it under certain conditions.
    There is absolutely no warranty for UrJTAG.

    WARNING: UrJTAG may damage your hardware!
    Type "quit" to exit, "help" for help.

    jtag> 

We now need to instruct `urjtag` how to find the Geophyte. Issue the following command, repeating until you see the message `Connected to libftd2xx driver.`:

    jtag> cable ft2232 pid=0x6010 vid=0x403
    Connected to libftdi driver.
    jtag> cable ft2232 pid=0x6010 vid=0x403
    Connected to libftd2xx driver.
    jtag> 

Then we detect the FPGA:

    jtag> detect
    IR length: 6
    Chain length: 1
    Device Id: 00010011011000110001000010010011 (0x0000000013631093)
      Manufacturer: Xilinx
      Unknown part!
    chain.c(149) Part 0 without active instruction
    chain.c(200) Part 0 without active instruction
    chain.c(149) Part 0 without active instruction
    jtag> 

Now we set the instruction length, then instruct it to program (here we use the softcore `geophyte_fpga.svf`:

    jtag> instruction length 6
    jtag> svf geophyte_fpga.svf progress
    ...
    Scanned device output matched expected TDO values.
    jtag>

We can now exit `urjtag` and go back to being a regular user:

    jtag> quit
    [ORP/hardware]# exit
    exit
    [ORP/hardware]$

The Geophyte has now been programmed with mselSoC.


Loading and running a binary image on the system-on-chip
--------------------------------------------------------

If you haven't already, load the system-on-chip onto the Geophyte using the instructions above.

This next part requires two terminals: one to run `openocd`, which acts as a GDB-to-jtag bridge, another to run GDB, which we can use to load an image into memory (and debug it if we desire).

To run `openocd`, we must run `bash openocd/openocd.sh` (which may require root):

    [ORP/hardware]$ su
    Password: 
    [ORP/hardware]# bash openocd/openocd.sh
    Open On-Chip Debugger 0.8.0 (2015-03-19-02:50)
    Licensed under GNU GPL v2
    For bug reports, read
	    http://openocd.sourceforge.net/doc/doxygen/bugs.html
    Info : only one transport option; autoselect 'jtag'
    adapter speed: 1000 kHz
    Info : xilinx_bscan tap selected
    Info : adv debug unit selected
    Info : Option 1 is passed to adv debug unit
    Warn : Using DEPRECATED interface driver 'ft2232'
    Info : Consider using the 'ftdi' interface driver, with configuration files in interface/ftdi/...
    Info : max TCK change to: 30000 kHz
    Info : clock speed 1000 kHz
    Info : JTAG tap: or1200.cpu tap/device found: 0x13631093 (mfg: 0x049, part: 0x3631, ver: 0x1)
    Info : adv debug unit is configured with option ADBG_USE_HISPEED
    Halting processor
    target state: halted
    Chip is or1200.cpu, Endian: big, type: or1k
    Target ready...

Now `openocd` is running until we exit with Ctrl-C.

While `openocd` is running, open another terminal. Source the or1k environment variables, then launch the `or1k-elf-gdb`, passing the binary image you wish to load onto the Geophyte (in this example, we are loading `$ORP/bin/bundle-urchat`):

    [ORP/hardware]$ or1k-elf-gdb $ORP/bin/bundle-urchat
    GNU gdb (GDB) 7.6.50.20130930-cvs
    Copyright (C) 2013 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
    and "show warranty" for details.
    This GDB was configured as "--host=x86_64-unknown-linux-gnu --target=or1k-elf".
    Type "show configuration" for configuration details.
    For bug reporting instructions, please see:
    <http://www.gnu.org/software/gdb/bugs/>.
    Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.
    For help, type "help".
    Type "apropos word" to search for commands related to "word"...
    Reading symbols from ../platform/or1k/slash/bin/bundle-urchat...done.
    (gdb) 

We connect to `openocd` as a remote debugging target:

    (gdb) target remote :3333
    Remote debugging using :3333
    0xf0000130 in ?? ()
    (gdb) 

We load the binary image:

    (gdb) load
    Loading section .vectors, size 0x1158 lma 0x100000
    Loading section .text, size 0x8fb2 lma 0x102000
    Loading section .data, size 0x41 lma 0x10afb2
    Start address 0x100100, load size 41291
    Transfer rate: 118 KB/sec, 8258 bytes/write.
    (gdb) 

Then we resume execution:

    (gdb) continue
    Continuing.

At this point our binary image is running on the system-on-chip on the Geophyte, with `gdb` attached as a remote debugger. We can set breakpoints, inspect memory, etc.

If we want to unplug the Geophyte from USB, we can safely do-so by first closing `openocd`, then `gdb`, then unplugging the Geophyte.

