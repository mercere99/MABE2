MABE_BUILD_DIR="../../build" # Relative to THIS_DIR below

THIS_DIR=`pwd`
MABE_BUILD_DIR=${THIS_DIR}/${MABE_BUILD_DIR}
LOCAL_BUILD_DIR=${THIS_DIR}/build
PROJECTS=`ls ${THIS_DIR}/projects`

set -e # If any errors occur, propogate error and stop

### Compile
mkdir -p ${LOCAL_BUILD_DIR}
cd ${MABE_BUILD_DIR}
## Debug mode
make clean
make debug 
# Move executable
mv ${MABE_BUILD_DIR}/MABE ${LOCAL_BUILD_DIR}/MABE_debug
## Release mode
make clean
make 
# Move executable
mv ${MABE_BUILD_DIR}/MABE ${LOCAL_BUILD_DIR}/MABE
cd ${THIS_DIR}

for NAME in ${PROJECTS}
do
  echo "Starting project: ${NAME}"
  # Setup directory structure
  PROJ_DIR="${THIS_DIR}/projects/${NAME}"
  OUTPUT_DIR="${PROJ_DIR}/output"
  FILE_DIR="${PROJ_DIR}/needed_files"
  EXPECTED_DIR="${PROJ_DIR}/expected"
  mkdir ${OUTPUT_DIR} -p
  mkdir ${EXPECTED_DIR} -p
  mkdir ${FILE_DIR} -p
  cd ${EXPECTED_DIR}
  # Copy over mabe executable and all needed files
  cp ${LOCAL_BUILD_DIR}/MABE ./
  cp ${LOCAL_BUILD_DIR}/MABE_debug ./
  cp ${FILE_DIR}/* ./
  # Run!
  env MABE_IS_REGEN=1 ./run_regression_test.sh
  TERMINAL_ERROR_CODE=$?
  if ! test ${TERMINAL_ERROR_CODE} -eq 0;
  then
    echo "Error encountered while running ${OUTPUT_DIR}/run_regression_test.sh"
    exit 1
  fi
  # Remove non-output files
  rm ${EXPECTED_DIR}/MABE
  rm ${EXPECTED_DIR}/MABE_debug
  NEEDED_FILES=`ls ${FILE_DIR}`
  for needed_file in ${NEEDED_FILES}
  do
    rm ${EXPECTED_DIR}/${needed_file}
  done
  # Return to starting directory
  cd ${THIS_DIR} 
done
echo "All expected data regenerated!"
echo "Latest commit at time of last regeneration:" > last_test_regen_info.txt
git log | head -n 5 >> last_test_regen_info.txt
echo " " >> last_test_regen_info.txt
echo " " >> last_test_regen_info.txt
echo "Time of last regeneration:" >> last_test_regen_info.txt
date >> last_test_regen_info.txt
echo "This regeneration was logged in file: last_test_regen_info.txt"
