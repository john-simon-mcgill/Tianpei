/* *
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 */
/*
 * This function is the main function of fixed complexity sphere decoding algorithm
 * INPUT:
 * sigRec: receive signal vector
 * pH: the prpagation channel matrix
 * Nt: the number of transmitted antennas
 * Nr: the number of received antennas
 * M: modulation scheme
 * SNR: signal to noise ratio
 * psymbolconstellation: the symbol constellation
 * OUTPUT:
 * symOut: the detected transmitted symbol vector
 * Tianpei Chen
 * Email: tianpei.chen@mail.mcgill.ca
 * 2014.8.19
 */
#include"common.h"
#include"cu_complex_operation.cuh"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <math.h>
#include<cublas_v2.h>
#include<cuda_runtime.h>
#include<cuda.h>
#include<cuda_profiler_api.h>
#include<cudaProfiler.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_vector_ulong.h>
#include <gsl/gsl_vector_int.h>
#include <gsl/gsl_vector_complex.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
/*#include <gsl/gsl_matrix.h>*/
#include <gsl/gsl_matrix_complex_float.h>
#include <gsl/gsl_randist.h>
/*#include <gsl/gsl_cblas.h>*/
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_combination.h>
#include <gsl/gsl_eigen.h>
#include<gsl/gsl_math.h>
#include<gsl/gsl_sort.h>
#include<gsl/gsl_sort_vector.h>
void FCSD_detection(cuComplex *sigRec,   //received signal vector
		cuComplex *pH,        //propagation matrix
		int Nt,             //number of transmit antennas
		int Nr,              //number of receive antennas
		int M,               //modulation scheme
		cuComplex *psymbolconstellation, //the symbol constellation
		float SNR,          //signal to noise ratio in decimal
		cuComplex *symOut,    //output symbol vector
		float *durationKernel

		) {

	//FCSD ordering
	int count1, count2, count3;
	int *list;
	list = (int*) malloc(MATRIX_SIZE * sizeof(int));
//	cudaMalloc((void**) &d_list, MATRIX_SIZE*sizeof(int));
	for (count1 = 0; count1 < Nt; count1++) {
		list[count1] = count1 + 1;
	}
	cuComplex *d_pH;    //permuted propagation channel matrix
//	d_pH_permuted=(cuComplex*)calloc(1,Nr*Nt*sizeof(cuComplex));
	cudaMalloc((void**) &d_pH, Nr * Nt * sizeof(cuComplex));
//	cudaMalloc((void**) &d_list, MATRIX_SIZE*sizeof(cuComplex));
	double duration;
	clock_t start, end;
	start = clock();
//	cudaProfilerStart();
//    FCSD_ordering(pH, list, d_pH);
	FCSD_ordering_CPU(pH, list, d_pH);
//    cudaProfilerStop();
	end = clock();
	duration = double(end - start);
//printf("hey the ordering time is:\n");
//printf("%0.4f ", duration);
//printf("the list of GPU is:\n");
//for(count1=0;count1<MATRIX_SIZE;count1++)
//{
//	printf("%d ", list[count1]);
//}
//    for (count1=0;count1<Nt; count1++)
//    {
//    	list_temp=sigRec[list[count1]];
//    	sigRec[Nt-1-count1]=list_temp;
//    }
	//cholesky factorization

	//FCSD decoding
	cublasHandle_t handle;
	cublasStatus_t ret;
	cudaError_t error;
	ret = cublasCreate(&handle);
	cuComplex one, zero, snr;
	one.x = 1;
	one.y = 0;
	zero.x = 0;
	zero.y = 0;
	snr.x = 1 / SNR;
	snr.y = 0;
	cuComplex *I, *pH_temp;
	I = (cuComplex*) calloc(1, Nt * Nt * sizeof(cuComplex));
	for (count1 = 0; count1 < Nt; count1++) {
		I[IDC2D(count1,count1,Nt)] = one;
	}
	pH_temp = (cuComplex*) calloc(1, Nr * Nt * sizeof(cuComplex));
	cuComplex *d_pW, *d_I, *d_sigRec, *d_pW1, *d_symOut_hat, *d_pR;
	error = cudaMalloc((void**) &d_pW, Nt * Nt * sizeof(cuComplex));
//    printf("%s\n",cudaGetErrorString(cudaGetLastError()));
	if (error != cudaSuccess) {
		printf("cudaMalloc d_pW returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}

	error = cudaMalloc((void**) &d_I, Nt * Nt * sizeof(cuComplex));
	if (error != cudaSuccess) {
		printf("cudaMalloc d_I returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}
	error = cudaMalloc((void**) &d_sigRec, Nt * sizeof(cuComplex));
	if (error != cudaSuccess) {
		printf("cudaMalloc d_sigRec returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}
	error = cudaMalloc((void**) &d_pW1, Nt * Nr * sizeof(cuComplex));
	if (error != cudaSuccess) {
		printf("cudaMalloc d_pW1 returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}
	error = cudaMalloc((void**) &d_symOut_hat, Nt * sizeof(cuComplex));
	if (error != cudaSuccess) {
		printf("cudaMalloc d_symOut_hat returned error code %d, line %d\n",
				error, __LINE__);
		exit(EXIT_FAILURE);
	}
	error = cudaMemcpy(d_I, I, Nt * Nt * sizeof(cuComplex),
			cudaMemcpyHostToDevice);
	if (error != cudaSuccess) {
		printf("cudaMemcpy d_pH returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}
	error = cudaMemcpy(d_sigRec, sigRec, Nt * sizeof(cuComplex),
			cudaMemcpyHostToDevice);
	if (error != cudaSuccess) {
		printf("cudaMemcpy d_pH returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}

	error = cudaMalloc((void**) &d_pR, Nt * Nt * sizeof(cuComplex));
	if (error != cudaSuccess) {
		printf("cudaMalloc d_pR returned error code %d, line %d\n", error,
				__LINE__);
		exit(EXIT_FAILURE);
	}
	double duration1, duration2;
	ret = cublasCgemm(handle, CUBLAS_OP_N, CUBLAS_OP_C, Nt, Nt, Nr, &one, d_pH,
			Nt, d_pH, Nt, &zero, d_pR, Nt);
	start = clock();
	gsl_matrix_complex *g_pR = gsl_matrix_complex_calloc(MATRIX_SIZE,
			MATRIX_SIZE);
	cuComplex *c_pR = (cuComplex*) calloc(1,
			MATRIX_SIZE * MATRIX_SIZE * sizeof(cuComplex));
	cudaMemcpy(c_pR, d_pR, MATRIX_SIZE * MATRIX_SIZE * sizeof(cuComplex),
			cudaMemcpyDeviceToHost);
	gsl_complex a;
	for (count1 = 0; count1 < MATRIX_SIZE; count1++) {
		for (count2 = 0; count2 < MATRIX_SIZE; count2++) {
			GSL_SET_COMPLEX(&a, c_pR[IDC2D(count1,count2,MATRIX_SIZE)].x,
					c_pR[IDC2D(count1,count2,MATRIX_SIZE)].y);
			gsl_matrix_complex_set(g_pR, count1, count2, a);
		}
	}
	gsl_linalg_complex_cholesky_decomp(g_pR);
//    	        chol(d_pR);
	end = clock();
	for (count1 = 0; count1 < MATRIX_SIZE; count1++) {
		for (count2 = 0; count2 < MATRIX_SIZE; count2++) {
			c_pR[IDC2D(count1,count2,MATRIX_SIZE)].x = gsl_matrix_complex_get(
					g_pR, count1, count2).dat[0];
			c_pR[IDC2D(count1,count2,MATRIX_SIZE)].y = gsl_matrix_complex_get(
					g_pR, count1, count2).dat[1];
		}
	}
	cudaMemcpy(d_pR, c_pR, MATRIX_SIZE * MATRIX_SIZE * sizeof(cuComplex),
			cudaMemcpyHostToDevice);
	duration1 = double((end - start) / CLOCKS_PER_SEC);
	gsl_matrix_complex_free(g_pR);
	free(c_pR);
//    	           printf("the duration of chol GPU is %0.4f:\n", duration1);
	ret = cublasCgemm(handle, CUBLAS_OP_N, CUBLAS_OP_C, Nt, Nt, Nr, &one, d_pH,
			Nt, d_pH, Nt, &snr, d_I, Nt);
	if (ret != CUBLAS_STATUS_SUCCESS) {
		printf("cublasCgemm returned error code %d, line(%d)\n", ret, __LINE__);
		exit(EXIT_FAILURE);
	}
	start = clock();
	int *x = (int*) calloc(1, sizeof(int));
	*x = 1;
	gsl_matrix_complex *LU = gsl_matrix_complex_calloc(MATRIX_SIZE,
			MATRIX_SIZE);
	gsl_matrix_complex *Pinv = gsl_matrix_complex_calloc(MATRIX_SIZE,
			MATRIX_SIZE);
	cuComplex *Pprod = (cuComplex*) calloc(1,
			MATRIX_SIZE * MATRIX_SIZE * sizeof(cuComplex));
	cudaMemcpy(Pprod, d_I, MATRIX_SIZE * MATRIX_SIZE * sizeof(cuComplex),
			cudaMemcpyDeviceToHost);
	for (count1 = 0; count1 < MATRIX_SIZE; count1++) {
		for (count3 = 0; count3 < MATRIX_SIZE; count3++) {
			GSL_SET_COMPLEX(&a, Pprod[IDC2D(count1,count3,MATRIX_SIZE)].x,
					Pprod[IDC2D(count1,count3,MATRIX_SIZE)].y);
			gsl_matrix_complex_set(LU, count1, count3, a);
		}
	}
	gsl_permutation *p = gsl_permutation_calloc((MATRIX_SIZE));
	gsl_permutation_init(p);
	gsl_linalg_complex_LU_decomp(LU, p, x);
	gsl_linalg_complex_LU_invert(LU, p, Pinv);
	gsl_permutation_free(p);
	for (count1 = 0; count1 < MATRIX_SIZE; count1++) {
		for (count3 = 0; count3 < MATRIX_SIZE; count3++) {
			Pprod[IDC2D(count1,count3,MATRIX_SIZE)].x = gsl_matrix_complex_get(
					Pinv, count1, count3).dat[0];
			Pprod[IDC2D(count1,count3,MATRIX_SIZE)].y = gsl_matrix_complex_get(
					Pinv, count1, count3).dat[1];
		}
	}
	cudaMemcpy(d_pW, Pprod, MATRIX_SIZE * MATRIX_SIZE * sizeof(cuComplex),
			cudaMemcpyHostToDevice);
	gsl_matrix_complex_free(LU);
	gsl_matrix_complex_free(Pinv);
	free(Pprod);
//    MATRIX_INVERSE(d_I,d_pW,Nr,Nt);
	end = clock();
	duration2 = double((end - start) / CLOCKS_PER_SEC);
//    printf("the duration of matrix inverse GPU is %0.4f:\n", duration2);
	ret = cublasCgemm(handle, CUBLAS_OP_C, CUBLAS_OP_N, Nt, Nt, Nr, &one, d_pH,
			Nt, d_pW, Nt, &zero, d_pW1, Nr);
	ret = cublasCgemv(handle, CUBLAS_OP_T, Nt, Nr, &one, d_pW1, Nr, d_sigRec, 1,
			&zero, d_symOut_hat, 1);
	start = clock();
	FCSD_decoding(d_pR, d_symOut_hat, symOut, Nt, Nr, M, list,
			psymbolconstellation);
	end = clock();
	*durationKernel = double((end - start));
	cudaFree(d_pH);
	cudaFree(d_pW);
	cudaFree(d_I);
	cudaFree(d_sigRec);
	cudaFree(d_pW1);
	cudaFree(d_symOut_hat);
	cudaFree(d_pR);
	free(I);
	free(pH_temp);
	free(list);

	free(x);

}
