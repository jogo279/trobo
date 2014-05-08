/*
 * simple-race.cilk
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
 * This file implements a simple Intel Cilk Plus program with a known race
 * condition to demonstrate the Cilkscreen race detector.
 */

#include <stdlib.h>
#include <stdio.h>
#include <cilk/cilk.h>


int x;
void race(void)
{
     x = 0;
}

void race1(void)
{
     x = 1;
}

void test(void)
{
     cilk_spawn race();
     cilk_spawn race1();
}

int main(int argc, char *argv[])
{
     test();

     printf("done: x = %d\n", x);
     return 0;
}
