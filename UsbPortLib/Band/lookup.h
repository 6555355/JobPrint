#ifndef _LOOK_UP_HPP
#define _LOOK_UP_HPP

extern unsigned char		LsbTbl[];
extern unsigned char		MsbTbl[];
extern unsigned char		BrokenTbl[];
extern unsigned char		s_to_d[][256];
extern unsigned short		DivMask[][256];
extern unsigned char		MapNumTbl_1Bit[][256];
extern unsigned char		MapNumTbl_2Bit[][256];

extern unsigned short		ChToShort[];
extern unsigned int			ChToInt[];
extern unsigned long long	ChToLL[];

extern unsigned char        Mirror1bitTable[];
extern unsigned char        Mirror2bitTable[];

extern unsigned char        OddBit[];
#endif