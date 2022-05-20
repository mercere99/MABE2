BUILD_DIR="../../build" # Relative to THIS_DIR below

THIS_DIR=`pwd`
PROJECTS=`ls ${THIS_DIR}/projects`

set -e # If any errors occur, propogate error and stop

cd ${BUILD_DIR}
make clean
make debug
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
  cp ${THIS_DIR}/${BUILD_DIR}/MABE ./
  cp ${FILE_DIR}/* ./
  # Run!
  ./MABE -f ${NAME}.mabe > terminal_output.txt
  # Remove non-output files
  rm ${EXPECTED_DIR}/MABE
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
