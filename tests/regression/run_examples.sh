BUILD_DIR="../../build"
SETTINGS_DIR="."

MABE_FILES=`ls ${SETTINGS_DIR} | grep .mabe`
THIS_DIR=`pwd`

set -e # If any errors occur, propogate error and stop

cd ${BUILD_DIR}
#make clean
#make debug
cd ${THIS_DIR}

for filename in ${MABE_FILES}
do
  NAME=`echo ${filename} | grep -P "^[\w|_][\w|\d_]+" -o`
  echo "${NAME}"
  DIR="output/${NAME}"
  mkdir ${DIR} -p
  cd ${DIR}
  cp ../../${BUILD_DIR}/MABE ./
  cp ../../${SETTINGS_DIR}/${filename} ./
  ./MABE -f ${filename} > terminal_output.txt

  cd ${THIS_DIR} 
done
echo "All examples successfully executed!"
