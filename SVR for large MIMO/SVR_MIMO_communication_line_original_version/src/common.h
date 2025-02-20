//============================================================================
// Name        : common.h
// Author      : Tianpei Chen
// Version     :
// Copyright   : Your copyright notice
// Description : Global head file including basic settings
//============================================================================

#ifndef COMMON_H_
#define COMMON_H_
#include <gsl/gsl_vector.h>
#include<gsl/gsl_matrix.h>
#include<gsl/gsl_vector_complex.h>
#include <gsl/gsl_vector_ulong.h>
//#include<cuComplex.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <time.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#define MATRIX_SIZE 16
#define Nt 8
#define Nr 8
#define IDC2D(i,j,row)    (i)*row+j
#define Constellationsize 4
#define  C  1e-3;      //Penalize weight for noise
#define epsilon 1e-5; //Training precision
#define tol 1e-4   //the tolerance for KKT condition
//void FCSD_detection(
//		cuComplex *sigRec,   //received signal vector
//		cuComplex *pH,        //propagation matrix
//		int Nt,             //number of transmit antennas
//		int Nr,              //number of receive antennas
//		int M,               //modulation scheme
//		cuComplex *psymbolconstellation, //the symbol constellation
//		float SNR,          //signal to noise ratio
//		cuComplex *symOut,    //output symbol vector
//		float *durationKernel
//		);
//void FCSD_decoding(
//		cuComplex *d_R,  //upper triangular matrix after cholesky factorization store in device side
////		cuComplex *s_sub, //the sub brute force rho vector matrix
//		cuComplex *s_hat,  //unconstrained estimation of transmitted symbol vector s
//		cuComplex *s_kernel,  //quantization of estimation ,decoding results
////		cuComplex *Eu,  //Euclidean distance
//		int Nt,    //the number of transmit antennas
//		int Nr,    //the number of receive antennas
//		int M,    //modulation scheme
//		int *list,   //the permutation list
//		cuComplex *psymbolconstellation //the symbol constellation
//		);
void fullfact(
		int rho,  //the number of elements that use full expansion
		int M,    //constellation size
		int *s_sub_index    //the indexes of the full expansion matrix  (pow(M,rho))
		);
void comb(int m, int k, int row, int * aaa, int *subset, int *count3);
//void MATRIX_INVERSE(
//	cuComplex *H,  //input square matrix
//	cuComplex *R,   //the inversion of the matrix H
//	int Nr,  //number of receive antennas
//	int Nt   //number of transmit antennas
//);
//void FCSD_ordering(
//		cuComplex *pH,
//		int *list,
//		cuComplex *pH_permuted
//);
//void chol(cuComplex *U
//);
void FCSD_CPU(
		gsl_vector_complex *preceived,
		gsl_matrix_complex *pH,
				int Nt,
				int Nr,              //number of receive antennas
				int M,               //modulation scheme
		gsl_vector_complex *psymbolconstellation, //the symbol constellation
				float SNR,
		gsl_vector_complex *symOut,
		float *durationKernel
);
void SVR_DETECTOR(
		const gsl_vector_complex *preceived,
		const gsl_matrix_complex *pH,
		float SNRb,
		gsl_vector_complex *psymout,
		int start_M,
		int select_M
		);
int Initialization(gsl_vector *alpha,//Lagrange multiplier
		gsl_vector *beta,//Lagrange multiplier
		const gsl_vector_complex *preceived,//received symbol vector(output of trainning data set)
		gsl_vector_complex *phi, // update parameter
		double S_C_real, //real stopping parameter
		double S_C_imag,  //imaginary stopping parameter
		int method //label which start strategy is used
		);
void 	WSS2_1Dsolver(gsl_vector *l_m,  //Lagrange Multiplier vector
        gsl_vector_complex *phi,  //update parameter
        gsl_matrix *R_Kernel,    //real kernel matrix
        int F_i,  //index of first maximum Lagrange multiplier
        int S_i, //index of second maximum Lagrange multiplier
		int model //determine this routine is for real part (0) or imaginary part (1)
);
void outterloop(gsl_vector_complex *alpha,
		gsl_vector_complex *eta,
		gsl_vector_complex *Error,
		gsl_matrix_complex *pH,
		gsl_vector_complex psymout
		);   //the outter loop to choose proper alpha

int TestData(gsl_vector_complex *alpha,
		gsl_vector_complex *Error,
		gsl_vector_complex *eta);    // the subroutine to determine whether alpha or alpha* is updated

int takestep(gsl_vector_complex *alpha,
		gsl_vector_complex *Error,
		gsl_vector_complex *eta);   //the subroutine to update chosen alpha or alpha*
Nalpha_Node checkNonbound(gsl_vector_complex *alpha);

//void FCSD_ordering_CPU(
//		cuComplex *pH,
//		int *list,
//		cuComplex *pH_permuted
//		);
//
//
//__global__ void queue_max(
//		cuComplex *P,
//		int *j,
//		int index
//		);
//__global__ void queue_min(
//		cuComplex *P,
//		int *j,
//		int index
//		);
//__global__ void MATRIX_ROWCOLUMNT_kernel(cuComplex *Hr,
//		cuComplex *Hc, int row, int column);
//__global__ void MATRIX_COLUMNROWT_kernel(cuComplex *Hc,
//		cuComplex *Hr, int row, int column);
#endif /* COMMON_H_ */
