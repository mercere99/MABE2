#!/bin/bash

if [ $MABE_IS_REGEN -eq 1 ]
then
  ./MABE_debug -f debug.mabe > terminal_output.txt
  rm terminal_output.txt
else
  ./MABE -f debug.mabe > terminal_output.txt
  rm terminal_output.txt
fi
