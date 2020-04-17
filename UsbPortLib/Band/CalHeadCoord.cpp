

int YOffsetInit(int * pyOffsetArray, int * vertical, int *y_coord, char(*overlap)[8], int colornum, int group, bool reverse)
{
	int offset[32];
	if (!reverse){
		for (int c = 0; c < colornum; c++){
			offset[c] = vertical[c];
		}
	}
	else
	{
		for (int c = 0; c < colornum; c++){
			offset[c] = vertical[c];
		}
		for (int c = 0; c < colornum; c++){
			for (int g = 0; g < group; g++){
				offset[c] -= overlap[c][g];
			}
		}
	}

	{
		for (int i = 0; i< colornum; i++){
			pyOffsetArray[i] = offset[i] + y_coord[i];
		}

		//int maxvalue = 0;
		//int minvalue = 1024;
		int maxvalue = -0x3FFFFF;
		int minvalue = 0x3FFFFF;
		for (int i = 0; i< colornum; i++){
			if (pyOffsetArray[i] < minvalue)
				minvalue = pyOffsetArray[i];
			if (pyOffsetArray[i] >  maxvalue)
				maxvalue = pyOffsetArray[i];
		}

		if (reverse){
			for (int i = 0; i< colornum; i++){
				pyOffsetArray[i] = maxvalue - pyOffsetArray[i];
			}
		}
		else{
			for (int i = 0; i< colornum; i++){
				pyOffsetArray[i] -= minvalue;
			}
		}
	}

	int maxvalue = -0x3FFFFF;
	int minvalue = 0x3FFFFF;
	for (int i = 0; i< colornum; i++)
	{
		if (pyOffsetArray[i] < minvalue)
			minvalue = pyOffsetArray[i];
		if (pyOffsetArray[i] >  maxvalue)
			maxvalue = pyOffsetArray[i];
	}

	return (maxvalue - minvalue);
}

void GetHeadArrang(float * buf, int color, float color_space, int group, float group_space, int div, int mirror, float *dis)
{
	float offset[128];
	int head_num_group = color * 2 / div;
	for (int g = 0; g < group; g++){
		for (int h = 0; h < head_num_group; h++){
			offset[head_num_group * g + h] = color_space * h + group_space * (g % 2);
		}
	}

	for (int g = 0; g < group; g++){
		for (int h = 0; h < head_num_group; h++){
			for (int d = 0; d < div; d++){
				buf[div * (head_num_group * g + h) + d] = offset[head_num_group * g + h] + dis[d];
			}
		}
	}
	float tmp[32];
	for (int g = 0; g < group; g++){
		for (int i = 0, h = head_num_group; i < head_num_group; h++, i++){
			tmp[i] = buf[head_num_group * 2 * g + h];
		}
		for (int i = 0, h = head_num_group; i < head_num_group; h++, i++){
			buf[head_num_group * 2 * g + h] = tmp[head_num_group - 1 - i];
		}
	}
}




