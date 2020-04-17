#ifndef CRANDOM_H
#define CRANDOM_H

#ifdef __cplusplus
extern "C" {
#endif

int crand();
int crand2(int seed);
double cdrand();

double CAverageRandom(double min,double max);

double CNormalRandom(double miu, double sigma,double min,double max);

double CNormRand(double miu, double offset);

#ifdef __cplusplus
}
#endif

#endif // CRANDOM_H
