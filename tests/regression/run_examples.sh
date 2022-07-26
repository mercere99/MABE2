MABE_BUILD_DIR="../../build" # Relative to THIS_DIR below

THIS_DIR=`pwd`
MABE_BUILD_DIR="${THIS_DIR}/${MABE_BUILD_DIR}"
LOCAL_BUILD_DIR="${THIS_DIR}/build"
PROJECTS=`ls "${THIS_DIR}/projects"`

### Compile
mkdir -p "${LOCAL_BUILD_DIR}"
cd "${MABE_BUILD_DIR}"
## Debug mode
make clean
make debug 
# Make sure it compiled
TERMINAL_ERROR_CODE=$?
if ! test ${TERMINAL_ERROR_CODE} -eq 0;
then
  echo "Error! Could not compile MABE in debug mode! (${expected_file})"
  exit 1
fi
# Move executable
mv "${MABE_BUILD_DIR}/MABE" "${LOCAL_BUILD_DIR}/MABE_debug"
## Release mode
make clean
make 
# Make sure it compiled
TERMINAL_ERROR_CODE=$?
if ! test ${TERMINAL_ERROR_CODE} -eq 0;
then
  echo "Error! Could not compile MABE in release mode! (${expected_file})"
  exit 1
fi
# Move executable
mv "${MABE_BUILD_DIR}/MABE" "${LOCAL_BUILD_DIR}/MABE"
cd "${THIS_DIR}"

for NAME in ${PROJECTS}
do
  echo "${NAME}"
  # Setup directory structure
  PROJ_DIR="${THIS_DIR}/projects/${NAME}"
  OUTPUT_DIR="${PROJ_DIR}/output"
  FILE_DIR="${PROJ_DIR}/needed_files"
  EXPECTED_DIR="${PROJ_DIR}/expected"
  mkdir "${OUTPUT_DIR}" -p
  cd "${OUTPUT_DIR}"
  # Copy over mabe executable and all needed files
  cp "${LOCAL_BUILD_DIR}/MABE" ./
  cp "${LOCAL_BUILD_DIR}/MABE_debug" ./
  cp "${FILE_DIR}"/* ./
  # Run!
  env MABE_IS_REGEN=0 ./run_regression_test.sh
  TERMINAL_ERROR_CODE=$?
  if ! test ${TERMINAL_ERROR_CODE} -eq 0;
  then
    echo "Error encountered while running ${OUTPUT_DIR}/run_regression_test.sh"
    exit 1
  fi
  # Remove non-output files
  rm "${OUTPUT_DIR}/MABE"
  rm "${OUTPUT_DIR}/MABE_debug"
  NEEDED_FILES=`ls "${FILE_DIR}"`
  for needed_file in ${NEEDED_FILES}
  do
    rm "${OUTPUT_DIR}/${needed_file}"
  done
  # Check generated data against expected data
  EXPECTED_FILES=`ls "${EXPECTED_DIR}"`
  for expected_file in ${EXPECTED_FILES}
  do
    if ! [ -e "${OUTPUT_DIR}/${expected_file}" ]
    then
      echo "Error! Expected file not found in output! (${expected_file})"
      exit 1
    fi
    cmp ${expected_file}  "${EXPECTED_DIR}/${expected_file}"
    TERMINAL_ERROR_CODE=$?
    if ! test ${TERMINAL_ERROR_CODE} -eq 0;
    then
      echo "Error! File generated different terminal output! (${expected_file})"
      echo "Result of \"diff\" command (left is output, right is expected):"
      diff "${expected_file}" "${EXPECTED_DIR}/${expected_file}"
      exit 1
    fi
  done
  # Reset back to original directory
  cd "${THIS_DIR}" 
done
echo "All examples successfully executed!"
# Placeholder comment
