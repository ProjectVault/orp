# You sourced the or1k-devenv-settings.sh vars, ya?
or1k-elf-sim -V --srv=60000 -f sim.cfg $1 &
SIM=$!

nc 127.0.0.1 60001 &
LOG=$1

trap "kill $SIM $LOG" SIGINT

wait $SIM $LOG
 

