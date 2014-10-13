
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "juart.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) vPrintf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define abso(a) ( a>=0?a:-a )


struct WavePeak {
	unsigned char ini;
	unsigned char end;
	char height;
	unsigned char peakPosition;
	char iniH;
	char endH;
};

struct CompressedWave {
	char alturaP;
	char alturaQ;
	char alturaR;
	char alturaS;
	char alturaT;
	char segmentoP;
	char segmentoPQ;
	char segmentoQS;
	char segmentoST;
	char segmentoT;
	unsigned char bpm;
	unsigned char spo2;
	char diag;
};

/*---------------------------------------------------------------------------*/
/*---------------------------Yoapy Functions---------------------------------*/
/*---------------------------------------------------------------------------*/
//devuelve verdadero si el primero es menor que el segundo
int compareWavePeaksHeight(const struct WavePeak w1, const struct WavePeak w2)
{
  if(abso(w1.height) > abso(w2.height)) {
    return 0;
  } else if(abso(w1.height) < abso(w2.height)) {
    return 1;
  } else {
    if(abso(w1.iniH) > abso(w2.iniH)) {
      return 0;
    } else if(abso(w1.iniH) < abso(w2.iniH)) {
      return 1;
    } else {
      return 1;
    }
  }
}
/*---------------------------------------------------------------------------*/
//devuelve verdadero si el primero es menor que el segundo
int compareWavePeaksPosition(const struct WavePeak w1, const struct WavePeak w2){
    if(w1.peakPosition > w2.peakPosition){
        return 0;
    }else{
        return 1;
    }
}
/*---------------------------------------------------------------------------*/
//Ordena de mayor a menor
int partition(struct WavePeak a[], int l, int r)
{
  struct WavePeak pivot, t;
  int i, j;
  PRINTF("Entramos patition\n");
  pivot = a[l];
  i = l;
  j = r + 1;
  
  while(1) {
    PRINTF("A dar weltas\n");
    do {
      ++i;
    } while(!compareWavePeaksHeight(a[i], pivot) && i > r);
    do {
      --j;
    } while(compareWavePeaksHeight(a[j], pivot));
    if(i >= j)
      break;
    t = a[i];
    a[i] = a[j];
    a[j] = t;

  }
  PRINTF("1\n");
  t = a[l];
  PRINTF("2\n");
  a[l] = a[j];
  PRINTF("3\n");
  a[j] = t;
  PRINTF("salgo partition\n");
  return j;
}
/*---------------------------------------------------------------------------*/
//Ordena de menor a mayor
int partitionPosition(struct WavePeak a[], int l, int r)
{
  struct WavePeak pivot, t;
  int i, j;
  PRINTF("Entramos patition position\n");
  pivot = a[l];
  i = l;
  j = r + 1;
  
  while(1) {
    do
      ++i;
    while(compareWavePeaksPosition(a[i], pivot) && i <= r);
    do
      --j;
    while(!compareWavePeaksPosition(a[j], pivot));
    if(i >= j)
      break;
    t = a[i];
    a[i] = a[j];
    a[j] = t;
    
  }
  
  t = a[l];
  a[l] = a[j];
  a[j] = t;
  PRINTF("salimos patition position\n");
  return j;
}
/*---------------------------------------------------------------------------*/
void quickSort( struct WavePeak a[], int l, int r, int typeSort)
{
  int j;
  PRINTF("empieza qquick\n");
  if(l < r) {
    // divide and conquer
    if(typeSort)
      j = partition(a, l, r);
    else
      j = partitionPosition(a, l, r);
    quickSort(a, l, j - 1, typeSort);
    quickSort(a, j + 1, r, typeSort);
  }
  PRINTF("termina qquick\n");
}
/*---------------------------------------------------------------------------*/
int mWithXContinua(int y1, int y2) {
    return y2 - y1;
}
/*---------------------------------------------------------------------------*/
int m(int x1, int y1, int x2, int y2) {
    return mWithXContinua(y1, y2) / (x2 - x1);
}
/*---------------------------------------------------------------------------*/
/* Burbuja
 * 
 */
void burbuja(struct WavePeak array[], int N, int typeSort)
{
  int i, j = 0;
  struct WavePeak aux;
  
  for(i = 0; i < N; i++) { // Hacer N-1 pasadas.
    for(j = 0; j < N - i; j++) { // Mirar los N-i-1 pares.
      if(typeSort) {
        if(!compareWavePeaksHeight(array[j + 1], array[j])) { // Si el elemento j+1 es menor que el elemento j:          
          aux = array[j + 1]; // Se intercambian los elementos
          array[j + 1] = array[j]; // de las posiciones j y j+1
          array[j] = aux; // usando una variable auxiliar.
        }
      } else {
        if(compareWavePeaksPosition(array[j + 1], array[j])) { // Si el elemento j+1 es menor que el elemento j:          
          aux = array[j + 1]; // Se intercambian los elementos
          array[j + 1] = array[j]; // de las posiciones j y j+1
          array[j] = aux; // usando una variable auxiliar.
        }
      }
    }
  }

}

/*---------------------------------------------------------------------------*/
/*
	YOAPY algorithm 

	return a CompressedWave

*/
struct CompressedWave compressWave(int *hearthBeat, const int tam, const char bpm, const char spo2) {
    int ini = 0;
    int end = 0;
    int height = 0;
    int pos = 0;
    struct WavePeak peaks[5];
    int nWaves = 0;
    struct CompressedWave cwave;
    struct WavePeak wave;
    int k=0;
    int i=0;
    int iniQ =0;
    unsigned char aux = 0;
    for (i = 0; i < tam; i++) {
        if(nWaves == 5){
          //quickSort(peaks, 0, 4, 1);
          burbuja(peaks,5,1);
          nWaves++;				
        }
        /**
         * Buscamos ondas positivas en la trama
         */
        if ((i - 1) > 0 && (i + 1) < tam
                && ((hearthBeat[i - 1] < hearthBeat[i] && hearthBeat[i + 1] < hearthBeat[i])
                || (hearthBeat[i - 1] == hearthBeat[i] && hearthBeat[i + 1] < hearthBeat[i])
                || (hearthBeat[i - 1] < hearthBeat[i] && hearthBeat[i + 1] == hearthBeat[i]))
                ){//&& i < (tam*0.86)
                //&& i > (tam*0.25)) {
            
            //Buscamos el inicio y lo obtenemos
            for (k = i - 1; k > 0; k--) {
                if (mWithXContinua(hearthBeat[k], hearthBeat[k + 1]) <= 4 && (k < i - 1)) {
                    ini = k + 1;
                    break;
                }
            }
            //Buscamos el fin y lo obtenemos
            for (k = i + 1; k < tam; k++) {
                if (mWithXContinua(hearthBeat[k - 1], hearthBeat[k]) >= -4 && (k > i + 1)) {
                    end = k - 1;
                    break;
                }
                if (k + 1 == tam) {
                    end = k - 1;
                    break;
                }
            }
            PRINTF("entra por aqui2\n");
            //Obtenemos la posicion y la altura del pico
            pos = i;
            height = ((hearthBeat[pos] - hearthBeat[ini])>(hearthBeat[pos] - hearthBeat[end]) ? (hearthBeat[pos] - hearthBeat[ini]) : (hearthBeat[pos] - hearthBeat[end]));	 
            //if we have less than 5 wave we can store the wave to process later   	    
            if(nWaves < 5){		
                wave.ini = ini;
                wave.end = end;
                wave.iniH = (hearthBeat[pos] - hearthBeat[ini]);
                wave.endH = (hearthBeat[pos] - hearthBeat[end]);
                wave.height = height;
                wave.peakPosition = pos;
                peaks[nWaves] = wave;
                nWaves++;
            //if we have more than 5 waves we need to take only the best 5 waves
            }else if(nWaves == 6){
                wave.ini = ini;
                wave.end = end;
                wave.iniH = (hearthBeat[pos] - hearthBeat[ini]);
                wave.endH = (hearthBeat[pos] - hearthBeat[end]);
                wave.height = height;
                wave.peakPosition = pos;	
                if(!compareWavePeaksHeight(wave,peaks[4])){
                  peaks[4] = wave;
                  //quickSort(peaks, 0, 4, 1);
                  burbuja(peaks,5,1);
                }
            }

        } 
        PRINTF("entra por aqui3 %d\n",nWaves);
        if(nWaves == 5){
          continue;
        }

        /**
         * Buscamos ondas negativas en la trama
         */
        if ((i - 1) > 0 && (i + 1) < tam
                && ((hearthBeat[i - 1] > hearthBeat[i] && hearthBeat[i + 1] > hearthBeat[i])
                || (hearthBeat[i - 1] == hearthBeat[i] && hearthBeat[i + 1] > hearthBeat[i])
                || (hearthBeat[i - 1] > hearthBeat[i] && hearthBeat[i + 1] == hearthBeat[i]))
                ){//&& i < (tam*0.86)
                //&& i > (tam*0.25)) {
            
            //Buscamos el inicio y lo obtenemos
            for (k = i - 1; k > 0; k--) {
                if (mWithXContinua(hearthBeat[k], hearthBeat[k + 1]) >= -4 && (k < i - 1)) {
                    ini = k + 1;
                    break;
                }
            }
            PRINTF("entra por aqui4\n");
            //Buscamos el fin y lo obtenemos
            for (k = i + 1; k < tam; k++) {
                if (mWithXContinua(hearthBeat[k - 1], hearthBeat[k]) <= 4 && (k > i + 1)) {
                    end = k - 1;
                    break;
                }
            }
            //Obtenemos la posicion y la altura del pico
            pos = i;
            height = ((hearthBeat[pos] - hearthBeat[ini])<(hearthBeat[pos] - hearthBeat[end]) ? (hearthBeat[pos] - hearthBeat[ini]) : (hearthBeat[pos] - hearthBeat[end]));


           if(nWaves < 5){		
                wave.ini = ini;
                wave.end = end;
                wave.iniH = (hearthBeat[pos] - hearthBeat[ini]);
                wave.endH = (hearthBeat[pos] - hearthBeat[end]);
                wave.height = height;
                wave.peakPosition = pos;
                peaks[nWaves] = wave;
                nWaves++;
	
           }else if(nWaves == 6){
                wave.ini = ini;
                wave.end = end;
                wave.iniH = (hearthBeat[pos] - hearthBeat[ini]);
                wave.endH = (hearthBeat[pos] - hearthBeat[end]);
                wave.height = height;
                wave.peakPosition = pos;	
                PRINTF("entra por aqui7\n");
                if(!compareWavePeaksHeight(wave,peaks[4])){
                  peaks[4] = wave;
                  //quickSort(peaks, 0, 4, 1);
                  burbuja(peaks,5,1);
                }
	
          }
		
	    
        }
    }
    PRINTF("entra por aqui5\n");
    //if we have less than 5 waves, the compression has failed
    if(nWaves < 5){
        cwave.alturaP = 0;
        cwave.alturaQ = 0;
        cwave.alturaR = 0;
        cwave.alturaS = 0;
        cwave.alturaT = 0;
        cwave.segmentoP = 0;
        cwave.segmentoPQ = 0;
        cwave.segmentoQS = 0;
        cwave.segmentoST = 0;
        cwave.segmentoT = 0;
        cwave.bpm = 0;
        cwave.spo2= 0;
        cwave.diag = -1;
        return cwave;
    }
    //quickSort(peaks, 0, 4, 0);
    burbuja(peaks,5,0);
    PRINTF("entra por aqui6\n");
    //Obtaining compressed data
    wave = (peaks[0]);
    //Obtaining P
    cwave.alturaP = wave.height;
    cwave.segmentoP = wave.end - wave.ini;
    aux = wave.end;
    //Obtaining Q
    wave = (peaks[1]);
    iniQ=wave.ini;
    cwave.alturaQ = hearthBeat[wave.peakPosition] - hearthBeat[wave.ini];
    cwave.segmentoPQ = wave.ini - aux;
    aux = wave.ini;
    
    //Obtaining R
    wave = (peaks[2]);
    cwave.alturaR = hearthBeat[wave.peakPosition] - hearthBeat[iniQ];
    
    //Obtaining S
    wave = (peaks[3]);
    cwave.alturaS = hearthBeat[wave.peakPosition] - hearthBeat[iniQ];
    cwave.segmentoQS = wave.end - aux;
    aux = wave.end;
    //Obtaining T
    wave = (peaks[4]); 
    cwave.segmentoST = wave.ini - aux;
    cwave.alturaT = hearthBeat[wave.peakPosition] - hearthBeat[iniQ];
    cwave.segmentoT = wave.end - wave.ini;
    cwave.diag = 0;
    cwave.bpm = bpm;
    cwave.spo2 = spo2;
    return cwave;
}
/*---------------------------------------------------------------------------*/
void printData(struct CompressedWave cwave, int spo2, int bpm) {
      PRINTF("\n--------- Values to reconstruct ---------\n");
    
      PRINTF("\tP wave (maximum height): %d\n",(int)cwave.alturaP);
      PRINTF("\tQ wave (minimun height): %d\n",(int)cwave.alturaQ);
      PRINTF("\tR wave (maximum height): %d\n",(int)cwave.alturaR);
      PRINTF("\tS wave (minimun height): %d\n",(int)cwave.alturaS);
      PRINTF("\tT wave (maximum height): %d\n",(int)cwave.alturaT);
      PRINTF("\tLength of P segment: %d\n",(int)cwave.segmentoP);
      PRINTF("\tLength of PQ segment: %d\n",(int)cwave.segmentoPQ);
      PRINTF("\tLength of QS segment: %d\n",(int)cwave.segmentoQS);
      PRINTF("\tLength of ST segment: %d\n",(int)cwave.segmentoST);
      PRINTF("\tLength of T segment: %d\n\n",(int)cwave.segmentoT);
      PRINTF("\tBpm: %d, spo2: %d\n",spo2, bpm);
       
}
