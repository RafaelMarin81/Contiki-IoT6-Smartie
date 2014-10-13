/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/cc.h>

#include <string.h>

#define DEBUG DEBUG_PRINT
#define __MOVITAL__
#include "net/uip-debug.h"

#define SEND_INTERVAL		15 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40

#include "parameters.h"


static struct uip_udp_conn *client_conn;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "ECC client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

#define esigual(a,b) (a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && \
a[3]==b[3] && a[4]==b[4])
#define noesigual(a,b) (a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2] || \
a[3]!=b[3] || a[4]!=b[4])
#define escero(a) (a[0]==0 && a[1]==0 && a[2]==0 && a[3]==0 && a[4]==0)
#define esmayor(a,b) (a[4]>b[4] || (a[4]==b[4] && (a[3] > b[3] || \
(a[3]==b[3] && (a[2]>b[2] || (a[2]==b[2] && (a[1]>b[1] || (a[1]==b[1] \
&& a[0]>b[0]))))))))
#define esmayorIgual(a,b) (a[4]>b[4] || (a[4]==b[4] && (a[3] > b[3] || \
(a[3]==b[3] && (a[2]>b[2] || (a[2]==b[2] && (a[1]>b[1] || (a[1]==b[1] \
&& (a[0]>b[0] || a[0]==b[0])))))))))

#define swap(x,y) {x^=y;y^=x;x^=y;}

uint32 addc(uint32 a, uint32 b, int *c)
{
  // In the final version, this is only one assembler instruction
  uint32 h,l;
  l = (a & 0x0000ffff)+(b & 0x0000ffff)+(*c);
  h = (a>>16) + (b>>16) + (l>>16);
  if((h>>16)!=0) (*c) = 1;
  else (*c) = 0;
  return ((h<<16)+(l&0x0000ffff));
/*
  uint32 t;  
  t = b+*c;
  if(t<b) *c = 1;
  else *c = 0;
  t = a+t;
  if(a<t) *c = 1;
  return t; */
}

int suma(uint32 * sol, uint32 * a, uint32 * b, int c)
{
  sol[0] = addc(a[0],b[0],&c);
  sol[1] = addc(a[1],b[1],&c);
  sol[2] = addc(a[2],b[2],&c);
  sol[3] = addc(a[3],b[3],&c);
  sol[4] = addc(a[4],b[4],&c);
  return c;
}

int inc(uint32 * sol)
{
  int c = 1;
  sol[0] = addc(sol[0],0,&c);
  sol[1] = addc(sol[1],0,&c);
  sol[2] = addc(sol[2],0,&c);
  sol[3] = addc(sol[3],0,&c);
  sol[4] = addc(sol[4],0,&c);
  return c;  
}

void negativo(uint32 * r, uint32 * m)
{
  r[0] = ~m[0];
  r[1] = ~m[1];
  r[2] = ~m[2];
  r[3] = ~m[3];
  r[4] = ~m[4];
  inc(r);
}

void resta(uint32 *a, uint32 *m)
{
  uint32 r[5];
  r[0] = ~m[0];
  r[1] = ~m[1];
  r[2] = ~m[2];
  r[3] = ~m[3];
  r[4] = ~m[4];
  suma(a,a,r,1);
}

uint32 invuint32(uint32 b)
{
  uint32 a,q,r,t,V,v;
  int ciclos = 0;
  a = (~b)+1;
  q = 1;
  t = 0;
  q+=(a/b);
  r = a%b;
  v = 1;
  do{
  V = t-v*q;
  t = v;
  v = V;
  a = b;
  b = r;
  q = a/b;
  r = a%b;
  ciclos++;
  } while(r!=0);
  return V;
}

void shift16right(uint32 * res)
{
  uint32 multPrime[5];
  uint32 v;
  v = (iPrime * (~res[0]+1)) & 0x0000ffff;
  multPrime[0] = (prime[0] & 0x0000ffff)*v;
  multPrime[1] = (prime[1] & 0x0000ffff)*v;
  multPrime[2] = (prime[2] & 0x0000ffff)*v;
  multPrime[3] = (prime[3] & 0x0000ffff)*v;
  multPrime[4] = (prime[4] & 0x0000ffff)*v;
  suma(res,res,multPrime,0);
  res[0]>>=16;
  res[0]+=res[1]<<16;
  res[1]>>=16;
  res[1]+=res[2]<<16;
  res[2]>>=16;
  res[2]+=res[3]<<16;
  res[3]>>=16;
  res[3]+=res[4]<<16;
  if(res[4]>=multPrime[4])
    res[4]>>=16;
  else
    res[4] = (res[4]>>16) + 0x00010000;
  while(esmayorIgual(res,prime))
    resta(res,prime);
  multPrime[0] = (prime[0]>>16)*v;
  multPrime[1] = (prime[1]>>16)*v;
  multPrime[2] = (prime[2]>>16)*v;
  multPrime[3] = (prime[3]>>16)*v;
  multPrime[4] = (prime[4]>>16)*v;
  if(suma(res,res,multPrime,0))
    resta(res,prime);
  while(esmayorIgual(res,prime))
    resta(res,prime);
}

void multandshift16right(uint32 * res, uint32 * a, uint32 b)
{
  uint32 multPrime[5], temp[5], multRes[5];
  uint32 v,carry;
  v = (iPrime * (~(a[0] * b + res[0])+1)) & 0x0000ffff;
  multPrime[0] = (prime[0] & 0x0000ffff)*v;
  multPrime[1] = (prime[1] & 0x0000ffff)*v;
  multPrime[2] = (prime[2] & 0x0000ffff)*v;
  multPrime[3] = (prime[3] & 0x0000ffff)*v;
  multPrime[4] = (prime[4] & 0x0000ffff)*v;
  temp[0] = (a[0] & 0x0000ffff)*b;
  temp[1] = (a[1] & 0x0000ffff)*b;
  temp[2] = (a[2] & 0x0000ffff)*b;
  temp[3] = (a[3] & 0x0000ffff)*b;
  temp[4] = (a[4] & 0x0000ffff)*b;
  suma(temp,temp,multPrime,0);
  carry = 0;
  if(temp[4]<multPrime[4]) carry = 0x00010000;
  suma(res,res,temp,0);
  if(res[4]<temp[4]) carry += 0x00010000;
  res[0]>>=16;
  res[0]+=res[1]<<16;
  res[1]>>=16;
  res[1]+=res[2]<<16;
  res[2]>>=16;
  res[2]+=res[3]<<16;
  res[3]>>=16;
  res[3]+=res[4]<<16;
  res[4]>>=16;
  res[4]+=carry;
  multPrime[0] = (prime[0]>>16)*v;
  multPrime[1] = (prime[1]>>16)*v;
  multPrime[2] = (prime[2]>>16)*v;
  multPrime[3] = (prime[3]>>16)*v;
  multPrime[4] = (prime[4]>>16)*v;
  temp[0] = (a[0]>>16)*b;
  temp[1] = (a[1]>>16)*b;
  temp[2] = (a[2]>>16)*b;
  temp[3] = (a[3]>>16)*b;
  temp[4] = (a[4]>>16)*b;
  if(suma(res,res,multPrime,0))
    resta(res,prime);
  while(esmayorIgual(res,prime))
    resta(res,prime);
  if(suma(res,res,temp,0))
    resta(res,prime);
  while(esmayorIgual(res,prime))
    resta(res,prime);
}

void montgomeryMult(uint32 * sol, uint32 * a, uint32 * b)
{
  uint32 res[5];
  res[0] = res[1] = res[2] = res[3] = res[4] = 0;
  multandshift16right(res,a,b[0]&0x0000ffff);
  multandshift16right(res,a,b[0]>>16);
  multandshift16right(res,a,b[1]&0x0000ffff);
  multandshift16right(res,a,b[1]>>16);
  multandshift16right(res,a,b[2]&0x0000ffff);
  multandshift16right(res,a,b[2]>>16);
  multandshift16right(res,a,b[3]&0x0000ffff);
  multandshift16right(res,a,b[3]>>16);
  multandshift16right(res,a,b[4]&0x0000ffff);
  multandshift16right(res,a,b[4]>>16);
  copy(sol,res);
}

void mult(uint32 * sol, uint32 * arg1, uint32 * arg2)
{
  montgomeryMult(sol,arg1,arg2);
}

void square(uint32 * sol, uint32 * arg)
{
  mult(sol,arg,arg);
}

void add(uint32 * sol, uint32 * arg1, uint32 * arg2)
{
  if(suma(sol,arg1,arg2,0))
    resta(sol,prime);
  while(esmayorIgual(sol,prime))
    resta(sol,prime);
}

void sub(uint32 * sol, uint32 * arg1, uint32 * arg2)
{
  uint32 arg2n[5];
  copy(arg2n,prime);
  resta(arg2n,arg2);
  add(sol,arg1,arg2n);
}

void half(uint32 * sol)
{
  int c0,c1,c2,c3,c4;
  c4 = 0;
  if(sol[0]%2 != 0)
    {
    c4 = suma(sol,sol,prime,0);    
    }
  c0 = sol[1] % 2; 
  c1 = sol[2] % 2;
  c2 = sol[3] % 2;
  c3 = sol[4] % 2;
  sol[0] >>= 1;
  sol[1] >>= 1;
  sol[2] >>= 1;
  sol[3] >>= 1;
  sol[4] >>= 1;
  if(c0) sol[0]+= 0x80000000;
  if(c1) sol[1]+= 0x80000000;
  if(c2) sol[2]+= 0x80000000;
  if(c3) sol[3]+= 0x80000000;
  if(c4) sol[4]+= 0x80000000;
}

void times(uint32 * sol, int t, uint32 * arg)
{
 if(t==2)
   add(sol,arg,arg);
 else if(t==3)
   {
   uint32 temp[INTSPERKEY];
   add(temp,arg,arg);
   add(sol,temp,arg);
   }
}

int equalPoints(jacobianPoint * Q, affinePoint * P)
{
  uint32 XX[5];
  uint32 YY[5];
  square(YY,Q->Z);
  mult(XX,P->x,YY);
  mult(YY,YY,Q->Z);
  mult(YY,YY,P->y);
  return (esigual(XX,Q->X) && esigual(YY,Q->Y));
}

void doubling(jacobianPoint * sol, jacobianPoint * P)
{
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.21, page 91. */
 if(infty(P))
   {
   copy(sol,P);
   }
 else
   {
   uint32 T1[INTSPERKEY];
   uint32 T2[INTSPERKEY];
   uint32 T3[INTSPERKEY];

   square(T1,P->Z);             //  2. T1 <- Z1^2
   sub(T2,P->X,T1);             //  3. T2 <- X1-T1
   add(T1,P->X,T1);             //  4. T1 <- X1+T1
   mult(T2,T2,T1);              //  5. T2 <- T2.T1
   times(T2,3,T2);              //  6. T2 <- 3T2

   times(sol->Y,2,P->Y);        //  7. Y3 <- 2Y1
   mult(sol->Z,sol->Y,P->Z);    //  8. Z3 <- Y3.Z1

   square(sol->Y,sol->Y);       //  9. Y3 <- Y3^2
   mult(T3,sol->Y,P->X);        // 10. T3 <- Y3.X1

   square(sol->Y,sol->Y);       // 11. Y3 <- Y3^2
   half(sol->Y);                // 12. Y3 <- Y3/2  = 8Y1^4

   square(sol->X,T2);           // 13. X3 <- T2^2
   times(T1,2,T3);              // 14. T1 <- 2T3
   sub(sol->X,sol->X,T1);       // 15. X3 <- X3-T1

   sub(T1,T3,sol->X);           // 16. T1 <- T3-X3
   mult(T1,T1,T2);              // 17. T1 <- T1.T2
   sub(sol->Y,T1,sol->Y);       // 18. Y3 <- T1-Y3 
   }
}

void addition(jacobianPoint * sol, jacobianPoint * P, affinePoint * Q)
{
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.22, page 91. */
 if(infty(P))
   {
   toJacobian(sol,Q);
   }
 else
   {
   uint32 T1[INTSPERKEY];
   uint32 T2[INTSPERKEY];
   uint32 T3[INTSPERKEY];
   uint32 T4[INTSPERKEY];

   square(T1,P->Z);   // 3. T1 <- Z1^2
   mult(T2,T1,P->Z);  // 4. T2 <- T1.Z1
   mult(T1,T1,Q->x);  // 5. T1 <- T1.x2
   mult(T2,T2,Q->y);  // 6. T2 <- T2.y2
   sub(T1,T1,P->X);   // 7. T1 <- T1-X1
   sub(T2,T2,P->Y);   // 8. T2 <- T2-Y1
   if(iszero(T1))
     {
     if(iszero(T2))
       {
       toJacobian(sol,Q);
       doubling(sol,sol);
       }
     else
       {
       setInfty(sol);
       }
     }
   else
     {
     mult(sol->Z,P->Z,T1);  // 10. Z3 <- Z1.T1
     square(T3,T1);         // 11. T3 <- T1^2
     mult(T4,T3,T1);        // 12. T4 <- T3.T1
     mult(T3,T3,P->X);      // 13. T3 <- T3.X1
     times(T1,2,T3);        // 14. T1 <- 2T3
     square(sol->X,T2);     // 15. X3 <- T2^2
     sub(sol->X,sol->X,T1); // 16. X3 <- X3-T1
     sub(sol->X,sol->X,T4); // 17. X3 <- X3-T4
     sub(T3,T3,sol->X);     // 18. T3 <- T3-X3
     mult(T3,T3,T2);        // 19. T3 <- T3.T2
     mult(T4,T4,P->Y);      // 20. T4 <- T4.Y1
     sub(sol->Y,T3,T4);     // 21. Y3 <- T3-T4
     }
   }
}

void powering(jacobianPoint * sol, affinePoint * base, uint32 * exp, int sizeExp)
{
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.27, page 97. */
  uint32 j,mask;
  setInfty(sol);

  for(j=sizeExp-1;j<sizeExp;j--)
    {
    for(mask=0x80000000; mask!=0; mask>>=1)
      {
      doubling(sol,sol);
      if((mask & exp[j])!=0)
         addition(sol,sol,base);
      }
    }
}

void powering2(jacobianPoint * sol, affinePoint * b1, uint32 * e1, affinePoint * b2, uint32 * e2, int sizeExp)
{
/* Double powering, Shamir's trick. Easy version */
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.48, page 109. */
  uint32 j,mask;
  setInfty(sol);

  for(j=sizeExp-1;j<sizeExp;j--)
    {
    for(mask=0x80000000; mask!=0; mask>>=1)
      {
      doubling(sol,sol);
      if((mask & e1[j])!=0)
         addition(sol,sol,b1);
      if((mask & e2[j])!=0)
         addition(sol,sol,b2);
      }
    }
}

void unMontgomerize(uint32 * sol)
{
  /* This function gets sol, a number in Montgomery representation, and returns the
  same number in normal representation in the same variable sol */
  uint32 u[5];
  u[4] = u[1] = u[2] = u[3] = 0;
  u[0] = 1;
  montgomeryMult(sol,sol,u);  
}

void toInteger(uint32 * x, jacobianPoint * P)
{
  uint32 A[2][5];
  uint32 B[2][5];
  copy(A[0],prime);
  A[1][0] = A[1][1] = A[1][2] = A[1][3] = A[1][4] = 0;
  square(B[0],P->Z);
  copy(B[1],P->X);
  while(B[0][0]%2 == 0) {
    half(B[0]);
    half(B[1]);
    }
  while(!(A[0][4] == 0 && A[0][3] == 0 && A[0][2] == 0 && A[0][1] == 0 && A[0][0]==1)) {
    if(esmayorIgual(B[0],A[0])) {
      swap(A[0][4],B[0][4]);
      swap(A[0][3],B[0][3]);
      swap(A[0][2],B[0][2]);
      swap(A[0][1],B[0][1]);
      swap(A[0][0],B[0][0]);
      swap(A[1][4],B[1][4]);
      swap(A[1][3],B[1][3]);
      swap(A[1][2],B[1][2]);
      swap(A[1][1],B[1][1]);
      swap(A[1][0],B[1][0]);
      }
    if((A[0][4] < 0x3FFFFFFF) && ((A[0][0] & 3) != (B[0][0] & 3))) {
      add(A[0],A[0],B[0]);
      add(A[1],A[1],B[1]); 
      } else {
      sub(A[0],A[0],B[0]);
      sub(A[1],A[1],B[1]);
      }
    while(A[0][0]%2 == 0) {
      half(A[0]);
      half(A[1]);
      }
    }
  copy(x,A[1]);
}

void halfModn(uint32 * sol)
{
  int c0,c1,c2,c3,c4;
  uint32 n[6];
  n[5] = 1;
  n[4] = 0;
  n[3] = 0;
  n[2] = 0x1f4c8;
  n[1] = 0xf927aed3;
  n[0] = 0xca752257;  
  if(sol[0]%2 != 0)
    {
    sol[5] += suma(sol,sol,n,0);
    sol[5]++;    
    }
  c0 = sol[1] % 2; 
  c1 = sol[2] % 2;
  c2 = sol[3] % 2;
  c3 = sol[4] % 2;
  c4 = sol[5] % 2;
  sol[0] >>= 1;
  sol[1] >>= 1;
  sol[2] >>= 1;
  sol[3] >>= 1;
  sol[4] >>= 1;
  sol[5] >>= 1;
  if(c0) sol[0]+= 0x80000000;
  if(c1) sol[1]+= 0x80000000;
  if(c2) sol[2]+= 0x80000000;
  if(c3) sol[3]+= 0x80000000;
  if(c4) sol[4]+= 0x80000000;
}

void addModn(uint32 * sol, uint32 * b)
{
  int c;
  c = 0;
  uint32 n[6];
  n[5] = 1;
  n[4] = 0;
  n[3] = 0;
  n[2] = 0x1f4c8;
  n[1] = 0xf927aed3;
  n[0] = 0xca752257;
  sol[0] = addc(sol[0],b[0],&c);
  sol[1] = addc(sol[1],b[1],&c);
  sol[2] = addc(sol[2],b[2],&c);
  sol[3] = addc(sol[3],b[3],&c);
  sol[4] = addc(sol[4],b[4],&c);
  sol[5] = addc(sol[5],b[5],&c);
  if(sol[5] == 2 || (sol[5] == 1 && esmayorIgual(sol,n)))
    {
    sol[5]--;
    resta(sol,n);
    }
}

void subModn(uint32 * sol, uint32 * b)
{
  int c;
  c = 1;
  uint32 nb[6];
  nb[5] = 1;
  nb[4] = 0;
  nb[3] = 0;
  nb[2] = 0x1f4c8;
  nb[1] = 0xf927aed3;
  nb[0] = 0xca752257;
  nb[0] = addc(nb[0],~b[0],&c);
  nb[1] = addc(nb[1],~b[1],&c);
  nb[2] = addc(nb[2],~b[2],&c);
  nb[3] = addc(nb[3],~b[3],&c);
  nb[4] = addc(nb[4],~b[4],&c);
  nb[5] = addc(nb[5],~b[5],&c);
  addModn(sol,nb);
}

void multModn(uint32 * sol, uint32 * a, uint32 * b)
{
  uint32 j,mask;
  sol[0] = sol[1] = sol[2] = sol[3] = sol[4] = sol[5] = 0;
  for(j=5;j<6;j--)
    {
    for(mask=0x80000000; mask!=0; mask>>=1)
      {
      addModn(sol,sol);
      if((mask & b[j])!=0)
         addModn(sol,a);
      }
    }
}

void divisionModn(uint32 * x, uint32 * y)
{
  /* This function computes xy^{-1} (n) and stores the result in x.
  with n the order of the group */
  uint32 A[2][6];
  uint32 B[2][6];
  A[0][5] = 1;
  A[0][4] = 0;
  A[0][3] = 0;
  A[0][2] = 0x1f4c8;
  A[0][1] = 0xf927aed3;
  A[0][0] = 0xca752257;
  A[1][0] = A[1][1] = A[1][2] = A[1][3] = A[1][4] = A[1][5] = 0;
  copy(B[1],x); B[1][5] = x[5];
  copy(B[0],y); B[0][5] = y[5];
  while(B[0][0]%2 == 0) {
    halfModn(B[0]);
    halfModn(B[1]);
    }
  while(!(A[0][5] == 0 && A[0][4] == 0 && A[0][3] == 0 && A[0][2] == 0 && A[0][1] == 0 && A[0][0]==1)) {
    if(B[0][5] > A[0][5] || (B[0][5] == A[0][5] && (esmayorIgual(B[0],A[0])))) { 
      swap(A[0][5],B[0][5]);
      swap(A[0][4],B[0][4]);
      swap(A[0][3],B[0][3]);
      swap(A[0][2],B[0][2]);
      swap(A[0][1],B[0][1]);
      swap(A[0][0],B[0][0]);
      swap(A[1][5],B[1][5]);
      swap(A[1][4],B[1][4]);
      swap(A[1][3],B[1][3]);
      swap(A[1][2],B[1][2]);
      swap(A[1][1],B[1][1]);
      swap(A[1][0],B[1][0]);
      } 
    subModn(A[0],B[0]);
    subModn(A[1],B[1]);
    while(A[0][0]%2 == 0) {
      halfModn(A[0]);
      halfModn(A[1]);
      }
    }
  copy(x,A[1]);
  x[5] = A[1][5];
}

void generateSignature(uint32 * e,uint32 * r, uint32 * s)
{
  /* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
  Cryptography, Springer 2004. Algorithm 4.29, page 184. */

  /* G is the generator of the curve, n the order of G and e the hash of 
  the message that we are signing. The signature generated is returned in
  the parameters e, r and s that should be uint32 [6] (although e and r stores
  variables of size [5] and the extra word is always 0 */
  uint32 k[6],x[5],d[6]; 
  /* This is the private Key */
  d[5] = 0;
  d[4] = 0x3f882575;
  d[3] = 0x01282305;
  d[2] = 0xa7f79589;
  d[1] = 0x6e00b17b;
  d[0] = 0x0197b99d;
  jacobianPoint P;
  do {
    do {
      do{
        k[0] = rand();
        k[1] = rand();
        k[2] = rand();
        k[3] = rand();
        k[4] = rand();
        k[5] = 0;
        }while(iszero(k));
      powering(&P,&G,k,6);
      toInteger(x,&P);
      /* r = x%n; In our case n>p */
      }while(iszero(x));
    copy(r,x);
    r[5] = 0;
    multModn(s,d,r);
    addModn(s,e);
  } while(s[5] == 0 && iszero(s));
  divisionModn(s,k);
}

int verifySignature(uint32 * e,uint32 * r, uint32 * s)
{
  /* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic 
     Curve Cryptography, Springer 2004. Algorithm 4.30, page 184. */
  /* (r,s) is the signature, e is the message signed. G is the 
     generator of the elliptic curve and n is the order of G      */
  /* The public Key is the affine Point Q                         */
  uint32 u1[6],u2[6],x[6];
  affinePoint Q;
  jacobianPoint X;
  Q.x[4] = 0x08d84d7e;
  Q.x[3] = 0xf717e8b8;
  Q.x[2] = 0x1e77456c;
  Q.x[1] = 0xeba5e6d5;
  Q.x[0] = 0x0a8c3c97;
  Q.y[4] = 0xbcf5d768;
  Q.y[3] = 0x113af899;
  Q.y[2] = 0xa9295f2e;
  Q.y[1] = 0xebfae957;
  Q.y[0] = 0x3fc89f00;

  copy(u1,e);
  copy(u2,r);
  u1[5] = u2[5] = 0;
  divisionModn(u1,s);
  divisionModn(u2,s);
  powering2(&X,&G,u1,&Q,u2,6);
  if(infty((&X))) return 0; 
  else
    {
    toInteger(x,&X);
    // This is because in our case p < n
    if(esigual(x,r)) return 1;
    else return 0;
    }
}

// #include "testing.c"

static void
tcpip_handler(void)
{
  char *str;

  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    printf("Response from the server: '%s'\n", str);
  }
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  PRINTF("Client sending to: ");
  PRINT6ADDR(&client_conn->ripaddr);
  sprintf(buf, "Hello %d from the client", ++seq_id);
  PRINTF(" (msg: %s)\n", buf);
#if SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION
  uip_udp_packet_send(client_conn, buf, UIP_APPDATA_SIZE);
#else /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
  uip_udp_packet_send(client_conn, buf, strlen(buf));
#endif /* SEND_TOO_LARGE_PACKET_TO_TEST_FRAGMENTATION */
}
/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Client IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/

static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}
/*---------------------------------------------------------------------------*/
static void
set_connection_address(uip_ipaddr_t *ipaddr)
{
  uip_ip6addr(ipaddr,0xaaaa,0,0,0,0,0,0,0x1);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer et;
  uip_ipaddr_t ipaddr;

  PROCESS_BEGIN();
  vUART_DataInit();
/*
  PRINTF("UDP client process started\n");

  set_global_address();

  print_local_addresses();

  set_connection_address(&ipaddr);
*/
  /* new connection with remote host */
/*
  client_conn = udp_new(&ipaddr, UIP_HTONS(3000), NULL);
  udp_bind(client_conn, UIP_HTONS(3001));

  PRINTF("Created a connection with the server ");
  PRINT6ADDR(&client_conn->ripaddr);
  PRINTF("local/remote port %d/%d\n",
	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

  etimer_set(&et, SEND_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }
*/

  prime[4] = PRIME4;
  prime[3] = PRIME3;
  prime[2] = PRIME2;
  prime[1] = PRIME1;
  prime[0] = PRIME0;
  iPrime = invuint32(prime[0]);
  G.x[4] = 0x92116efc;
  G.x[3] = 0xb227a7ec;
  G.x[2] = 0xfac62f66;
  G.x[1] = 0x054f3754;
  G.x[0] = 0x39175736;
  G.y[4] = 0xbc5a7293;
  G.y[3] = 0xde56f906;
  G.y[2] = 0x5bee71ad;
  G.y[1] = 0xca6fd8e5;
  G.y[0] = 0x8c990f5c;

  jacobianPoint P;
  uint32 n[6];
  n[5] = 1;
  n[4] = 0;
  n[3] = 0;
  n[2] = 0x1f4c8;
  n[1] = 0xf927aed3;
  n[0] = 0xca752257;

  //PRINTF("Init\n");
PRINTF("A\n");

  uint32 e[6],r[6],s[6];
  e[5] = 0;
  e[4] = 0x96494d99;
  e[3] = 0x3f6f8bf1;
  e[2] = 0xd998588a;
  e[1] = 0x4fb11c53;
  e[0] = 0x23c1bdfe;

  generateSignature(e,r,s);
PRINTF("B\n");
  //PRINTF("Signature Generated\n");
  if(verifySignature(e,r,s))
    //PRINTF("Signature Verified OK\n");
PRINTF("C\n");
  else
    //PRINTF("Signature Not Valid\n");
PRINTF("D\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
