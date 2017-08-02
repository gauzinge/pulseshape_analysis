#!/bin/bash
source /afs/cern.ch/sw/lcg/releases/gcc/4.9.2/x86_64-slc6/setup.sh	
#export PATH=/afs/cern.ch/sw/lcg/contrib/gdb/7.8/x86_64-slc6-gcc48-opt/bin:$PATH
#source /afs/cern.ch/sw/lcg/app/releases/ROOT/6.04.02/x86_64-slc6-gcc49-opt/root/bin/thisroot.sh
export BASEDIR='pwd'
echo $BASEDIR
/afs/cern.ch/user/g/gauzinge/pulseshape_analysis/tester_batch.exe TECP DECO
echo 'Done'
