
# Find abs path to this directory
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# Place the tools into search paths 
OR1KTOOLS=${DIR}/slash
PATH=${OR1KTOOLS}/bin:${PATH}
LD_LIBRARY_PATH=${OR1KTOOLS}/x86_64-unknown-linux-gnu/or1k-elf/lib/:${LD_LIBRARY_PATH}
LD_LIBRARY_PATH=${OR1KTOOLS}/lib/:${LD_LIBRARY_PATH}
LD_LIBRARY_PATH=${ORP}/lib/:${LD_LIBRARY_PATH}
export PATH LD_LIBRARY_PATH
export OR1KTOOLS
OR1KORP=${DIR}/orp
export OR1KORP
ORP=${OR1KORP}
export ORP
