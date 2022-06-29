#!/bin/bash
./MABE -f phylo_logic_9.mabe  > terminal_output.txt

for FILENAME in $(ls phylogeny*)
do
  cat ${FILENAME} | sort > ${FILENAME}
done
