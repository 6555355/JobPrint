#ifndef BI_RLE
#define BI_RLE


#ifdef __cplusplus
extern "C"{
#endif

/* Compress inBuf to outBuf
 * Input:
 *  inSize:  input buf size
 *  inBuf:  input buf, need compress
 *  outSize: output buf size, reality outSize <= 2*inSize, but suggest outSize = 2*inSize
 * Output:
 *  outBuf: compressed buf address
 * Return:
 *  0:  Err, input para fault
 *  1:  Err, outSize < reality compressed buf size
 *  >= 2: Succ, reality used compressed buf size
*/
int BI_RLE8_Compress(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf);

/* Compress one line buf to outBuf
 * Input:
 *  inSize:  input buf size
 *  inBuf:  input one line buf, need compress
 *  outSize: output buf size, reality outSize <= 2*inSize+2, but suggest outSize = 2*inSize+2
 * Output:
 *  outBuf: compressed buf address
 * Return:
 *  0:  Err, input para fault
 *  1:  Err, outSize < reality compressed buf size
 *  >= 2: Succ, reality used compressed buf size
*/
int BI_RLE8_CompressOneLine(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf);


/* uncompress data
 * Input:
 *  inSize      compressed data size
 *  inBuf       compressed data buf
 *  outSize     uncompress data buf size
 * Output:
 *  outBuf      uncompress data buf address
 *  inLen       reality use'd compressed data size, if (*inLen) < 0, inBuf should reserve last size(*inLen) data
 * Return:
 *  -1          input error
 *  -2          outSize < one line size
 *  -3          inSize < one line size
 *  >=   0       success, reality uncompress data size
*/
int BI_RLE8_UnCompressOneLine(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf, int *inLen);

/* uncompress data
 * Input:
 *  inSize      compressed data size
 *  inBuf       compressed data buf
 *  outSize     uncompress data buf size
 * Output:
 *  outBuf      uncompress data buf address
 *  inLen       reality use'd compressed data size
 * Return:
 *  <=  0       failure
 *  >   0       success, reality uncompress data size
*/
int BI_RLE8_UnCompress(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf, int *inLen);


/* uncompress data
 * Input:
 *  inSize      compressed data size
 *  inBuf       compressed data buf
 *  outSize     uncompress data buf size
 * Output:
 *  outBuf      uncompress data buf address
 *  inLen       reality use'd compressed data size, if (*inLen) < 0, inBuf should reserve last size(*inLen) data
 * Return:
 *  <=  0       failure
 *  >   0       success, reality uncompress data size
*/
int BI_RLE8_UnCompressBuf(int inSize, unsigned char *inBuf, int outSize, unsigned char *outBuf, int *inLen);


#ifdef __cplusplus
}
#endif

#endif // BI_RLE

