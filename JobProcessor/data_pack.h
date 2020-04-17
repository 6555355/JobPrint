#ifndef _DATA_PACK_H
#define _DATA_PACK_H



#ifdef __X64
#define	BYTE_PACK_LEN	8
#define	BIT_PACK_LEN	(BYTE_PACK_LEN * 8)
#else
#define	BYTE_PACK_LEN	4
#define	BIT_PACK_LEN	(BYTE_PACK_LEN * 8)
#endif

#endif