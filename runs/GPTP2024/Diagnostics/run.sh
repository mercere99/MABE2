foreach i ( 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 )
  mkdir age-1$i
  cd age-1$i
  /Users/charles/Dropbox/Development/MABE2/build/MABE -f ../AgeDiagnostics.mabe
  cd ..
end
  
