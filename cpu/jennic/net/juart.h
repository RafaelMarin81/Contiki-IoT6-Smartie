#ifndef JUART_H_INCLUDED
#define JUART_H_INCLUDED

/****************************************************************************
 *
 * MODULE:             JUART Header
 *
 * COMPONENT:          $RCSfile: juart.h,v $
 *
 * DATED:              $Date: 2008/12/02 17:46:00 $
 *
 * AUTHOR:             Antonio Jara
 *
 * DESCRIPTION:
 * UART use header file
 *
 ****/

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h> //Necesario para extender el ANSI C con los tipos que aqui usamos como el bool_t

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vUART_printInit(void);
PUBLIC void vUART_DataInit(void);

PUBLIC void vPrintf(const char *fmt, ...);
PUBLIC void vPutC_UART0(unsigned char c);
PUBLIC uint8 uGetC_UART0(void);
PUBLIC bool_t bAvaibleC_UART0(void);

PUBLIC void vPutC_UART1(unsigned char c);
PUBLIC uint8 uGetC_UART1(void);
PUBLIC bool_t bAvaibleC_UART1(void);

#if defined __cplusplus
}
#endif


#endif // JUART_H_INCLUDED
