void hexPrint(uint32 a)
{
  /* This function is just for testing. Remove in the final version */
  char l[17];
  l[0] = '0';
  l[1] = '1';
  l[2] = '2';
  l[3] = '3';
  l[4] = '4';
  l[5] = '5';
  l[6] = '6';
  l[7] = '7';
  l[8] = '8';
  l[9] = '9';
  l[10] = 'A';
  l[11] = 'B';
  l[12] = 'C';
  l[13] = 'D';
  l[14] = 'E';
  l[15] = 'F';
  l[16] = 0;

  char r[9];
  r[8] = 0;
  r[7] = l[a%16];
  a>>=4;
  r[6] = l[a%16];
  a>>=4;
  r[5] = l[a%16];
  a>>=4;
  r[4] = l[a%16];
  a>>=4;
  r[3] = l[a%16];
  a>>=4;
  r[2] = l[a%16];
  a>>=4;
  r[1] = l[a%16];
  a>>=4;
  r[0] = l[a%16];
  PRINTF(r);
  PRINTF("|");
}

void imprimePunto(jacobianPoint * P)
{
  /* This function is just for testing. Remove in the final version */
  int j;
  PRINTF("X:");
  for(j=4;j>=0;j--)
    hexPrint(P->X[j]);
  PRINTF("\n");
  PRINTF("Y:");
  for(j=4;j>=0;j--)
    hexPrint(P->Y[j]);
  PRINTF("\n");
  PRINTF("Z:");
  for(j=4;j>=0;j--)
    hexPrint(P->Z[j]);
  PRINTF("\n");
}

void test_add(){ 
/* Vector tests for void add(uint32 * sol, uint32 * arg1, uint32 * arg2) */
PRINTF("Test Addition"); {
uint32 sol[5];
uint32 arg1[5];
uint32 arg2[5];
uint32 solCorrect[5];
arg1[0]= 0x21c7e014;
arg1[1]= 0xae420757;
arg1[2]= 0xc2f2dc33;
arg1[3]= 0xc1e974d6;
arg1[4]= 0x7b3d3e68;
/* Check arg1[5]= 0x0 */;
arg2[0]= 0xcb5f6f68;
arg2[1]= 0xad0fc8f2;
arg2[2]= 0x93f6c022;
arg2[3]= 0x9844383e;
arg2[4]= 0xfb11dcc9;
/* Check arg2[5]= 0x0 */;
sol[0]= 0x6d274f7d;
sol[1]= 0x5b51d04a;
sol[2]= 0x56e99c56;
sol[3]= 0x5a2dad15;
sol[4]= 0x764f1b32;
/* Check sol[5]= 0x0 */;
add(solCorrect,arg1,arg2);
if(esigual(solCorrect,sol)) { PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_sub(){ 
/* Vector tests for void sub(uint32 * sol, uint32 * arg1, uint32 * arg2) */
PRINTF("Test Substraction"); {
uint32 sol[5];
uint32 arg1[5];
uint32 arg2[5];
uint32 solCorrect[5];
arg1[0]= 0x1d6421c2;
arg1[1]= 0xd0fc7b80;
arg1[2]= 0xa5a24b39;
arg1[3]= 0x3d0d8cd8;
arg1[4]= 0x65c3bb1e;
/* Check arg1[5]= 0x0 */;
arg2[0]= 0x6dd538a5;
arg2[1]= 0x3ee03070;
arg2[2]= 0x2ad55f93;
arg2[3]= 0x806fc33e;
arg2[4]= 0x7b92c73b;
/* Check arg2[5]= 0x0 */;
sol[0]= 0x2f8ee91c;
sol[1]= 0x921c4b0f;
sol[2]= 0x7acceba6;
sol[3]= 0xbc9dc99a;
sol[4]= 0xea30f3e2;
/* Check sol[5]= 0x0 */;
sub(solCorrect,arg1,arg2);
if(esigual(solCorrect,sol)) { PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_mult(){ 
/* Vector tests for void mult(uint32 * sol, uint32 * arg1, uint32 * arg2) */
PRINTF("Test Multiplication"); {
uint32 sol[5];
uint32 arg1[5];
uint32 arg2[5];
uint32 solCorrect[5];
arg1[0]= 0x82584356;
arg1[1]= 0xb2db085a;
arg1[2]= 0x65b9c90e;
arg1[3]= 0xa95f30bb;
arg1[4]= 0x60d85b32;
/* Check arg1[5]= 0x0 */;
arg2[0]= 0xa2f94d01;
arg2[1]= 0x303a7d84;
arg2[2]= 0x32384d30;
arg2[3]= 0x861f21a8;
arg2[4]= 0xccf0cdae;
/* Check arg2[5]= 0x0 */;
sol[0]= 0x423c9074;
sol[1]= 0x5cd10f20;
sol[2]= 0x4b303513;
sol[3]= 0xfc0e608c;
sol[4]= 0x5348cdc8;
/* Check sol[5]= 0x0 */;
mult(solCorrect,arg1,arg2);
if(esigual(solCorrect,sol)) { PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_half(){ 
/* Vector tests for void half(uint32 * sol) */
PRINTF("Test Half"); {
uint32 sol[5];
uint32 solCorrect[5];
solCorrect[0]= 0xe2894880;
solCorrect[1]= 0x89b0920f;
solCorrect[2]= 0x7a43be12;
solCorrect[3]= 0x26879795;
solCorrect[4]= 0xfd75e6f9;
/* Check solCorrect[5]= 0x0 */;
half(solCorrect);
sol[0]= 0xf144a440;
sol[1]= 0x44d84907;
sol[2]= 0xbd21df09;
sol[3]= 0x9343cbca;
sol[4]= 0x7ebaf37c;
/* Check sol[5]= 0x0 */;
if(esigual(solCorrect,sol)) { PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_addModn(){ 
/* Vector tests for void addModn(uint32 * sol, uint32 * b) */
PRINTF("Test Addition Modulo n"); {
uint32 sol[6];
uint32 b[6];
uint32 solCorrect[6];
b[0]= 0xc25c6fce;
b[1]= 0x932d2dda;
b[2]= 0x6659acae;
b[3]= 0x3d628104;
b[4]= 0xe1ee988;
b[5]= 0x0;
/* Check b[6]= 0x0 */;
solCorrect[0]= 0x7bc0654d;
solCorrect[1]= 0x6abcc5c5;
solCorrect[2]= 0x5482c249;
solCorrect[3]= 0x8b9f33d9;
solCorrect[4]= 0xfe9c2693;
solCorrect[5]= 0x0;
/* Check solCorrect[6]= 0x0 */;
addModn(solCorrect,b);
sol[0]= 0x73a7b2c4;
sol[1]= 0x4c244cc;
sol[2]= 0xbada7a2f;
sol[3]= 0xc901b4dd;
sol[4]= 0xcbb101b;
sol[5]= 0x0;
/* Check sol[6]= 0x0 */;
if(solCorrect[5] == sol[5] && esigual(solCorrect,sol)) 
{ PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[5]); PRINTF("::"); hexPrint(sol[5]); PRINTF("\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_subModn(){ 
/* Vector tests for void subModn(uint32 * sol, uint32 * b) */
PRINTF("Test Substraction Modulo n"); {
uint32 sol[6];
uint32 b[6];
uint32 solCorrect[6];
b[0]= 0x7bf67920;
b[1]= 0x65819cfa;
b[2]= 0x5a31a852;
b[3]= 0x585aaf50;
b[4]= 0x76ca8b51;
b[5]= 0x0;
/* Check b[6]= 0x0 */;
solCorrect[0]= 0x8ac23fac;
solCorrect[1]= 0x6652be54;
solCorrect[2]= 0x4f90e617;
solCorrect[3]= 0x9d59430e;
solCorrect[4]= 0xe9c7ebc4;
solCorrect[5]= 0x0;
/* Check solCorrect[6]= 0x0 */;
subModn(solCorrect,b);
sol[0]= 0xecbc68c;
sol[1]= 0xd1215a;
sol[2]= 0xf55f3dc5;
sol[3]= 0x44fe93bd;
sol[4]= 0x72fd6073;
sol[5]= 0x0;
/* Check sol[6]= 0x0 */;
if(solCorrect[5] == sol[5] && esigual(solCorrect,sol)) 
{ PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[5]); PRINTF("::"); hexPrint(sol[5]); PRINTF("\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_multModn(){ 
/* Vector tests for void multModn(uint32 * sol, uint32 * a, uint32 * b) */
PRINTF("Test Multiplication Modulo n"); {
uint32 sol[6];
uint32 a[6];
uint32 b[6];
uint32 solCorrect[6];
b[0]= 0x866d3fd3;
b[1]= 0x2211e011;
b[2]= 0xbe19d6bd;
b[3]= 0xb19f3d55;
b[4]= 0x7cba9aa9;
b[5]= 0x0;
/* Check b[6]= 0x0 */;
a[0]= 0x8ae85c04;
a[1]= 0x4ef3c89c;
a[2]= 0x7b846e8e;
a[3]= 0xfd110dcf;
a[4]= 0xeba84d98;
a[5]= 0x0;
/* Check a[6]= 0x0 */;
multModn(solCorrect,a,b);
sol[0]= 0x67386846;
sol[1]= 0xa94a38b0;
sol[2]= 0x1cc4d294;
sol[3]= 0xf6ea4c6c;
sol[4]= 0xfc9c96fc;
sol[5]= 0x0;
/* Check sol[6]= 0x0 */;
if(solCorrect[5] == sol[5] && esigual(solCorrect,sol)) 
{ PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[5]); PRINTF("::"); hexPrint(sol[5]); PRINTF("\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
void test_halfModn(){ 
/* Vector tests for void halfModn(uint32 * sol) */
PRINTF("Test Addition Modulo n"); {
uint32 sol[6];
uint32 solCorrect[6];
solCorrect[0]= 0xc3f38595;
solCorrect[1]= 0x280b435b;
solCorrect[2]= 0xabf4d9fa;
solCorrect[3]= 0xd7481578;
solCorrect[4]= 0x1cea4be3;
solCorrect[5]= 0x0;
/* Check solCorrect[6]= 0x0 */;
halfModn(solCorrect);
sol[0]= 0xc73453f6;
sol[1]= 0x90997917;
sol[2]= 0x55fb6761;
sol[3]= 0xeba40abc;
sol[4]= 0x8e7525f1;
sol[5]= 0x0;
/* Check sol[6]= 0x0 */;
if(solCorrect[5] == sol[5] && esigual(solCorrect,sol)) 
{ PRINTF("OK"); } else { PRINTF("Error:\n");
hexPrint(solCorrect[5]); PRINTF("::"); hexPrint(sol[5]); PRINTF("\n");
hexPrint(solCorrect[4]); PRINTF("::"); hexPrint(sol[4]); PRINTF("\n");
hexPrint(solCorrect[3]); PRINTF("::"); hexPrint(sol[3]); PRINTF("\n");
hexPrint(solCorrect[2]); PRINTF("::"); hexPrint(sol[2]); PRINTF("\n");
hexPrint(solCorrect[1]); PRINTF("::"); hexPrint(sol[1]); PRINTF("\n");
hexPrint(solCorrect[0]); PRINTF("::"); hexPrint(sol[0]); PRINTF("\n"); }
}
}
