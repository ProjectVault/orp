#!/bin/bash 

. common/build_vars

#################
# Status handling

# initialize these here for later cleanup
QEMUPID=0
GDBPID=0

FIFO=$(mktemp)
QEMUIO_INP=${FIFO}.in
QEMUIO_OUT=${FIFO}.out

EXIT_PASS=0
EXIT_SKIP=77
EXIT_ERROR=99
EXIT_FAIL=1

function cleanup()
{
	test "$QEMUPID" -ne 0 && kill $QEMUPID
	test "$GDBPID" -ne 0 && kill -9 $GDBPID
	exec 3>&-
	rm -rf $QEMUIO
	rm -rf $QEMUIO_INP
	rm -rf $QEMUIO_OUT
}

function pass() {
	cleanup
	exit $EXIT_PASS
}

function skip() {
	cleanup
	exit $EXIT_SKIP
}

function error() {
	cleanup
	exit $EXIT_ERROR
}

function fail() {
	cleanup
	exit $EXIT_FAIL
}

trap fail SIGINT SIGSEGV SIGKILL
trap cleanup exit

#######################
# Locating Needed Tools

function find_qemu() {
	locs="${PATH}:/home/sean/lol/bin"
	name="$ARCH_QEMU_BIN"
	#TODO: Obviously this is not the best way...
	saved_IFS=$IFS
	IFS=":"
	for dir in $locs
	do
		if [ -x "${dir}/${name}" ]
		then
			echo "${dir}/${name}"
			break
		fi
	done
	IFS=$saved_IFS
}

function find_gdb() {
	locs="${PATH}:/home/sean/lol/bin"
	name="$ARCH_GDB_BIN"
	#TODO: Obviously this is not the best way...
	saved_IFS=$IFS
	IFS=":"
	for dir in $locs
	do
		if [ -x "${dir}/${name}" ]
		then
			echo "${dir}/${name}"
			break
		fi
	done
	IFS=$saved_IFS
}

########################
# Setup the test 

# argv[1] is the test binary
TESTBIN=$1
# TODO: choose unique and unused TCP port so parallel works reliably 
TCPPORT=60000

rm -f $QEMUIO_OUT
mkfifo $QEMUIO_OUT

###############
# Start up a VM
QEMU=$(find_qemu)
$QEMU $ARCH_QEMU_FLAGS -S -gdb tcp::$TCPPORT -nographic -kernel $TESTBIN >$QEMUIO_OUT 2>&1 &
QEMUPID=$!



###########################
# Startup a controlling GDB
GDB=$(find_gdb)
$GDB --batch --nh -ex "file $TESTBIN" -ex "target remote :$TCPPORT" \
	 -ex "continue" &

# Generic breakpoints for or1k
#	 -ex "b msel_panic" -ex "b *0x300" -ex "b *0x400" -ex "b *0x600" \
#	 -ex "b *0x700" -ex "b *0xb00" -ex "b *0xd00" -ex "b *0xe00" \

GDBPID=$!

###################################################
# Source the test script so it runs in this context
sleep 1

/usr/bin/expect -d -f ${TESTBIN}.expect <$QEMUIO_OUT

if [ $? -eq 0 ]
then
	pass
else
	fail
fi






