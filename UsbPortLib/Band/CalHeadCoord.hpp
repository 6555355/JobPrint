#ifndef CAL_HEAD_COORD
#define CAL_HEAD_COORD

extern void GetHeadArrang(float * buf, int color, float color_space, int group, float group_space, int div, int mirror, float *dis);
extern int YOffsetInit(int * pyOffsetArray, int * vertical, int *y_coord, char(*overlap)[8], int colornum, int group, bool reverse);

#endif