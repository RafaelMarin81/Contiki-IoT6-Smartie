#define BITS_KEY 160
#define BITS_INTEGER 32
#define INTSPERKEY 5

typedef struct jacobianPoint_struct{
 uint32 X[INTSPERKEY];
 uint32 Y[INTSPERKEY];
 uint32 Z[INTSPERKEY];
}jacobianPoint;

typedef struct affinePoint_struct{
 uint32 x[INTSPERKEY];
 uint32 y[INTSPERKEY];
}affinePoint;


#define PRIME4 0xFFFFFFFF
#define PRIME3 0xFFFFFFFF
#define PRIME2 0xFFFFFFFF
#define PRIME1 0xFFFFFFFF
#define PRIME0 0x7FFFFFFF 
#define ONE4 0x0
#define ONE3 0x0
#define ONE2 0x0
#define ONE1 0x0
#define ONE0 0x80000001

uint32 prime[5];
uint32 iPrime;
affinePoint G;

#define isequal(a,b) (a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && \
a[3]==b[3] && a[4]==b[4])
#define copy(d,o) d[0]=o[0]; d[1]=o[1]; d[2]=o[2]; \
d[3]=o[3]; d[4]=o[4];
#define notequal(a,b) (a[0]!=b[0] || a[1]!=b[1] || a[2]!=b[2] || \
a[3]!=b[3] || a[4]!=b[4])
#define iszero(a) (a[0]==0 && a[1]==0 && a[2]==0 && \
a[3]==0 && a[4]==0)



#define infty(P) (P->X[0] == ONE0 && P->X[1] == ONE1 && P->X[2] == ONE2 && P->X[3] == ONE3 && P->X[4] == ONE4 && \
P->Y[0] == ONE0 && P->Y[1] == ONE1 && P->Y[2] == ONE2 && P->Y[3] == ONE3 && P->Y[4] == ONE4 && \
P->Z[0] == 0 && P->Z[1] == 0 && P->Z[2] == 0 && P->Z[3] == 0 && P->Z[4] == 0)
#define setInfty(P) { P->X[0] = ONE0; P->X[1] = ONE1; P->X[2] = ONE2; P->X[3] = ONE3; P->X[4] = ONE4; \
P->Y[0] = ONE0; P->Y[1] = ONE1; P->Y[2] = ONE2; P->Y[3] = ONE3; P->Y[4] = ONE4; \
P->Z[0] = 0; P->Z[1] = 0; P->Z[2] = 0; P->Z[3] = 0; P->Z[4] = 0; }
#define toJacobian(PP,P) { PP->X[0] = P->x[0]; PP->X[1] = P->x[1]; PP->X[2] = P->x[2]; PP->X[3] = P->x[3]; PP->X[4] = P->x[4]; \
PP->Y[0] = P->y[0]; PP->Y[1] = P->y[1]; PP->Y[2] = P->y[2]; PP->Y[3] = P->y[3]; PP->Y[4] = P->y[4]; \
PP->Z[0] = ONE0; PP->Z[1] = ONE1; PP->Z[2] = ONE2; PP->Z[3] = ONE3; PP->Z[4] = ONE4; }



