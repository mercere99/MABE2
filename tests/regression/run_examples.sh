BUILD_DIR="../../build" # Relative to THIS_DIR below

THIS_DIR=`pwd`
PROJECTS=`ls ${THIS_DIR}/projects`

cd ${BUILD_DIR}
make clean
make debug
cd ${THIS_DIR}

for NAME in ${PROJECTS}
do
  echo "${NAME}"
  # Setup directory structure
  PROJ_DIR="${THIS_DIR}/projects/${NAME}"
  OUTPUT_DIR="${PROJ_DIR}/output"
  FILE_DIR="${PROJ_DIR}/needed_files"
  EXPECTED_DIR="${PROJ_DIR}/expected"
  mkdir ${OUTPUT_DIR} -p
  cd ${OUTPUT_DIR}
  # Copy over mabe executable and all needed files
  cp ${THIS_DIR}/${BUILD_DIR}/MABE ./
  cp ${FILE_DIR}/* ./
  # Run!
  ./MABE -f ${NAME}.mabe  > terminal_output.txt
  # Remove non-output files
  rm ${OUTPUT_DIR}/MABE
  NEEDED_FILES=`ls ${FILE_DIR}`
  for needed_file in ${NEEDED_FILES}
  do
    rm ${OUTPUT_DIR}/${needed_file}
  done
  # Check generated data against expected data
  EXPECTED_FILES=`ls ${EXPECTED_DIR}`
  for expected_file in ${EXPECTED_FILES}
  do
    if ! [ -e ${OUTPUT_DIR}/${expected_file} ]
    then
      echo "Error! Expected file not found in output! (${expected_file})"
      exit 1
    fi
    cmp ${expected_file}  ${EXPECTED_DIR}/${expected_file}
    TERMINAL_ERROR_CODE=$?
    if ! test ${TERMINAL_ERROR_CODE} -eq 0;
    then
      echo "Error! File generated different terminal output! (${expected_file})"
      head ${OUTPUT_DIR}/${expected_file} -n 200
      exit 1
    fi
  done
  # Reset back to original directory
  cd ${THIS_DIR} 
done
echo "All examples successfully executed!"
# Placeholder comment
