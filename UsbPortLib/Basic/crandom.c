
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#include "crandom.h"

void crand_init()
{

#ifdef WIN32
    srand((unsigned)time(NULL));
#else
    srand48( time(NULL) );
#endif

}

void crand_init_seed(int seed)
{

#ifdef WIN32
    srand((unsigned int)seed);
#else
    srand48( seed );
#endif

}

int crand()
{
    static int is_inited=0;
    if(!is_inited){
        crand_init();
        is_inited = 1;
    }
    return rand();
}

int crand2(int seed)
{
    static int is_inited=0;
    if(!is_inited){
        crand_init_seed(seed);
        is_inited = 1;
    }
    return rand();
}

double cdrand()
{
    double r=0.0;
#if 1
    static int is_inited=0;
    if(!is_inited){
        crand_init();
        is_inited = 1;
    }
#else
    crand_init();
#endif

#ifdef WIN32
    r = rand();
    r = r / RAND_MAX;
#else
    r = drand48();
#endif

    return r;
}


/* �������ȷֲ������ */
double CAverageRandom(double min,double max)
{
    double r=0.0;
    if(min>=max){
        return 0;
    }
    r = min + (max - min)*cdrand();
    return r;
}


/* ����˵����double miu���̣���̬��������ѧ����

                    double sigma���ң���̬�����ľ�����

                    double min,double max������������������ķ�Χ

    �����Ϸ�����ȡ ��=0����=0.2����Χ��-1~1����400����̬�����
 */


#define PI 3.141592653

/* �����ܶȺ��� */
double K2_Normal(double x,double miu,double sigma)
{
    if(sigma>0){
        return 1.0/sqrt(2*PI*sigma) * exp(-1*(x-miu)*(x-miu)/(2*sigma*sigma));
    }else{
        //fprintf(stderr, "K2_Normal! sigma>0 \n");
        return 0;
    }
}

/* ������̬�ֲ������ */
double CNormalRandom(double miu, double sigma,double min,double max)
{
    double x;
    double dScope;
    double y;
    do
    {
		x = CAverageRandom(min,max);
		y = K2_Normal(x, miu, sigma);
		dScope = CAverageRandom(0, K2_Normal(miu,miu,sigma));
    }while( dScope > y);
    return x;
}

/*  ������miuΪ�м�ֵ��offset��������̬�ֲ������
    miu:�м�ֵ offset:����ֵ */
double CNormRand(double miu, double offset)
{
    double sgm;
    double boundary;

    sgm = offset/3.0;
//    boundary = offset+1;
    boundary = offset;

    return CNormalRandom(miu, sgm, miu-boundary, miu+boundary);
}
