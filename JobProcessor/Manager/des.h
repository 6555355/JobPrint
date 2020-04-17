#ifndef  __DES__H__
#define __DES__H__
/* ================================================================
des()
Description: DES algorithm,do encript or descript.
flg: 0 Ω‚√‹	1:º”√‹
inkey: 8x8 bytes
================================================================ */
void des(unsigned char *source,unsigned char * dest,unsigned char * inkey, unsigned char flg);
#endif