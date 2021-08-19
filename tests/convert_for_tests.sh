#!/usr/bin/env bash

EMP_DIR=../source/third-party/empirical
COV_DIR=./coverage_source
TEST_DIR=./unit

FLAGS="-std=c++17 -pthread -Wall -Wno-unused-function -Wno-unused-private-field -I${EMP_DIR}/include/ -I${EMP_DIR}/third-party/Catch/single_include/catch2 -I../source/ -DEMP_TRACK_MEM"

mkdir ${COV_DIR} -p

set -e # exit with error if any of this fails
rm -rf ${COV_DIR} 
cp -r ${TEST_DIR} ${COV_DIR}
for filename in $(find ${COV_DIR} -name "*.cpp" ! -name "_*" ! -path "*/OLD/*" ! -path "*/in_progress/*" ! -path "*/web/*" ! -path "*/third-party/*")
do
    echo "${filename}"
    ${EMP_DIR}/third-party/force-cover/force_cover ${filename} -- -I. -I${COV_DIR} --language c++ ${FLAGS} | xargs -0 echo > ${filename}.temp
    mv ${filename}.temp ${filename}

done
