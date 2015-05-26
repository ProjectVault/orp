
# Find abs path to this directory
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# Place the tools into search paths 
SOCTOOLS=${DIR}/slash
PATH=${SOCTOOLS}/bin:${PATH}
LD_LIBRARY_PATH=${SOCTOOLS}/lib/:${LD_LIBRARY_PATH}
export PATH LD_LIBRARY_PATH
export SOCTOOLS
