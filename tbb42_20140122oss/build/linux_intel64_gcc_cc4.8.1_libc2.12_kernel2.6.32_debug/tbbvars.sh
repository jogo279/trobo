#!/bin/bash
export TBBROOT="/afs/andrew.cmu.edu/usr7/ericwong/private/trobo/tbb42_20140122oss" #
tbb_bin="/afs/andrew.cmu.edu/usr7/ericwong/private/trobo/tbb42_20140122oss/build/linux_intel64_gcc_cc4.8.1_libc2.12_kernel2.6.32_debug" #
if [ -z "$CPATH" ]; then #
    export CPATH="${TBBROOT}/include" #
else #
    export CPATH="${TBBROOT}/include:$CPATH" #
fi #
if [ -z "$LIBRARY_PATH" ]; then #
    export LIBRARY_PATH="${tbb_bin}" #
else #
    export LIBRARY_PATH="${tbb_bin}:$LIBRARY_PATH" #
fi #
if [ -z "$LD_LIBRARY_PATH" ]; then #
    export LD_LIBRARY_PATH="${tbb_bin}" #
else #
    export LD_LIBRARY_PATH="${tbb_bin}:$LD_LIBRARY_PATH" #
fi #
 #
