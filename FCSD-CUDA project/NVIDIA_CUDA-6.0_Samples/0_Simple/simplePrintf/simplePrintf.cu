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


// System includes
#include <stdio.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>

// helper functions and utilities to work with CUDA
#include <helper_functions.h>
#include <helper_cuda.h>

#include "cuPrintf.cu"

#ifndef MAX
#define MAX(a,b) (a > b ? a : b)
#endif

//The macro CUPRINTF is defined for architectures
//with different compute capabilities.
#if __CUDA_ARCH__ < 200     //Compute capability 1.x architectures
#define CUPRINTF cuPrintf
#else                       //Compute capability 2.x architectures
#define CUPRINTF(fmt, ...) printf("[%d, %d]:\t" fmt, \
                                  blockIdx.y*gridDim.x+blockIdx.x,\
                                  threadIdx.z*blockDim.x*blockDim.y+threadIdx.y*blockDim.x+threadIdx.x,\
                                  __VA_ARGS__)
#endif


__global__ void testKernel(int val)
{
    CUPRINTF("\tValue is:%d\n", val);
}

int main(int argc, char **argv)
{
    int devID;
    cudaDeviceProp props;

    // This will pick the best possible CUDA capable device
    devID = findCudaDevice(argc, (const char **)argv);

    //Get GPU information
    checkCudaErrors(cudaGetDevice(&devID));
    checkCudaErrors(cudaGetDeviceProperties(&props, devID));
    printf("Device %d: \"%s\" with Compute %d.%d capability\n",
           devID, props.name, props.major, props.minor);

    //Architectures with compute capability 1.x, function
    //cuPrintf() is used. Otherwise, function printf() is called.
    bool use_cuPrintf = (props.major < 2);

    if (use_cuPrintf)
    {
        //Initializaton, allocate buffers on both host
        //and device for data to be printed.
        cudaPrintfInit();

        printf("cuPrintf() is called. Output:\n\n");
    }
    //Architecture with compute capability 2.x, function
    //printf() is called.
    else
    {
        printf("printf() is called. Output:\n\n");
    }

    //Kernel configuration, where a two-dimensional grid and
    //three-dimensional blocks are configured.
    dim3 dimGrid(2, 2);
    dim3 dimBlock(2, 2, 2);
    testKernel<<<dimGrid, dimBlock>>>(10);
    cudaDeviceSynchronize();

    if (use_cuPrintf)
    {
        //Dump current contents of output buffer to standard
        //output, and origin (block id and thread id) of each line
        //of output is enabled(true).
        cudaPrintfDisplay(stdout, true);

        //Free allocated buffers by cudaPrintfInit().
        cudaPrintfEnd();
    }

    // cudaDeviceReset causes the driver to clean up all state. While
    // not mandatory in normal operation, it is good practice.  It is also
    // needed to ensure correct operation when the application is being
    // profiled. Calling cudaDeviceReset causes all profile data to be
    // flushed before the application exits
    cudaDeviceReset();

    return EXIT_SUCCESS;
}

