/*
 * Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef __REDUCTION_H__
#define __REDUCTION_H__

template <class T>
void reduce_sm10(int size, int threads, int blocks,
                 int whichKernel, T *d_idata, T *d_odata);

template <class T>
void reduce_sm13(int size, int threads, int blocks,
                 int whichKernel, T *d_idata, T *d_odata);

cudaError_t setRetirementCount(int retCnt);

#endif
