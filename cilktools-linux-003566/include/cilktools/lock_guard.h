/* lock_guard.h                  -*-C++-*-
 *
 *************************************************************************
 *                         INTEL CONFIDENTIAL
 *
 * Copyright (C) 2011 Intel Corporation.  All Rights Reserved.
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
 * Lock guard patterned after the std::lock_guard class template proposed in
 * the C++ 0x draft standard.
 *
 * An object of type lock_guard controls the ownership of a mutex object
 * within a scope. A lock_guard object maintains ownership of a mutex object
 * throughout the lock_guard object's lifetime. The behavior of a program is
 * undefined if the mutex referenced by pm does not exist for the entire
 * lifetime of the lock_guard object.
 */

#ifndef LOCK_GUARD_H_INCLUDED
#define LOCK_GUARD_H_INCLUDED

#include <cilk/cilk.h>

namespace cilkscreen
{
    template <class Mutex>
    class lock_guard
    {
    public:
        typedef Mutex mutex_type;

        explicit lock_guard(mutex_type &m) : pm(m)
        {
            pm.lock();
            locked = true;
        }

        ~lock_guard()
        {
            locked = false;
            pm.unlock();
        }

    private:
        lock_guard(lock_guard const&);
        lock_guard& operator=(lock_guard const&);

    private:
        // exposition only:
        mutex_type &pm;
        bool locked;
    };
}

#endif  // LOCK_GUARD_H_INCLUDED
