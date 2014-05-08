/* cilkscreen.h                  -*-C++-*-
 *
 *************************************************************************
 *                         INTEL CONFIDENTIAL
 *
 * Copyright (C) 2010-2011 Intel Corporation.  All Rights Reserved.
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
 **************************************************************************/

#ifndef INCLUDED_CILKSCREEN_H
#define INCLUDED_CILKSCREEN_H

#include <cilk/cilk_api.h>

/*
 * Cilkscreen "functions".  These macros generate metadata in your application
 * to notify Cilkscreen of program state changes
 */

#if ! defined(CILK_STUB) && defined(__INTEL_COMPILER)
#  define __cilkscreen_metacall(annotation,expr) \
    __notify_zc_intrinsic((char *)annotation, expr)
#else
#  define __cilkscreen_metacall(annotation,expr) ((void)annotation, (void)(expr))
#endif

/* Call once when a user thread enters a spawning function */
#define __cilkscreen_enable_instrumentation() \
    __cilkscreen_metacall("cilkscreen_enable_instrumentation", 0)

/* Call once when a user thread exits a spawning function */
#define  __cilkscreen_disable_instrumentation() \
    __cilkscreen_metacall("cilkscreen_disable_instrumentation", 0)

/* Call to temporarily disable cilkscreen instrumentation */
#define __cilkscreen_enable_checking() \
    __cilkscreen_metacall("cilkscreen_enable_checking", 0)

/* Call to re-enable temporarily-disabled cilkscreen instrumentation */
#define __cilkscreen_disable_checking() \
    __cilkscreen_metacall("cilkscreen_disable_checking", 0)

/* Inform cilkscreen that memory from begin to end can be reused without
 * causing races (e.g., for memory that comes from a memory allocator) */
#define __cilkscreen_clean(begin, end)                      \
    do {                                                    \
        void *__data[2] = { (begin), (end) };               \
        __cilkscreen_metacall("cilkscreen_clean", &__data); \
    } while(0)

/* Inform cilkscreen that a lock is being acquired.
 * If the lock type is not a handle, then the caller should take its address
 * and pass the pointer to the lock.  Otherwise, the caller should pass the
 * lock handle directly.
 */
#define __cilkscreen_acquire_lock(lock) \
    __cilkscreen_metacall("cilkscreen_acquire_lock", (lock))

#define __cilkscreen_release_lock(lock) \
    __cilkscreen_metacall("cilkscreen_release_lock", (lock))

/*
 * Metacall data
 *
 * A metacall is a way to pass data to a function implemented by a tool.
 * Metacalls are always instrumented when the tool is loaded.
 */

// Tool code for Cilkscreen
#define METACALL_TOOL_CILKSCREEN 1

// Metacall codes implemented by Cilkscreen
#define CS_METACALL_PUTS 0  // Write string to the Cilkscreen log

#define __cilkscreen_puts(text) \
    __cilkrts_metacall(METACALL_TOOL_CILKSCREEN, CS_METACALL_PUTS, (void *)(const char *)text)

#endif /* defined(INCLUDED_CILKSCREEN_H) */
