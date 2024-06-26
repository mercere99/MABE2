#!/bin/tcsh
/Users/charles/Dropbox/Development/Empirical/demos/utils/data/summarize-change 1 age-???/max_org.csv > age-summary.csv
/Users/charles/Dropbox/Development/Empirical/demos/utils/data/summarize-change 1 control-???/max_org.csv > control-summary.csv
/Users/charles/Dropbox/Development/Empirical/demos/utils/data/summarize-change 1 -a age-???/max_org.csv > age-full.csv
/Users/charles/Dropbox/Development/Empirical/demos/utils/data/summarize-change 1 -a control-???/max_org.csv > control-full.csv
/Users/charles/Dropbox/Development/Empirical/demos/utils/data/summarize-change 7 -a age-???/max_org.csv > age-births.csv
/Users/charles/Dropbox/Development/Empirical/demos/utils/data/summarize-change 7 -a control-???/max_org.csv > control-births.csv
