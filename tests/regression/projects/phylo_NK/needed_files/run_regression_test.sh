#!/bin/bash

./MABE_debug -f phylo_NK.mabe > terminal_output.txt

for FILENAME in $(ls phylogeny*)
do
  cat ${FILENAME} | sort > ${FILENAME}
done
