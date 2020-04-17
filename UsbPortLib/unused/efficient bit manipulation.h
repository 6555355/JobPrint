
#ifndef EFF_BIT_MPU
#define EFF_BIT_MPU

typedef		unsigned long long		UINT64;
typedef		long long				INT64;
typedef		unsigned int			UINT32;

extern void _bit_copy_x32(
	unsigned char * src, int src_offset,
	unsigned char * dst, int dst_offset,
	unsigned int bit_len, int bit2mode
);

extern void _bit_copy_x64(
	unsigned char * src, int src_offset,
	unsigned char * dst, int dst_offset,
	unsigned int bit_len, int bit2mode);

extern void *rotation32_x64_left_1bit(
	unsigned int * const data_dst,
	unsigned int * const data_src,
	const UINT32 width, const UINT32 curbandwidth,
	const UINT32 curbandstart,const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end
);

extern void *rotation32_x64_right_1bit(
	unsigned int * const data_dst,
	unsigned int * const data_src,
	const UINT32 width,  const UINT32 curbandwidth,
	const UINT32 curbandstart,const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end
);

extern void *rotation32_x64_left_2bit(
	unsigned int * const data_dst,
	unsigned int * const data_src,
	const UINT32 width,  const UINT32 curbandwidth,
	const UINT32 curbandstart,const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end
);

extern void *rotation32_x64_right_2bit(
	unsigned int * const data_dst,
	unsigned int * const data_src,
	const UINT32 width,  const UINT32 curbandwidth,
	const UINT32 curbandstart,const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end
);

extern void shuffle_quick_2bit(
	unsigned char *data,
	unsigned int line,
	unsigned int len,
	unsigned int dir
);


#endif