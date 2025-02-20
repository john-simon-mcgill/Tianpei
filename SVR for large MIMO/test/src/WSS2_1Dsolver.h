/*
 * WSS2_1Dsolver.h
 *
 *  Created on: Aug 4, 2015
 *      Author: tianpei.chen@mail.mcgill.ca
 *      This is 2D selection subroutine based on maximum 2 1D direction
 *      subroutine, choose the two Lagrange multipliers with maximum 1D gain
 *      to perform 2D solver.
 *      INPUT: alpha(beta), alpha_hat(beta_hat), label1, label2, phi, R_Kernel
 *      OUTPUT: First, Second
 */
#include "public.h"
#ifndef WSS2_1DSOLVER_H_
#define WSS2_1DSOLVER_H_
void 	WSS2_1Dsolver(gsl_vector *l_m,  //Lagrange Multiplier vector
        gsl_vector_complex *phi,  //update parameter
        gsl_matrix *R_Kernel,    //real kernel matrix
        int F_i,  //index of first maximum Lagrange multiplier
        int S_i, //index of second maximum Lagrange multiplier
		int model //determine this routine is for real part (0) or imaginary part (1)
){
double best_g, g;
double sigma, sigma_hat;
double temp_l_m, temp_l_m_hat;
int count;
//double phi_p[2*Nr];
//for(count=0;count<2*Nr;count++){  //take real and imaginary part of phi
//	if(count<Nr){
//  phi_p[count]=gsl_vector_complex_get(phi,count).dat[0];
//	}else{
//		phi_p[count]=gsl_vector_complex_get(phi,count-Nr).dat[1];
//	}
//}
best_g=-1;
F_i=S_i=-1;
sigma=0;
sigma_hat=0;
for(count=0;count<2*Nr;count++){
	if(count<Nr){
		temp_l_m=gsl_vector_get(l_m, count)+(gsl_vector_complex_get(phi,count).dat[model]-epsilon)/(gsl_matrix_get(R_Kernel,count,count));
		if(temp_l_m<0){
			temp_l_m=0;
		}else if (temp_l_m>C){
			temp_l_m=C;
		}

		sigma=temp_l_m-gsl_vector_get(l_m,count);
		sigma_hat=0;
	}else{
		temp_l_m_hat=gsl_vector_get(l_m, count)-(double)((gsl_vector_complex_get(phi,count).dat[model]+epsilon))/(double)((gsl_matrix_get(R_Kernel, count,count)));
		if(temp_l_m_hat<0){
			temp_l_m_hat=0;
		}else if (temp_l_m_hat>C){
			temp_l_m_hat=C;
		}
		sigma=0;
		sigma_hat=temp_l_m_hat-gsl_vector_get(l_m,count);
	}

	g=fabs((sigma-sigma_hat)*(-0.5*(sigma-sigma_hat)*gsl_matrix_get(R_Kernel, count,count)+gsl_vector_complex_get(phi, count).dat[model]-epsilon*(sigma+sigma_hat)/(sigma-sigma_hat)));
	if(g>best_g){
		best_g=g;
		S_i=F_i;
		F_i=count;
	}
}


}




#endif /* WSS2_1DSOLVER_H_ */
