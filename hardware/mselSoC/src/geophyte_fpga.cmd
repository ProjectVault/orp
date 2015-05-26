setMode -bs
setMode -bs
setMode -bs
setMode -bs
setCable -port svf -file "./geophyte_fpga.svf"
addDevice -p 1 -file "./build/geophyte/bld-ise/orpsoc_top.bit"
program -p 1 
quit

