BUILD_DIR="../../build"
SETTINGS_DIR="."

MABE_FILES=`ls ${SETTINGS_DIR} | grep .mabe`
THIS_DIR=`pwd`

cd ${BUILD_DIR}
make clean
make debug
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

  # Check generated data against expected data
  cmp terminal_output.txt ${THIS_DIR}/expected_terminal_output_${NAME}.txt 
  TERMINAL_ERROR_CODE=$?
  if ! test ${TERMINAL_ERROR_CODE} -eq 0;
  then
    echo "Error! File generated different terminal output!"
    exit 1
  fi

  cmp output.csv ${THIS_DIR}/expected_csv_output_${NAME}.csv
  CSV_ERROR_CODE=$?
  if ! test ${CSV_ERROR_CODE} -eq 0;
  then
    echo "Error! File generated different values for output.csv"
    exit 2
  fi
  # Reset back to original directory
  cd ${THIS_DIR} 
done
echo "All examples successfully executed!"
