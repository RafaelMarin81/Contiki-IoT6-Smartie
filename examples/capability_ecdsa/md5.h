typedef struct {
  unsigned long i[2];                   /* number of _bits_ handled mod 2^64 */
  unsigned long buf[4];                 /* scratch buffer */
  unsigned char in[64];                   /* input buffer */
} MD5_CTX;

void MD5Init (MD5_CTX *mdContext);
void MD5Update (MD5_CTX *mdContext, unsigned char *inBuf, unsigned short inLen);
void MD5Final (MD5_CTX *mdContext, unsigned char* digest);




