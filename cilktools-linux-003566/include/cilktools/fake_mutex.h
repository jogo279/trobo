/* fake_mutex.h                  -*-C++-*-
 *
 *************************************************************************
 *                         INTEL CONFIDENTIAL
 *
 * Copyright (C) 2013 Intel Corporation.  All Rights Reserved.
 *
 *  The source code contained or described herein and all documents related
 *  to the source code ("Material") are owned by Intel Corporation or its
 *  suppliers or licensors.  Title to the Material remains with Intel
 *  Corporation or its suppliers and licensors.  The Material is protected
 *  by worldwide copyright laws and treaty provisions.  No part of the
 *  Material may be used, copied, reproduced, modified, published, uploaded,
 *  posted, transmitted, distributed, or disclosed in any way without
 *  Intel's prior express written permission.
 *
 *  No license under any patent, copyright, trade secret or other
 *  intellectual property right is granted to or conferred upon you by
 *  disclosure or delivery of the Materials, either expressly, by
 *  implication, inducement, estoppel or otherwise.  Any license under such
 *  intellectual property rights must be express and approved by Intel in
 *  writing.
 *
 **************************************************************************
 *
 * Cilkscreen fake mutexes are provided to indicate to the Cilkscreen race
 * detector that a race should be ignored.
 *
 * NOTE: This class does not provide mutual exclusion.  You should use the
 * mutual exclusion constructs provided by TBB or your operating system to
 * protect against real data races.
 */

#ifndef FAKE_MUTEX_H_INCLUDED
#define FAKE_MUTEX_H_INCLUDED

#include <cilktools/cilkscreen.h>

namespace cilkscreen
{
    class fake_mutex
    {
    public:
	fake_mutex() : locked(false)
	{
	}

	~fake_mutex()
	{
	    __CILKRTS_ASSERT(! locked);
	}

        // Wait until mutex is available, then enter
        void lock()
        {
            __cilkscreen_acquire_lock(&locked);
	    __CILKRTS_ASSERT(! locked);
	    locked = true;
        }

        // A fake mutex is always available
        bool try_lock() { lock(); return true; }

        // Releases the mutex
        void unlock()
        {
	    __CILKRTS_ASSERT(locked);
	    locked = false;
            __cilkscreen_release_lock(&locked);
        }

    private:
        bool locked;
    };

} // namespace cilk

#endif  // FAKE_MUTEX_H_INCLUDED
