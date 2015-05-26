set remotetimeout unlimited
target remote :3333

file tests/mtc_test
load tests/mtc_test

# Setup traps for all terminal errors
b *0x100100

# BusError
b *0x100200 

# DataPageFault
b *0x100300 

# InstructionPageFault
b *0x100400

# TickTimerHandler
# b *0x500

# AlignmentError
b *0x100600 

# IllegalInstruction
b *0x100700 

# ExternalInterrupt
b *0x100800 

# DTLBMiss
# b *0x900 

# ITLBMiss
# b *0xa00 

# RangeException
b *0x100b00 

# SystemCallHandler
# b *0xc00

# FloatingPointException
b *0x100d00 

# TrapException
b *0x100e00 

# System error handler
b msel_panic

