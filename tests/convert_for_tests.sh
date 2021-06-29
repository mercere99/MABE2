#!/usr/bin/env bash

EMP_DIR=../source/third-party/empirical
COV_DIR=./coverage_source

set -e # exit with error if any of this fails
rm -rf ${COV_DIR} 
cp -r ../source ${COV_DIR}
for filename in $(find ${COV_DIR} -name "*.hpp" ! -name "_*" ! -path "*/OLD/*" ! -path "*/in_progress/*" ! -path "*/web/*" ! -path "*/third-party/*")
do
    echo $filename
    ${EMP_DIR}/third-party/force-cover/force_cover $filename -- -I. -I${COV_DIR} --language c++ -std=c++17 -DEMP_TRACK_MEM -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/../../../../include/c++/8 -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/../../../../include/x86_64-linux-gnu/c++/8 -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/../../../../include/c++/8/backward -I/usr/include/clang/7/include -I/usr/local/include -I/usr/bin/../lib/gcc/x86_64-linux-gnu/8/include -I/usr/include/x86_64-linux-gnu -I/usr/include -I${EMP_DIR}/include -I${EMP_DIR}/third-party/cereal/include/ | xargs -0 echo > $filename.temp
    #mv $filename.temp $filename

done
