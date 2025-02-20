#include "public.h"
void channel_generator (gsl_matrix_complex *pH, gsl_rng *pr)
{
	int  count1=0, count2=0;
//	Nr1=pH->size1;
//	Nt1=pH->size2;
	
	gsl_complex z;
	GSL_SET_COMPLEX (&z, 0.0, 0.0);

	for (count1=0 ; count1<Nr ; count1++)
	    {
		for (count2=0; count2<Nt; count2++)
		    { 
			GSL_SET_COMPLEX (&z, gsl_ran_gaussian (pr, 1.0/sqrt(2.0)), gsl_ran_gaussian (pr, 1.0/sqrt(2.0)));
			gsl_matrix_complex_set (pH, count1, count2, z);
			//printf("z = %g \n", gsl_ran_gaussian (pr, 1.0));
		     }
	    }
	return;
		   
}
