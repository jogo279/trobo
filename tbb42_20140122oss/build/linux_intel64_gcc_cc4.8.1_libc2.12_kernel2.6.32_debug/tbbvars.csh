#!/bin/csh
setenv TBBROOT "/afs/andrew.cmu.edu/usr7/ericwong/private/trobo/tbb42_20140122oss" #
setenv tbb_bin "/afs/andrew.cmu.edu/usr7/ericwong/private/trobo/tbb42_20140122oss/build/linux_intel64_gcc_cc4.8.1_libc2.12_kernel2.6.32_debug" #
if (! $?CPATH) then #
    setenv CPATH "${TBBROOT}/include" #
else #
    setenv CPATH "${TBBROOT}/include:$CPATH" #
endif #
if (! $?LIBRARY_PATH) then #
    setenv LIBRARY_PATH "${tbb_bin}" #
else #
    setenv LIBRARY_PATH "${tbb_bin}:$LIBRARY_PATH" #
endif #
if (! $?LD_LIBRARY_PATH) then #
    setenv LD_LIBRARY_PATH "${tbb_bin}" #
else #
    setenv LD_LIBRARY_PATH "${tbb_bin}:$LD_LIBRARY_PATH" #
endif #
 #
