

#ifndef PRINTCOLOR_H
#define PRINTCOLOR_H


/*
	 0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  ..... 30   31|  32 ........48 | 49 .....63
		Y  M  C  K  k  c  m  y  O  G   R   B   P                    |  user defined  |  Reserved

*/

#define PRINTCOLOR_K		4
#define PRINTCOLOR_C		3
#define PRINTCOLOR_M		2
#define PRINTCOLOR_Y		1

#define PRINTCOLOR_Lk		5
#define PRINTCOLOR_Lc		6
#define PRINTCOLOR_Lm		7
#define PRINTCOLOR_Ly		8
//#define PRINTCOLOR_W		9
//#define PRINTCOLOR_V		10

#define PRINTCOLOR_O		9
#define PRINTCOLOR_G		10
#define PRINTCOLOR_R		11
#define PRINTCOLOR_B		12
#define PRINTCOLOR_P		15
#define PRINTCOLOR_W1		29
#define PRINTCOLOR_W2		30
#define PRINTCOLOR_W8		36
#define PRINTCOLOR_V1		37
#define PRINTCOLOR_V8		44
#define PRINTCOLOR_P1		45
#define PRINTCOLOR_P4		48

#define MAX_PRINTCOLOR		63

#endif
