
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR/build
cmake -DBUILD_SHARED_LIBS=ON ${SCRIPT_DIR} ..
cmake --build ${SCRIPT_DIR}/build
${SCRIPT_DIR}/build/rChart