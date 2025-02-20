/*
 * INPUT:
 * symOut: the output complex symbol
 * psymbolconstellation: the symbol constellation
 * pgrayindexes: the corresponding gray code
 * OUTPUT:
 * poutput: the demodulated gray code
 */
#include"common.h"
void demodulator_CPU(gsl_vector_complex *symOut, gsl_vector_complex *psymbolconstellation,
		gsl_vector_ulong *pgrayindexes, gsl_vector_ulong *poutput) {
	int count1, count2;
	float epsilon=1e-5;
	int M = psymbolconstellation->size;
	int Nt = poutput->size;
	for (count1 = 0; count1 < Nt; count1++) {
		for (count2 = 0; count2 < M; count2++) {
			if (sqrt(pow(gsl_vector_complex_get(symOut,count1).dat[0]
					-gsl_vector_complex_get(psymbolconstellation, count2).dat[0],2))<epsilon
					&& sqrt(pow(gsl_vector_complex_get(symOut,count1).dat[1]
							-gsl_vector_complex_get(psymbolconstellation,
									count2).dat[1],2))<epsilon) {
				gsl_vector_ulong_set(poutput, count1,
						gsl_vector_ulong_get(pgrayindexes, count2));
				break;
			}
		}
	}
}
//void demodulator_GPU(cuComplex *symOut, gsl_vector_complex *psymbolconstellation,
//		gsl_vector_ulong *pgrayindexes, gsl_vector_ulong *poutput) {
//	int count1, count2;
//	int M = psymbolconstellation->size;
//	int Nt = poutput->size;
//	for (count1 = 0; count1 < Nt; count1++) {
//		for (count2 = 0; count2 < M; count2++) {
//			if (symOut[count1].x
//					== float(gsl_vector_complex_get(psymbolconstellation, count2).dat[0])
//					&& symOut[count1].y
//							== float(gsl_vector_complex_get(psymbolconstellation,
//									count2).dat[1])) {
//				gsl_vector_ulong_set(poutput, count1,
//						gsl_vector_ulong_get(pgrayindexes, count2));
//				break;
//			}
//		}
//	}
//}
