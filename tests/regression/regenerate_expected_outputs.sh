BUILD_DIR="../../build"
SETTINGS_DIR="."

MABE_FILES=`ls ${SETTINGS_DIR} | grep .mabe`
THIS_DIR=`pwd`

set -e # If any errors occur, propogate error and stop

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

  # Copy output files back to this directory
  cp ./terminal_output.txt ${THIS_DIR}/expected_terminal_output_${NAME}.txt
  cp ./output.csv ${THIS_DIR}/expected_csv_output_${NAME}.csv

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
