//#include<stdio.h>
//#include<cuda_runtime.h>
//#include<complex.h>
//#include<math.h>
//some complex operations in host side

//define complex number



//define complex addtion
#include<cuComplex.h>
//#include<mathcalls.h>
#include<device_functions.h>
 inline  __device__   cuComplex complex_add(const cuComplex a, const cuComplex b)
	{
      cuComplex result;
      result.x=a.x+b.x;
      result.y=a.y+b.y;
      return result;
	}

//define complex substraction
 inline  __device__  cuComplex complex_sub(const cuComplex a, const cuComplex b)
	{
      cuComplex result;
      result.x=a.x-b.x;
      result.y=a.y-b.y;
      return(result);
	}

//define complex multiplication
 inline  __device__  cuComplex complex_mulcom(const cuComplex a, const cuComplex b)
	{
      cuComplex result;
      result.x=a.x*b.x-a.y*b.y;
      result.y=a.x*b.y+a.y*b.x;
      return(result);
	}

//define complex_diviation
 inline  __device__  cuComplex complex_div(const cuComplex a, const cuComplex b)
	{
      cuComplex result;
      result.x=(a.x*b.x+a.y*b.y)/(pow((b.x),2)+pow((b.y),2));
      result.y=(-a.x*b.y+a.y*b.x)/(pow((b.x),2)+pow((b.y),2));
      return(result);
	}

//define complex abslute value
 inline  __device__  float complex_abs(const cuComplex a)
	{
      float result;
      result=pow(a.x,2)+pow(a.y,2);

      return(result);
	}

 inline  __device__  cuComplex complex_mulreal(const cuComplex a, const float b)
{
	cuComplex result;
	result.x=a.x*b;
	result.y=a.y*b;
	return result;
}

//define conjugate
 inline  __device__  cuComplex complex_conjugate(cuComplex arg)
{
	 cuComplex a;
	 a.x=arg.x;
	 a.y=arg.y;
    return a;
}

//exponential of imaginery number
 inline  __device__  cuComplex complex_exp(float arg)
{
	 cuComplex a;
	 a.x=cos(arg);
	 a.y=sin(arg);
    return a;
}

//complex pow
 inline  __device__  cuComplex complex_pow(cuComplex arg,  float e)
{
	if(e>=1.0)
	{
	int	f=(int)e;
	int i;
	cuComplex a=arg;
	cuComplex temp={0,0};
	//temp=(cuComplex)malloc(sizeof(cuComplex));
	for(i=1;i<f;i++)
	{
	temp.x=arg.x*a.x-arg.y*a.y;
	temp.y=arg.x*a.y+arg.y*a.x;
	arg.x=temp.x;
	arg.y=temp.y;

	}
	//free(temp);
	}
	else    //e=0.5
	{
		float r;
		cuComplex a;
		float b;
		r=sqrt(__powf(arg.x,2)+__powf(arg.y,2));
		b=sqrt(__powf((arg.x+r),2)+__powf(arg.y,2));
		a.x=sqrt(r)*(arg.x+r)/b;
		a.y=sqrt(r)*arg.y/b;
		arg=a;
	}
	cuComplex arg1=arg;
	return arg1;
}
 //transform the matrix from column wise to row wise storage




