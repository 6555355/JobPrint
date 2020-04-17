
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lookup.h"

unsigned int mask_tbl[] = {
	0x0000, 0x0001, 0x0003, 0x0007,
	0x000F, 0x001F, 0x003F, 0x007F,
	0x00FF, 0x01FF, 0x03FF, 0x07FF,
	0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF,
};
unsigned int setup_0 = 0;
unsigned int setup_1 = 0;
unsigned int setup_2 = 0;
unsigned int setup_3 = 0; 
unsigned int setup_4 = 0;
unsigned int setup_5 = 0;
unsigned int setup_6 = 0;
unsigned int setup_7 = 0;
unsigned int setup_8 = 0;

#ifndef __X64
//RDSEED int _rdseed16_step(unsigned short *);
/* 内嵌汇编，只能被x32平台 调用 */
void _bit_copy_x32(
	unsigned char * src, int src_offset, 
	unsigned char * dst, int dst_offset,
	unsigned int bit_len, int bit2mode)
{
	if ((src_offset % 8) == (dst_offset % 8))
	{
		int offset;
		int offset_s0 = src_offset / 8;
		int offset_d0 = dst_offset / 8;

		setup_0++;
		if (src_offset % 8)
		{
			setup_1++;
			offset = 8 - (src_offset % 8);

			dst[offset_d0] &= ~mask_tbl[offset];
			dst[offset_d0] |= src[offset_s0] & mask_tbl[offset];

			src_offset += offset;
			dst_offset += offset;
			bit_len    -= offset;

			offset_s0 = src_offset >> 3;
			offset_d0 = dst_offset >> 3;
		}

		assert(src_offset % 8 == 0);
		assert(dst_offset % 8 == 0);

		offset = (bit_len + src_offset) % 8;

		int len = bit_len - offset;

		assert(len % 8 == 0);

		len /= 8;

		memcpy(dst + offset_d0, src + offset_s0, len);

		if (offset)
		{
			setup_2++;
			offset_s0 += len;
			offset_d0 += len;

			dst[offset_d0] &= mask_tbl[8 - offset];
			dst[offset_d0] |= src[offset_s0] & ~mask_tbl[8 - offset];
		}
	}
	else
	{
		//cnt_bit_cpy++;
		unsigned char * p_src = &src[src_offset >> 3];
		unsigned char * p_dst = &dst[dst_offset >> 3];
		
		setup_3++;
		//copy header
		if (dst_offset % 8)
		{
			unsigned char high;
			unsigned char low;
			unsigned int offset_s = src_offset % 8;
			unsigned int offset_d = dst_offset % 8;

			if (offset_s > offset_d)
			{
				setup_4++;
				high = (*p_src++ &  mask_tbl[8 - offset_s]);
				low  = (*p_src   & ~mask_tbl[8 - (offset_s - offset_d)]);

				high <<= offset_s - offset_d;
				low >>= offset_d + 8 - offset_s;

				*p_dst &= ~mask_tbl[8 - offset_d];
				*p_dst |= high | low;
			}
			else
			{
				setup_5++;
				high  = *p_src &  mask_tbl[8 - offset_s];
				high &= ~mask_tbl[offset_d - offset_s];

				high >>= offset_d - offset_s;

				*p_dst &= ~mask_tbl[8 - offset_d];
				*p_dst |= high;
			}

			int deta_d = 8 - (dst_offset % 8);

			dst_offset += deta_d;
			src_offset += deta_d;
			bit_len    -= deta_d;

			p_src = &src[src_offset >> 3];
			p_dst = &dst[dst_offset >> 3];
		}
		
		assert(dst_offset % 8 == 0);

		int len = bit_len / 24;
		int left = bit_len % 24;
		int offset = 8 - (src_offset % 8);

		if (len > 0){
			len -= 1;
			left += 24;
		}

		assert(offset != 0);
		{
			unsigned int __offset = offset;		//OFFSET is the keyword in assembly
			__asm{
					push		ebx
					push		edx
					push		edi
					push		esi

					; RDMSR
					; mov			eax, cr3

					mov         ecx, __offset
					mov			ebx, p_src
					mov			edx, p_dst
					mov			edi, 0
					mov			esi, len

					;//for(int edi = 0; edi < esi; edi++)
				LOOP0:
					cmp			edi, esi
					je			LOOP_END

					; read data
					mov         eax, [ebx]
					; shift by word in little - endian mode
					bswap		eax
					shr			eax, cl
					bswap		eax
					shr			eax, 8
					; save data
					mov[edx], eax

					; the valid number of shifting is 3
					add			ebx, 3
					add			edx, 3
					add			edi, 1
					jmp			LOOP0

				LOOP_END:
					mov			p_src, ebx
					mov			p_dst, edx

					pop			esi
					pop			edi
					pop			edx
					pop			ebx
			}
		}
		//copy tail
		while (left > 8)
		{
			setup_6++;
			unsigned char high = (*p_src++ &  mask_tbl[offset]) << (8 - offset);
			unsigned char low  = (*p_src   & ~mask_tbl[offset]) >> (offset);

			*p_dst++ = high | low;
			left -= 8;
		}
		if (left >= offset)
		{
			setup_7++;
			unsigned char high = (*p_src++ &  mask_tbl[offset]) << (8 - offset);
			unsigned char low  = (*p_src   & ~mask_tbl[8 - (left - offset)]) >> (offset);

			*p_dst &= ~mask_tbl[8 - left];
			*p_dst |= high | low;
		}
		else if(left)
		{
			setup_8++;
			unsigned char high;

			high = *p_src &  mask_tbl[offset];
			high &= ~mask_tbl[offset - left];
			high <<= 8 - offset;

			*p_dst &= mask_tbl[8 - left];
			*p_dst |= high;
		}
	}
}
#else

extern "C" int __stdcall  bit_cpy_x64(unsigned int offset, unsigned int len, unsigned char * src, unsigned char * dst);
void _bit_copy_x64(
	unsigned char * src, int src_offset,
	unsigned char * dst, int dst_offset,
	unsigned int bit_len, int bit2mode)
{
	if ((src_offset % 8) == (dst_offset % 8))
	{
		int offset;
		int offset_s0 = src_offset / 8;
		int offset_d0 = dst_offset / 8;

		setup_0++;
		if (src_offset % 8)
		{
			setup_1++;
			offset = 8 - (src_offset % 8);

			dst[offset_d0] &= ~mask_tbl[offset];
			dst[offset_d0] |= src[offset_s0] & mask_tbl[offset];

			src_offset += offset;
			dst_offset += offset;
			bit_len -= offset;

			offset_s0 = src_offset >> 3;
			offset_d0 = dst_offset >> 3;
		}

		assert(src_offset % 8 == 0);
		assert(dst_offset % 8 == 0);

		offset = (bit_len + src_offset) % 8;

		int len = bit_len - offset;

		assert(len % 8 == 0);

		len /= 8;

		memcpy(dst + offset_d0, src + offset_s0, len);

		if (offset)
		{
			setup_2++;
			offset_s0 += len;
			offset_d0 += len;

			dst[offset_d0] &= mask_tbl[8 - offset];
			dst[offset_d0] |= src[offset_s0] & ~mask_tbl[8 - offset];
		}
	}
	else
	{
		//cnt_bit_cpy++;
		unsigned char * p_src = &src[src_offset >> 3];
		unsigned char * p_dst = &dst[dst_offset >> 3];

		setup_3++;
		//copy header
		if (dst_offset % 8)
		{
			unsigned char high;
			unsigned char low;
			unsigned int offset_s = src_offset % 8;
			unsigned int offset_d = dst_offset % 8;

			if (offset_s > offset_d)
			{
				setup_4++;
				high = (*p_src++ &  mask_tbl[8 - offset_s]);
				low = (*p_src   & ~mask_tbl[8 - (offset_s - offset_d)]);

				high <<= offset_s - offset_d;
				low >>= offset_d + 8 - offset_s;

				*p_dst &= ~mask_tbl[8 - offset_d];
				*p_dst |= high | low;
			}
			else
			{
				setup_5++;
				high = *p_src &  mask_tbl[8 - offset_s];
				high &= ~mask_tbl[offset_d - offset_s];

				high >>= offset_d - offset_s;

				*p_dst &= ~mask_tbl[8 - offset_d];
				*p_dst |= high;
			}

			int deta_d = 8 - (dst_offset % 8);

			dst_offset += deta_d;
			src_offset += deta_d;
			bit_len -= deta_d;

			p_src = &src[src_offset >> 3];
			p_dst = &dst[dst_offset >> 3];
		}

		assert(dst_offset % 8 == 0);

		int len = bit_len / 56;
		int left = bit_len % 56;
		int offset = 8 - (src_offset % 8);

		if (len > 0){
			len -= 1;
			left += 56;
		}

		assert(offset != 0);

		/* 调用失败 */
		//copy
		//bit_cpy_x64(offset, len, src, dst);

		//copy tail
		while (left > 8)
		{
			setup_6++;
			unsigned char high = (*p_src++ &  mask_tbl[offset]) << (8 - offset);
			unsigned char low = (*p_src   & ~mask_tbl[offset]) >> (offset);

			*p_dst++ = high | low;
			left -= 8;
		}
		if (left >= offset)
		{
			setup_7++;
			unsigned char high = (*p_src++ &  mask_tbl[offset]) << (8 - offset);
			unsigned char low = (*p_src   & ~mask_tbl[8 - (left - offset)]) >> (offset);

			*p_dst &= ~mask_tbl[8 - left];
			*p_dst |= high | low;
		}
		else if (left)
		{
			setup_8++;
			unsigned char high;

			high = *p_src &  mask_tbl[offset];
			high &= ~mask_tbl[offset - left];
			high <<= 8 - offset;

			*p_dst &= mask_tbl[8 - left];
			*p_dst |= high;
		}
	}
}
#endif

#define	TransPos8(ret, data){\
	ret = \
	(data & 0x8040201008040201LL) | \
	(data & 0x0080402010080402LL) << 7 | \
	(data & 0x0000804020100804LL) << 14 | \
	(data & 0x0000008040201008LL) << 21 | \
	(data & 0x0000000080402010LL) << 28 | \
	(data & 0x0000000000804020LL) << 35 | \
	(data & 0x0000000000008040LL) << 42 | \
	(data & 0x0000000000000080LL) << 49 | \
	((data >> 7) & 0x0080402010080402LL) | \
	((data >> 14) & 0x0000804020100804LL) | \
	((data >> 21) & 0x0000008040201008LL) | \
	((data >> 28) & 0x0000000080402010LL) | \
	((data >> 35) & 0x0000000000804020LL) | \
	((data >> 42) & 0x0000000000008040LL) | \
	((data >> 49) & 0x0000000000000080LL); \
}

/*
*	width:图像宽度，必须32位(4字节)对其
*	height:图像高度，必须32位(4字节)对其
*
*	分割图像，旋转元素，方便多核协作
*	x_start:x方向旋转开始的位置(必须32位(4字节)对齐)
*	x_end : x方向旋转结束的位置(必须32位(4字节)对齐)
*	y_start:y方向旋转开始的位置(必须32位(4字节)对齐)
*	y_end:  y方向旋转结束的位置(必须32位(4字节)对齐)
*
*/
void *rotation32_x64_left_1bit(
	unsigned int * const data_dst, unsigned int * const data_src,
	const unsigned int width, const unsigned int curbandwidth, 
	const unsigned int  curbandstart,const unsigned int height,
	const unsigned int x_start, const unsigned int x_end,
	const unsigned int y_start, const unsigned int y_end)
{
	for (unsigned int j = y_start; j < y_end; j++)
	{
		for (unsigned int i = x_start; i < x_end; i++)
		{
			unsigned long long data_buf[32];
			unsigned long long data0, data1, data2, data3, data4, data5, data6, data7;
			//对Raw(4) * Row(4) 个旋转因子进行位旋转，每次旋转1行
			for (unsigned int n = 0; n < 4; n++)
			{
				unsigned int * src = &data_src[((j << 5) + (n << 3)) * width + i];			

				data0 = src[4 * width] + ((unsigned long long)src[0 * width] << 32);
				data1 = src[5 * width] + ((unsigned long long)src[1 * width] << 32);
				data2 = src[6 * width] + ((unsigned long long)src[2 * width] << 32);
				data3 = src[7 * width] + ((unsigned long long)src[3 * width] << 32);

				data4 =
					((data0 & 0x000000FF000000FF) << 0x18) |
					((data1 & 0x000000FF000000FF) << 0x10) |
					((data2 & 0x000000FF000000FF) << 0x08) |
					((data3 & 0x000000FF000000FF) << 0x00);

				data5 =
					((data0 & 0x0000FF000000FF00) << 0x10) |
					((data1 & 0x0000FF000000FF00) << 0x08) |
					((data2 & 0x0000FF000000FF00) << 0x00) |
					((data3 & 0x0000FF000000FF00) >> 0x08);

				data6 =
					((data0 & 0x00FF000000FF0000) << 0x08) |
					((data1 & 0x00FF000000FF0000) >> 0x00) |
					((data2 & 0x00FF000000FF0000) >> 0x08) |
					((data3 & 0x00FF000000FF0000) >> 0x10);

				data7 =
					((data0 & 0xFF000000FF000000) >> 0x00) |
					((data1 & 0xFF000000FF000000) >> 0x08) |
					((data2 & 0xFF000000FF000000) >> 0x10) |
					((data3 & 0xFF000000FF000000) >> 0x18);

				//8*8位旋转
				TransPos8(data_buf[0 + 4 * n], data4);
				TransPos8(data_buf[1 + 4 * n], data5);
				TransPos8(data_buf[2 + 4 * n], data6);
				TransPos8(data_buf[3 + 4 * n], data7);
			}

			//以最小旋转因子为单位，做字节旋转
			for (unsigned int n = 0; n < 4; n++)
			{

				data0 = data_buf[0x00 + n];
				data1 = data_buf[0x04 + n];
				data2 = data_buf[0x08 + n];
				data3 = data_buf[0x0C + n];

				data4 =
					((data0 & 0x000000FF000000FF) << 0x00) |
					((data1 & 0x000000FF000000FF) << 0x08) |
					((data2 & 0x000000FF000000FF) << 0x10) |
					((data3 & 0x000000FF000000FF) << 0x18);

				data5 =
					((data0 & 0x0000FF000000FF00) >> 0x08) |
					((data1 & 0x0000FF000000FF00) >> 0x00) |
					((data2 & 0x0000FF000000FF00) << 0x08) |
					((data3 & 0x0000FF000000FF00) << 0x10);

				data6 =
					((data0 & 0x00FF000000FF0000) >> 0x10) |
					((data1 & 0x00FF000000FF0000) >> 0x08) |
					((data2 & 0x00FF000000FF0000) << 0x00) |
					((data3 & 0x00FF000000FF0000) << 0x08);

				data7 =
					((data0 & 0xFF000000FF000000) >> 0x18) |
					((data1 & 0xFF000000FF000000) >> 0x10) |
					((data2 & 0xFF000000FF000000) >> 0x08) |
					((data3 & 0xFF000000FF000000) >> 0x00);

				long long offset = (((curbandwidth - 1 - (i-curbandstart)) << 5) + ((3 - n) << 3))* height + j;
				unsigned int * dst = &data_dst[offset];

				dst[0 * height] = (unsigned int)data4;
				dst[4 * height] = data4 >> 32;
				dst[1 * height] = (unsigned int)data5;
				dst[5 * height] = data5 >> 32;
				dst[2 * height] = (unsigned int)data6;
				dst[6 * height] = data6 >> 32;
				dst[3 * height] = (unsigned int)data7;
				dst[7 * height] = data7 >> 32;
			}
		}
	}

	return 0;
}

/*
*	width:图像宽度，必须32位(4字节)对其
*	height:图像高度，必须32位(4字节)对其
*
*	分割图像，旋转元素，方便多核协作
*	x_start:x方向旋转开始的位置(必须32位(4字节)对齐)
*	x_end : x方向旋转结束的位置(必须32位(4字节)对齐)
*	y_start:y方向旋转开始的位置(必须32位(4字节)对齐)
*	y_end:  y方向旋转结束的位置(必须32位(4字节)对齐)
*
*/
void *rotation32_x64_right_1bit(
	unsigned int * const data_dst, unsigned int * const data_src,
	const unsigned int width,   const unsigned int curbandwidth,
	const unsigned int  curbandstart,const unsigned int height,
	const unsigned int x_start, const unsigned int x_end,
	const unsigned int y_start, const unsigned int y_end)
{
	for (unsigned int j = y_start; j < y_end; j++)
	{
		for (unsigned int i = x_start; i < x_end; i++)
		{
			unsigned long long data_buf[32];
			unsigned long long data0, data1, data2, data3, data4, data5, data6, data7;

			for (unsigned int n = 0; n < 4; n++)
			{
				//unsigned int * src = &data_src[((j << 4) + (n << 3)) * width + i];
				unsigned int * src = &data_src[((j << 5) + (n << 3)) * width + i];			

				data0 = src[4 * width] + ((unsigned long long)src[0 * width] << 32);
				data1 = src[5 * width] + ((unsigned long long)src[1 * width] << 32);
				data2 = src[6 * width] + ((unsigned long long)src[2 * width] << 32);
				data3 = src[7 * width] + ((unsigned long long)src[3 * width] << 32);

				data4 =
					((data0 & 0x000000FF000000FF) << 0x18) |
					((data1 & 0x000000FF000000FF) << 0x10) |
					((data2 & 0x000000FF000000FF) << 0x08) |
					((data3 & 0x000000FF000000FF) << 0x00);

				data5 =
					((data0 & 0x0000FF000000FF00) << 0x10) |
					((data1 & 0x0000FF000000FF00) << 0x08) |
					((data2 & 0x0000FF000000FF00) << 0x00) |
					((data3 & 0x0000FF000000FF00) >> 0x08);

				data6 =
					((data0 & 0x00FF000000FF0000) << 0x08) |
					((data1 & 0x00FF000000FF0000) >> 0x00) |
					((data2 & 0x00FF000000FF0000) >> 0x08) |
					((data3 & 0x00FF000000FF0000) >> 0x10);

				data7 =
					((data0 & 0xFF000000FF000000) >> 0x00) |
					((data1 & 0xFF000000FF000000) >> 0x08) |
					((data2 & 0xFF000000FF000000) >> 0x10) |
					((data3 & 0xFF000000FF000000) >> 0x18);

				TransPos8(data_buf[0 + 4 * n], data4);
				TransPos8(data_buf[1 + 4 * n], data5);
				TransPos8(data_buf[2 + 4 * n], data6);
				TransPos8(data_buf[3 + 4 * n], data7);
			}
			for (unsigned int n = 0; n < 4; n++)
			{
				data0 = data_buf[0x00 + n];
				data1 = data_buf[0x04 + n];
				data2 = data_buf[0x08 + n];
				data3 = data_buf[0x0C + n];

				data4 =
					((data0 & 0x000000FF000000FF) << 0x00) |
					((data1 & 0x000000FF000000FF) << 0x08) |
					((data2 & 0x000000FF000000FF) << 0x10) |
					((data3 & 0x000000FF000000FF) << 0x18);

				data5 =
					((data0 & 0x0000FF000000FF00) >> 0x08) |
					((data1 & 0x0000FF000000FF00) >> 0x00) |
					((data2 & 0x0000FF000000FF00) << 0x08) |
					((data3 & 0x0000FF000000FF00) << 0x10);

				data6 =
					((data0 & 0x00FF000000FF0000) >> 0x10) |
					((data1 & 0x00FF000000FF0000) >> 0x08) |
					((data2 & 0x00FF000000FF0000) << 0x00) |
					((data3 & 0x00FF000000FF0000) << 0x08);

				data7 =
					((data0 & 0xFF000000FF000000) >> 0x18) |
					((data1 & 0xFF000000FF000000) >> 0x10) |
					((data2 & 0xFF000000FF000000) >> 0x08) |
					((data3 & 0xFF000000FF000000) >> 0x00);

				unsigned int * dst = &data_dst[(((i-curbandstart) << 5) + (n << 3)) * height + j];
				//unsigned int * dst = &data_dst[(((width - 1 - i) << 5) + ((3 - n) << 3))* height + j];

				dst[7 * height] = (unsigned int)data4;
				dst[3 * height] = data4 >> 32;
				dst[6 * height] = (unsigned int)data5;
				dst[2 * height] = data5 >> 32;
				dst[5 * height] = (unsigned int)data6;
				dst[1 * height] = data6 >> 32;
				dst[4 * height] = (unsigned int)data7;
				dst[0 * height] = data7 >> 32;
			}
		}
	}

	return 0;
}

//未完成
void *rotation32_x64_left_2bit(
	unsigned int * const data_dst, unsigned int * const data_src,
	const unsigned int width,const unsigned int curbandwidth,
	const unsigned int curbandstart,const unsigned int height,
	const unsigned int x_start, const unsigned int x_end,
	const unsigned int y_start, const unsigned int y_end)
{
	for (unsigned int j = y_start; j < y_end; j++)
	{
		for (unsigned int i = x_start; i < x_end; i++)
		{

		}
	}

	return 0;
}


//未完成
void *rotation32_x64_right_2bit(
	unsigned int * const data_dst, unsigned int * const data_src,
	const unsigned int width,const unsigned int curbandwidth,
	const unsigned int  curbandstart,const unsigned int height,
	const unsigned int x_start, const unsigned int x_end,
	const unsigned int y_start, const unsigned int y_end)
{
	for (unsigned int j = y_start; j < y_end; j++)
	{
		for (unsigned int i = x_start; i < x_end; i++)
		{

		}
	}

	return 0;
}

void shuffle_quick_2bit(unsigned char *data, const unsigned int line, const unsigned int len, const unsigned int dir)
{
	assert(line % 4 == 0);
	assert((len / 2) % line == 0);

	unsigned int j;
	unsigned long long * buf = (unsigned long long *)new unsigned char[line * 32 * 2];
	register unsigned long long * p = buf;
	register unsigned long *p_high;
	register unsigned long *p_low;
	if (dir){
		p_high = (unsigned long *)data;
		p_low  = (unsigned long *)((unsigned long long)data + line);
	}
	else{
		p_low  = (unsigned long *)data;
		p_high = (unsigned long *)((unsigned long long)data + line);
	}

	for (j = 0; j < len / 2 / line; j++)
	{
		for (unsigned int i = 0; i < line / 4; i++)
		{
			register unsigned long data_high = *p_high;
			register unsigned long data_low  = *p_low;
			register unsigned long long tmp;

			tmp =
				(unsigned long long)ChToShort[(unsigned char)data_low] | 
				(unsigned long long)ChToShort[(unsigned char)data_high] << 1;
			data_low  >>= 8;
			data_high >>= 8;

			tmp |=
				((unsigned long long)ChToShort[(unsigned char)data_low] |
				((unsigned long long)ChToShort[(unsigned char)data_high] << 1)) << 16;
			data_low  >>= 8;
			data_high >>= 8;

			tmp |=
				((unsigned long long)ChToShort[(unsigned char)data_low] |
				((unsigned long long)ChToShort[(unsigned char)data_high] << 1)) << 32;
			data_low  >>= 8;
			data_high >>= 8;

			tmp |=
				((unsigned long long)ChToShort[(unsigned char)data_low] |
				((unsigned long long)ChToShort[(unsigned char)data_high] << 1)) << 48;
			*p++ = tmp;
			p_high++;
			p_low++;
		}

		p_high += line>>2;
		p_low  += line>>2;
		
		if ((j % 32 == 31) && j){
			memcpy(data, buf, line * 32 * 2);
			p = buf;
			data += line * 32 * 2;
		}
	}

	if (j % 32){
		memcpy(data, buf, line * (j % 32) * 2);
	}

	delete[] buf;

}
