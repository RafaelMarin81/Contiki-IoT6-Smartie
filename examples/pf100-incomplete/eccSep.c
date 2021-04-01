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
#include <stdarg.h>
#include <ctype.h>
#include <sys/cc.h>

#include <string.h>

#define DEBUG DEBUG_PRINT
#define __MOVITAL__
#include "net/uip-debug.h"

#define SEND_INTERVAL		15 * CLOCK_SECOND
#define MAX_PAYLOAD_LEN		40


#define BITS_KEY 160
#define BITS_INTEGER 32
#define INTSPERKEY 5

#define isequal(a,b) (a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && \
a[3]==b[3] && a[4]==b[4])
#define copy(d,o) d[0]=o[0]; d[1]=o[1]; d[2]=o[2]; \
d[3]=o[3]; d[4]=o[4];
#define notequal(a,b) (a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2] || \
a[3]!=b[3] || a[4]!=b[4])
#define iszero(a) (a[0]==0 && a[1]==0 && a[2]==0 && \
a[3]==0 && a[4]==0)


typedef struct jacobianPoint_struct{
 uint32 X[INTSPERKEY];
 uint32 Y[INTSPERKEY];
 uint32 Z[INTSPERKEY];
}jacobianPoint;

typedef struct affinePoint_struct{
 uint32 x[INTSPERKEY];
 uint32 y[INTSPERKEY];
}affinePoint;


#define infty(P) (P->X[0] == 1 && P->X[1] == 0 && P->X[2] == 0 && P->X[3] == 0 && P->X[4] == 0 && \
P->Y[0] == 1 && P->Y[1] == 0 && P->Y[2] == 0 && P->Y[3] == 0 && P->Y[4] == 0 && \
P->Z[0] == 0 && P->Z[1] == 0 && P->Z[2] == 0 && P->Z[3] == 0 && P->Z[4] == 0)
#define toJacobian(PP,P) PP->X[0] = P->x[0]; PP->X[1] = P->x[1]; PP->X[2] = P->x[2]; PP->X[3] = P->x[3]; PP->X[4] = P->x[4]; \
PP->Y[0] = P->y[0]; PP->Y[1] = P->y[1]; PP->Y[2] = P->y[2]; PP->Y[3] = P->y[3]; PP->Y[4] = P->y[4]; \
PP->Z[0] = 1; PP->Z[1] = 0; PP->Z[2] = 0; PP->Z[3] = 0; PP->Z[4] = 0;

static struct uip_udp_conn *client_conn;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "ECC client process");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

void hexPrint(uint32 * a)
{
    int j;
  for(j=4;j>=0;j--)
    PRINTF("%4.4x.%4.4x|",(a[j]>>16),a[j]%65536);
}

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

void suma3(uint32 * r, uint32 * a, uint32 * m)
{
   r[0] = a[0]+m[0];
   r[1] = a[1]+m[1];
   r[2] = a[2]+m[2];
   r[3] = a[3]+m[3];
   r[4] = a[4]+m[4];
   if(a[0]>~m[0])
      {
      r[1]++;
      if(r[1]==0)
        {
        r[2]++;
        if(r[2]==0)
          {
          r[3]++;
          if(r[3]==0)
            r[4]++;
          }
        }
      }
   if(a[1]>~m[1])
      {
      r[2]++;
      if(r[2]==0)
        {
        r[3]++;
        if(r[3]==0)
          r[4]++;
        }
      }
   if(a[2]>~m[2])
      {
      r[3]++;
      if(r[3]==0)
        r[4]++;
      }
   if(a[3]>~m[3])
        r[4]++;
}

void suma(uint32 * a, uint32 * m)
{
   uint32 r[5];
   r[0] = a[0]+m[0];
   r[1] = a[1]+m[1];
   r[2] = a[2]+m[2];
   r[3] = a[3]+m[3];
   r[4] = a[4]+m[4];
   if(a[0]>~m[0])
      {
      r[1]++;
      if(r[1]==0)
        {
        r[2]++;
        if(r[2]==0)
          {
          r[3]++;
          if(r[3]==0)
            r[4]++;
          }
        }
      }
   if(a[1]>~m[1])
      {
      r[2]++;
      if(r[2]==0)
        {
        r[3]++;
        if(r[3]==0)
          r[4]++;
        }
      }
   if(a[2]>~m[2])
      {
      r[3]++;
      if(r[3]==0)
        r[4]++;
      }
   if(a[3]>~m[3])
        r[4]++;
   copy(a,r);
}


void negativo(uint32 * r, uint32 * m)
{
  r[0] = ~m[0];
  r[1] = ~m[1];
  r[2] = ~m[2];
  r[3] = ~m[3];
  r[4] = ~m[4];
  r[0]++;
  if(r[0]==0)
    {
    r[1]++;
    if(r[1]==0)
      {
      r[2]++;
      if(r[2]==0)
        {
        r[3]++;
        if(r[3]==0)
          r[4]++;
        }
      }
    }
}

void resta(uint32 *a, uint32 *m)
{
  uint32 r[5];
  negativo(r,m);
  suma(a,r);
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

void shift16right(uint32 * res, uint32 * prime, uint32 iPrime)
{
  uint32 multPrime[5];
  uint32 v;
  v = (iPrime * (~res[0]+1)) & 0x0000ffff;
  multPrime[0] = (prime[0] & 0x0000ffff)*v;
  multPrime[1] = (prime[1] & 0x0000ffff)*v;
  multPrime[2] = (prime[2] & 0x0000ffff)*v;
  multPrime[3] = (prime[3] & 0x0000ffff)*v;
  multPrime[4] = (prime[4] & 0x0000ffff)*v;
  suma(res,multPrime);
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
  suma(res,multPrime);
  if(res[4]<multPrime[4]) resta(res,prime);
  while(esmayorIgual(res,prime))
    resta(res,prime);
}

void multandshift16right(uint32 * res, uint32 * a, uint32 b, uint32 * prime, uint32 iPrime)
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
  suma(temp,multPrime);
  carry = 0;
  if(temp[4]<multPrime[4]) carry = 0x00010000;
  suma(res,temp);
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
  suma(res,multPrime);
  if(res[4]<multPrime[4]) resta(res,prime);
  while(esmayorIgual(res,prime))
    resta(res,prime);
  suma(res,temp);
  if(res[4]<temp[4]) resta(res,prime);
  while(esmayorIgual(res,prime))
    resta(res,prime);
}

void montgomeryMult(uint32 * res, uint32 * a, uint32 * b, uint32 * prime, uint32 iPrime)
{
  res[0] = res[1] = res[2] = res[3] = res[4] = 0;
  multandshift16right(res,a,b[0]&0x0000ffff,prime,iPrime);
  multandshift16right(res,a,b[0]>>16,prime,iPrime);
  multandshift16right(res,a,b[1]&0x0000ffff,prime,iPrime);
  multandshift16right(res,a,b[1]>>16,prime,iPrime);
  multandshift16right(res,a,b[2]&0x0000ffff,prime,iPrime);
  multandshift16right(res,a,b[2]>>16,prime,iPrime);
  multandshift16right(res,a,b[3]&0x0000ffff,prime,iPrime);
  multandshift16right(res,a,b[3]>>16,prime,iPrime);
  multandshift16right(res,a,b[4]&0x0000ffff,prime,iPrime);
  multandshift16right(res,a,b[4]>>16,prime,iPrime);
}

void mult(uint32 * sol, uint32 * arg1, uint32 * arg2, uint32 * prime, uint32 iprime)
{
  montgomeryMult(sol,arg1,arg2,prime,iprime);
}

void square(uint32 * sol, uint32 * arg, uint32 * prime, uint32 iprime)
{
  mult(sol,arg,arg,prime,iprime);
}

void add(uint32 * sol, uint32 * arg1, uint32 * arg2, uint32 * prime, uint32 iprime)
{
  if(sol!=arg1)
    suma3(sol,arg1,arg2);
  else
    suma(sol,arg2);
  while(esmayorIgual(sol,prime))
    resta(sol,prime);
}

void sub(uint32 * sol, uint32 * arg1, uint32 * arg2, uint32 * prime, uint32 iprime)
{
  uint32 arg2n[5];
  copy(arg2n,prime);
  resta(arg2n,arg2);
  add(sol,arg1,arg2n,prime,iprime);
}

void half(uint32 * sol, uint32 * arg, uint32 * prime, uint32 iprime)
{
  if(arg[0]%2 != 0)
    {
    add(sol,arg,prime,prime,iprime);    
    }
  else
    {
    copy(sol,arg);
    }
  int c0,c1,c2,c3;
  c0 = sol[0] % 2; 
  c1 = sol[1] % 2;
  c2 = sol[2] % 2;
  c3 = sol[3] % 2;
  sol[0] >>= 1;
  sol[1] >>= 1;
  sol[2] >>= 1;
  sol[3] >>= 1;
  sol[4] >>= 1;
  if(c0) sol[1]+= 0x80000000;
  if(c1) sol[2]+= 0x80000000;
  if(c2) sol[3]+= 0x80000000;
  if(c3) sol[4]+= 0x80000000;
}

void times(uint32 * sol, int t, uint32 * arg, uint32 * prime, uint32 iprime)
{
 if(t==2)
   add(sol,arg,arg,prime,iprime);
 else if(t==3)
   {
   uint32 temp[INTSPERKEY];
   add(temp,arg,arg,prime,iprime);
   add(sol,temp,arg,prime,iprime);
   }
}

void doubling(jacobianPoint * sol, jacobianPoint * P, uint32 * prime, uint32 iprime)
{
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.21, page 91. */
 if(infty(P))
   {
   sol->X[0] = 1;
   sol->X[1] = 0;
   sol->X[2] = 0;
   sol->X[3] = 0;
   sol->X[4] = 0;
   sol->Y[0] = 1;
   sol->Y[1] = 0;
   sol->Y[2] = 0;
   sol->Y[3] = 0;
   sol->Y[4] = 0;
   sol->Z[0] = 0;
   sol->Z[1] = 0;
   sol->Z[2] = 0;
   sol->Z[3] = 0;
   sol->Z[4] = 0;
   }
 else
   {
   uint32 T1[INTSPERKEY];
   uint32 T2[INTSPERKEY];
   uint32 T3[INTSPERKEY];

   square(T1,P->Z,prime,iprime);
   sub(T2,P->X,T1,prime,iprime);
   add(T1,P->X,T1,prime,iprime);
   add(T1,P->X,T1,prime,iprime);
   mult(T2,T2,T1,prime,iprime);
   times(T2,3,T2,prime,iprime);
   times(sol->Y,2,P->Y,prime,iprime);
   mult(sol->Z,sol->Y,P->Z,prime,iprime);
   square(sol->Y,sol->Y,prime,iprime);
   mult(T3,sol->Y,P->X,prime,iprime);
   square(sol->Y,sol->Y,prime,iprime);
   half(sol->Y,sol->Y,prime,iprime);
   square(sol->X,T2,prime,iprime);
   times(T1,2,T3,prime,iprime);
   sub(sol->X,sol->X,T1,prime,iprime);
   sub(T1,T3,sol->X,prime,iprime);
   mult(T1,T1,T2,prime,iprime);
   sub(sol->Y,T1,sol->Y,prime,iprime);
   }
}

void addition(jacobianPoint * sol, jacobianPoint * P, affinePoint * Q, uint32 * prime, uint32 iprime)
{
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.22, page 91. */
 if(infty(P))
   {
   copy(sol->X,Q->x);
   copy(sol->Y,Q->y);
   sol->Z[0]=1;
   sol->Z[1]=0;
   sol->Z[2]=0;
   sol->Z[3]=0;
   sol->Z[4]=0;
   }
 else
   {
   uint32 T1[INTSPERKEY];
   uint32 T2[INTSPERKEY];
   uint32 T3[INTSPERKEY];
   uint32 T4[INTSPERKEY];
   square(T1,P->Z,prime,iprime);
   mult(T2,T1,P->Z,prime,iprime);
   mult(T1,T1,Q->x,prime,iprime);
   mult(T2,T2,Q->y,prime,iprime);
   sub(T1,T1,P->X,prime,iprime);
   sub(T2,T2,P->Y,prime,iprime);
   if(iszero(T1))
     {
     if(iszero(T2))
       {
       toJacobian(sol,Q);
       doubling(sol,sol,prime,iprime);
       }
     else
       {
       sol->X[0] = 1;
       sol->X[1] = 0;
       sol->X[2] = 0;
       sol->X[3] = 0;
       sol->X[4] = 0;
       sol->Y[0] = 1;
       sol->Y[1] = 0;
       sol->Y[2] = 0;
       sol->Y[3] = 0;
       sol->Y[4] = 0;
       sol->Z[0] = 0;
       sol->Z[1] = 0;
       sol->Z[2] = 0;
       sol->Z[3] = 0;
       sol->Z[4] = 0;
       }
     }
   else
     {
     mult(sol->Z,P->Z,T1,prime,iprime);
     square(T2,T1,prime,iprime);
     mult(T4,T3,T1,prime,iprime);
     mult(T3,T3,P->X,prime,iprime);
     times(T1,2,T3,prime,iprime);
     sub(sol->X,sol->X,T1,prime,iprime);
     sub(sol->X,sol->X,T4,prime,iprime);
     sub(T3,T3,sol->X,prime,iprime);
     mult(T3,T3,T2,prime,iprime);
     mult(T4,T4,P->Y,prime,iprime);
     sub(sol->Y,T3,T4,prime,iprime);
     }
   }
}

void powering(jacobianPoint * sol, affinePoint * base, uint32 * exp, int sizeExp, uint32 * prime, uint32 iprime)
{
/* Hankerson, D.; Menezes, A.; Vanstone, S.: Guide to Elliptic Curve
Cryptography, Springer 2004. Algorithm 3.27, page 97. */
  int j;
 uint32 mask;
 sol->X[0] = 1;
 sol->X[1] = 0;
 sol->X[2] = 0;
 sol->X[3] = 0;
 sol->X[4] = 0;
 sol->Y[0] = 1;
 sol->Y[1] = 0;
 sol->Y[2] = 0;
 sol->Y[3] = 0;
 sol->Y[4] = 0;
 sol->Z[0] = 0;
 sol->Z[1] = 0;
 sol->Z[2] = 0;
 sol->Z[3] = 0;  
 sol->Z[4] = 0;

 for(j=sizeExp-1;j>=0;j--)
   {
   for(mask=0x80000000; mask!=0; mask>>=1)
     {
//PRINTF("Cycle in powering %d, Mask: %d\n", j, mask);

     doubling(sol,sol,prime,iprime);
     if((mask & exp[j])!=0)
        addition(sol,sol,base,prime,iprime);
     }
   }
}


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

PRINTF("Start\n");
uint16 cycle = 0;
  while(1)
  {
PRINTF("Cycle %d\n", cycle++);
  uint32 prime[5];
  uint32 iPrime;
  affinePoint P;
  jacobianPoint Res;
  uint32 exp[5];
  prime[0] = 0x2a26a579;
  prime[1] = 0xb1c2675b;
  prime[2] = 0x8db3b6e4;
  prime[3] = 0x580e0278;
  prime[4] = 0x8c414514;
  iPrime = invuint32(prime[0]);
  powering(&Res,&P,exp,5,prime,iPrime);

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
