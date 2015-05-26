Building mselSoC
================

If you haven't already,

* Source your Xilinx settings (for example: `source /opt/Xilinx/14.7/ISE_DS/settings64.sh`).
* Source the Core Tools settings (`source ../cores-devenv-settings.sh`).

Doing so will set various environment variables which are necessary for the building of mselSoC:

    $ source /opt/Xilinx/14.7/ISE_DS/settings64.sh
    . /opt/Xilinx/14.7/ISE_DS/common/.settings64.sh /opt/Xilinx/14.7/ISE_DS/common
    . /opt/Xilinx/14.7/ISE_DS/EDK/.settings64.sh /opt/Xilinx/14.7/ISE_DS/EDK
    . /opt/Xilinx/14.7/ISE_DS/PlanAhead/.settings64.sh /opt/Xilinx/14.7/ISE_DS/PlanAhead
    . /opt/Xilinx/14.7/ISE_DS/ISE/.settings64.sh /opt/Xilinx/14.7/ISE_DS/ISE
    $ which impact
    /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/impact

Now we enter the `src` directory and run two commands:

    $ pushd src
    $ time fusesoc build geophyte
    ...
    Process "Generate Programming File" completed successfully
    INFO:TclTasksC:1850 - process run : Generate Programming File is done.

    real	26m36.558s
    user	26m20.543s
    sys	0m7.557s

    $ impact -batch geophyte_fpga.cmd && impact -batch geophyte.cmd
    ...
    $ 

This generates two files (`geophyte_fpga.svf` and `geophyte.svf`). Both can be loaded onto the Geophyte using urjtag. When loaded onto a Geophyte, `geophyte.svf` persists across reboots while `geophyte_fpga.svf` does not. They are otherwise the same.
