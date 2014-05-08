// -*- C++ -*-

/*
 * qsort-race.cilk
 *
 * Copyright (C) 2009-2011 Intel Corporation. All Rights Reserved.
 *
 * The source code contained or described herein and all
 * documents related to the source code ("Material") are owned by 
 * Intel Corporation or its suppliers or licensors. Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors. The Material is protected by worldwide copyright
 * laws and treaty provisions.  No part of the Material may be
 * used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed,  or disclosed in any way
 * except as expressly provided in the license provided with the
 * Materials.  No license under any patent, copyright, trade
 * secret or other intellectual property right is granted to or
 * conferred upon you by disclosure or delivery of the Materials,
 * either expressly, by implication, inducement, estoppel or
 * otherwise, except as expressly provided in the license
 * provided with the Materials. 
 *
 * This file implements a quicksort with a known race condition to demonstrate
 * the Cilkscreen race detector.
 */

#include <algorithm>
#include <iostream>
#include <iterator>
#include <functional>

#include <math.h>
#include <cilk/cilk.h>

using namespace std;

// Sort the range between bidiriectional iterators begin and end.  end is one
// past the final element in the range.
//
// Use the Quick Sort algorithm, implemented using recursive divide and
// conquer.  This function is NOT the same as the Standard C Library qsort()
// function.  This implementation uses Cilk++, and has a race condition that
// Cilkscreen will detect.

template<typename T> void qsort(T begin, T end)
{
    if (begin != end) {
        T middle = partition(begin, end,
                       bind2nd(less<typename iterator_traits<T>::value_type>(),
                           *begin)); 

        cilk_spawn qsort(begin, middle);
        // INTENTIONAL RACE: MIDDLE-1 should be MIDDLE
        qsort(max(begin + 1, middle - 1), end);
        cilk_sync;
    }
}

#define TESTSIZE 1024

// A simple test harness 
int qmain(int n)
{
    int* a = new int[n];

    // Fill the array with "random" data
    cout << "in:  ";
    for (int i = 0; i < n; ++i) {
        a[i] = (int) (100 * sin((float) i));
        cout << a[i] << " ";
    }
    cout << "\n";

    // Do the sort

    qsort(a, a + n);

    // Display the result
    cout << "out: ";
    copy(a, a + n, ostream_iterator<int>(cout, " "));
    cout << endl;
    return 42;
}


int main(int argc, char* argv[])
{
    int n = 10;

    if (argc > 1) {
        n = atoi(argv[1]);
    }

    return qmain(n);
}
