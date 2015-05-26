setMode -pff
setMode -pff
addConfigDevice  -name "geophyte" -path "."
setSubmode -pffspi
setAttribute -configdevice -attr multibootBpiType -value ""
addDesign -version 0 -name "0"
setMode -pff
addDeviceChain -index 0
setMode -pff
addDeviceChain -index 0
setAttribute -configdevice -attr compressed -value "FALSE"
setAttribute -configdevice -attr compressed -value "FALSE"
setAttribute -configdevice -attr autoSize -value "TRUE"
setAttribute -configdevice -attr fileFormat -value "mcs"
setAttribute -configdevice -attr fillValue -value "FF"
setAttribute -configdevice -attr swapBit -value "FALSE"
setAttribute -configdevice -attr dir -value "UP"
setAttribute -configdevice -attr multiboot -value "FALSE"
setAttribute -configdevice -attr multiboot -value "FALSE"
setAttribute -configdevice -attr spiSelected -value "TRUE"
setAttribute -configdevice -attr spiSelected -value "TRUE"
setMode -pff
setMode -pff
setMode -pff
setMode -pff
addDeviceChain -index 0
setMode -pff
addDeviceChain -index 0
setSubmode -pffspi
setMode -pff
setAttribute -design -attr name -value "0000"
addDevice -p 1 -file "./build/geophyte/bld-ise/orpsoc_top.bit"
setMode -pff
setSubmode -pffspi
generate -generic

setMode -bs
setMode -bs
setMode -bs
setMode -bs
setCable -port svf -file "./geophyte.svf"
addDevice -p 1 -file "./build/geophyte/bld-ise/orpsoc_top.bit"
attachflash -position 1 -spi "N25Q128"
assignfiletoattachedflash -position 1 -file "./geophyte.mcs"
program -p 1 -dataWidth 4 -spionly -e -v -loadfpga 
quit

