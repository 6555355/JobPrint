#ifndef		ROTATION_X64_1BIT
#define		ROTATION_X64_1BIT

#define		ROTATION	32



extern void rotation_mp(
	unsigned char * const data_dst,
	unsigned char * const data_src,
	const UINT32 width, const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end,
	int bit, int dir);

extern void rotation(
	unsigned char * const data_dst, 
	unsigned char * const data_src,
	const UINT32 width, const UINT32 height,
	const UINT32 x_start, const UINT32 x_end,
	const UINT32 y_start, const UINT32 y_end,
	int bit, int dir);


#endif // !




