
/****************************************************************************
 *
 * MODULE:             Printf Function
 *
 * COMPONENT:          $RCSfile: juart.c,v $
 *
 * AUTHOR:             Antonio Jara
 *
 * DESCRIPTION:
 * Code to provide simple UART functions
 *
 ****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "contiki.h"
#include "juart.h"
#include <AppHardwareApi.h>


PUBLIC void vUART_Init_UART0(void);
PUBLIC void vUART_Init_UART1(void);

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#define UART                    E_AHI_UART_1
#define UART_BAUD_RATE          9600

#define RX_QUEUE 0                 
#define TX_QUEUE 1

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#if UART == E_AHI_UART_0
    #define UART_START_ADR  	0x30000000UL
#else
    #define UART_START_ADR  	0x40000000UL
#endif

#define UART_LCR_OFFSET 	0x0C
#define UART_DLM_OFFSET 	0x04
#define CIRCBUFF_PTR_MASK   0x03FFU
#define MAX_CIRCBUFF_SIZE   256 //Ponia 1024
#define NBR_QUEUES          2


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef struct
{
    uint16 u16Head;
    uint16 u16Tail;
    uint8  u8Buff[MAX_CIRCBUFF_SIZE];
} tsCircBuff;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE tsCircBuff sRxQueue, sTxQueue;
PRIVATE const tsCircBuff *apsQueueList[NBR_QUEUES] = { &sRxQueue, &sTxQueue };

PRIVATE void vNum2String(uint32 u32Number, uint8 u8Base);
PRIVATE void vNum2StringUART0(uint32 u32Number, uint8 u8Base);
PRIVATE void vInteger2StringUART0(uint32 s32Number, uint8 u8Base);
PRIVATE void vUART_Init(void);
PRIVATE void vUART_StartTx(void);
PRIVATE void vUART_RxCharISR(uint8 u8RxChar);
PRIVATE void vUART_TxCharISR(void);
PRIVATE void   vSerialQ_Init(void);
PRIVATE bool_t bSerialQ_Full(int eQueue);
PRIVATE bool_t bSerialQ_Empty(int eQueue);
PRIVATE uint8  u8SerialQ_RemoveItem(int eQueue);
PRIVATE void   vSerialQ_AddItem(int eQueue, uint8 u8Item);

/* pointer to whatever putchar function the user gives us */
PRIVATE void (*vPutChar0) (char c) = NULL;
PRIVATE void (*vPutChar1) (char c) = NULL;
PRIVATE void vUART_SetBuadRate(uint32 u32BaudRate);
PRIVATE void vSerialQ_Flush(int eQueue);


PRIVATE void vSerialQ_Init(void)
{
    vSerialQ_Flush(RX_QUEUE);
    vSerialQ_Flush(TX_QUEUE);
}


PRIVATE void vSerialQ_AddItem(int eQueue, uint8 u8Item)
{
    tsCircBuff *psQueue;
    uint16 u16NextLocation;

    psQueue = (tsCircBuff *)apsQueueList[eQueue]; /* Set pointer to the requested queue */

    u16NextLocation = (psQueue->u16Head + 1) & CIRCBUFF_PTR_MASK;

    if (u16NextLocation != psQueue->u16Tail)
    {
        /* Space available in buffer so add data */
        psQueue->u8Buff[psQueue->u16Head] = u8Item;
        psQueue->u16Head = u16NextLocation;
    }
}


PRIVATE uint8 u8SerialQ_RemoveItem(int eQueue)
{
    uint8 u8Item = 0;
    tsCircBuff *psQueue;

    psQueue = (tsCircBuff *)apsQueueList[eQueue]; /* Set pointer to the requested queue */

    if (psQueue->u16Tail != psQueue->u16Head)
    {
        /* Data available on queue so remove a single item */
        u8Item = psQueue->u8Buff[psQueue->u16Tail];
        psQueue->u16Tail = (psQueue->u16Tail + 1) & CIRCBUFF_PTR_MASK;
    }
    return(u8Item);
}


PRIVATE bool_t bSerialQ_Empty(int eQueue)
{
    bool_t bResult = FALSE;
    tsCircBuff *psQueue;

    psQueue = (tsCircBuff *)apsQueueList[eQueue];

    if (psQueue->u16Tail == psQueue->u16Head)
    {
        bResult = TRUE;
    }
    return(bResult);
}

PRIVATE bool_t bSerialQ_Full(int eQueue)
{
    bool_t bResult = FALSE;
    tsCircBuff *psQueue;
    uint16 u16NextLocation;

    psQueue = (tsCircBuff *)apsQueueList[eQueue];

    u16NextLocation = (psQueue->u16Head + 1) & CIRCBUFF_PTR_MASK;

    if (u16NextLocation == psQueue->u16Tail)
    {
	    bResult = TRUE;
    }
    return(bResult);
}

PRIVATE void vSerialQ_Flush(int eQueue)
{
    tsCircBuff *psQueue;

    psQueue = (tsCircBuff *)apsQueueList[eQueue]; /* Set pointer to the requested queue */

    psQueue->u16Head  = 0;
    psQueue->u16Tail  = 0;
}

////////////////////////////////////////////////////////////////

#if UART == E_AHI_UART_0
PRIVATE void vUART_HandleUart0Interrupt(uint32 u32Device, uint32 u32ItemBitmap);
#else
PRIVATE void vUART_HandleUart1Interrupt(uint32 u32Device, uint32 u32ItemBitmap);
#endif

PRIVATE void vUART_Init(void)
{
    /* Enable UART 1 */
    vAHI_UartEnable(UART);

    vAHI_UartReset(UART, TRUE, TRUE);
    vAHI_UartReset(UART, FALSE, FALSE);

    /* Register function that will handle UART interrupts */
    #if UART == E_AHI_UART_0
        vAHI_Uart0RegisterCallback(vUART_HandleUart0Interrupt);
    #else
        vAHI_Uart1RegisterCallback(vUART_HandleUart1Interrupt);
    #endif

    /* Set the clock divisor register to give required buad, this has to be done
       directly as the normal routines (in ROM) do not support all baud rates */
    vUART_SetBuadRate(UART_BAUD_RATE);

    vAHI_UartSetControl(UART, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, FALSE);

    vAHI_UartSetInterrupt(UART, FALSE, FALSE, TRUE, TRUE, E_AHI_UART_FIFO_LEVEL_1);
}

PRIVATE void vUART_SetBuadRate(uint32 u32BaudRate)
{
    uint8 *pu8Reg;
    uint8  u8TempLcr;
    uint16 u16Divisor;
    uint32 u32Remainder;

    /* Put UART into clock divisor setting mode */
    pu8Reg    = (uint8 *)(UART_START_ADR + UART_LCR_OFFSET);
    u8TempLcr = *pu8Reg;
    *pu8Reg   = u8TempLcr | 0x80;

    /* Write to divisor registers:
       Divisor register = 16MHz / (16 x baud rate) */
    u16Divisor = (uint16)(16000000UL / (16UL * u32BaudRate));

    /* Correct for rounding errors */
    u32Remainder = (uint32)(16000000UL % (16UL * u32BaudRate));

    if (u32Remainder >= ((16UL * u32BaudRate) / 2))
    {
        u16Divisor += 1;
    }

    pu8Reg  = (uint8 *)UART_START_ADR;
    *pu8Reg = (uint8)(u16Divisor & 0xFF);
    pu8Reg  = (uint8 *)(UART_START_ADR + UART_DLM_OFFSET);
    *pu8Reg = (uint8)(u16Divisor >> 8);

    /* Put back into normal mode */
    pu8Reg    = (uint8 *)(UART_START_ADR + UART_LCR_OFFSET);
    u8TempLcr = *pu8Reg;
    *pu8Reg   = u8TempLcr & 0x7F;
}


PRIVATE void vUART_StartTx(void)
{
    /* Has interrupt driven transmit stalled (tx fifo is empty) */
    if (u8AHI_UartReadLineStatus(UART) & E_AHI_UART_LS_THRE)
    {
        if(!bSerialQ_Empty(TX_QUEUE))
        {
            vAHI_UartWriteData(UART, u8SerialQ_RemoveItem(TX_QUEUE));
        }
    }
}

PRIVATE void vUART_TxCharISR(void)
{
    if(!bSerialQ_Empty(TX_QUEUE))
	{
        vAHI_UartWriteData(UART, u8SerialQ_RemoveItem(TX_QUEUE));
	}
}


PRIVATE void vUART_RxCharISR(uint8 u8RxChar)
{
    vSerialQ_AddItem(RX_QUEUE, u8RxChar);
}

#if UART == E_AHI_UART_0
PRIVATE void vUART_HandleUart0Interrupt(uint32 u32Device, uint32 u32ItemBitmap)
{
    if (u32Device == E_AHI_DEVICE_UART0)
    {
        if ((u32ItemBitmap & 0x000000FF) == E_AHI_UART_INT_RXDATA)
        {
            vSerialQ_AddItem(RX_QUEUE, u8AHI_UartReadData(E_AHI_UART_0));
            //vUART_RxCharISR(u8AHI_UartReadData(E_AHI_UART_0));
        }
        else if (u32ItemBitmap == E_AHI_UART_INT_TX)
        {
            vUART_TxCharISR();
        }
    }
}
#else
PRIVATE void vUART_HandleUart1Interrupt(uint32 u32Device, uint32 u32ItemBitmap)
{
    if (u32Device == E_AHI_DEVICE_UART1)
    {
        if ((u32ItemBitmap & 0x000000FF) == E_AHI_UART_INT_RXDATA)
        {
            vUART_RxCharISR(u8AHI_UartReadData(E_AHI_UART_1));
        }
        else if (u32ItemBitmap == E_AHI_UART_INT_TX)
        {
            vUART_TxCharISR();
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC void vInitPrintf(void (*fp)(char c))
{
    vPutChar0 = fp;
}

PUBLIC void vInitData(void (*fp)(char c))
{
    vPutChar1 = fp;
}

/*
 * printf()
 *  Print to display - really trivial impelementation!
 */


PUBLIC void vPrintf(const char *fmt, ...){
    char *bp = (char *)fmt;
    va_list ap;
    char c;
    char *p;
    int32 i;

    va_start(ap, fmt);

    while ((c = *bp++)) {
        if (c != '%') {
            if (c == '\n'){
                vPutChar0('\n');
                vPutChar0('\r');
            } else {
                vPutChar0(c);
            }
            continue;
        }

        switch ((c = *bp++)) {
        
	case 'p':
		vInteger2StringUART0(va_arg(ap, uint32),10);		
		break;
	/* %d - show a decimal value */
        case 'd':
            vNum2StringUART0(va_arg(ap, uint32), 10);
            break;

        /* %x - show a value in hex */
        case 'x':
            vPutChar0('0');
            vPutChar0('x');
            vNum2StringUART0(va_arg(ap, uint32), 16);
            break;

        /* %b - show a value in binary */
        case 'b':
            vPutChar0('0');
            vPutChar0('b');
            vNum2StringUART0(va_arg(ap, uint32), 2);
            break;

        /* %c - show a character */
        case 'c':
            vPutChar1(va_arg(ap, int));
            break;

        case 'i':
            i = va_arg(ap, int32);
            if(i < 0){
                vPutChar0('-');
                vNum2StringUART0((~i)+1, 10);
            } else {
                vNum2StringUART0(i, 10);
            }
            break;

        /* %s - show a string */
        case 's':
            p = va_arg(ap, char *);
            do {
                vPutChar0(*p++);
            } while (*p);
            break;

        /* %% - show a % character */
        case '%':
            vPutChar0('%');
            break;

        /* %something else not handled ! */
        default:
            vPutChar0('?');
            break;

        }
    }

    return;
}

/****************************************************************************/
/***  Functions To Allow Printf To Work Via The UART                      ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: vUART_printInit
 *
 * DESCRIPTION:
 * Initialises the UART print environment
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/

PUBLIC void vUART_printInit(void)
{
    vInitPrintf((void*)vPutC_UART0);
    vUART_Init_UART0();
}

PUBLIC void vUART_DataInit(void)
{
    vInitData((void*)vPutC_UART1);
    vUART_Init_UART1();
}


/****************************************************************************
 *
 * NAME: vUART_Init
 *
 * DESCRIPTION:
 * Initialises the UART
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
/** CONFIGURACIÓN UART - CONFIG UART */
PUBLIC void vUART_Init_UART1(void) //PRINT UART
{
    /* Enable UART 1: 9600-8-N-1 */
    vAHI_UartEnable(E_AHI_UART_1);

    vAHI_UartReset(E_AHI_UART_1, TRUE, TRUE);
    vAHI_UartReset(E_AHI_UART_1, FALSE, FALSE);

    /**
    CONSTANTES QUE TENEMOS PARA CONFIGURAR LA UART (from AppHardwareAPI.h
    #define E_AHI_UART_RATE_4800       0
    #define E_AHI_UART_RATE_9600       1
    #define E_AHI_UART_RATE_19200      2
    #define E_AHI_UART_RATE_38400      3
    #define E_AHI_UART_RATE_76800      4
    #define E_AHI_UART_RATE_115200     5
    #define E_AHI_UART_WORD_LEN_5      0
    #define E_AHI_UART_WORD_LEN_6      1
    #define E_AHI_UART_WORD_LEN_7      2
    #define E_AHI_UART_WORD_LEN_8      3
    #define E_AHI_UART_FIFO_LEVEL_1    0
    #define E_AHI_UART_FIFO_LEVEL_4    1
    #define E_AHI_UART_FIFO_LEVEL_8    2
    #define E_AHI_UART_FIFO_LEVEL_14   3
    #define E_AHI_UART_LS_ERROR        0x80
    #define E_AHI_UART_LS_TEMT         0x40
    #define E_AHI_UART_LS_THRE         0x20
    #define E_AHI_UART_LS_BI           0x10
    #define E_AHI_UART_LS_FE           0x08
    #define E_AHI_UART_LS_PE           0x04
    #define E_AHI_UART_LS_OE           0x02
    #define E_AHI_UART_LS_DR           0x01
    #define E_AHI_UART_MS_DCTS         0x01
    #define E_AHI_UART_INT_MODEM       0
    #define E_AHI_UART_INT_TX          1
    #define E_AHI_UART_INT_RXDATA      2
    #define E_AHI_UART_INT_RXLINE      3
    #define E_AHI_UART_INT_TIMEOUT     6
    #define E_AHI_UART_TX_RESET        TRUE
    #define E_AHI_UART_RX_RESET        TRUE
    #define E_AHI_UART_TX_ENABLE       FALSE
    #define E_AHI_UART_RX_ENABLE       FALSE
    #define E_AHI_UART_EVEN_PARITY     TRUE
    #define E_AHI_UART_ODD_PARITY      FALSE
    #define E_AHI_UART_PARITY_ENABLE   TRUE
    #define E_AHI_UART_PARITY_DISABLE  FALSE
    #define E_AHI_UART_1_STOP_BIT      TRUE
    #define E_AHI_UART_2_STOP_BITS     TRUE
    #define E_AHI_UART_RTS_HIGH        TRUE
    #define E_AHI_UART_RTS_LOW         FALSE
    */

    vAHI_UartSetClockDivisor(E_AHI_UART_1, E_AHI_UART_RATE_9600);

    /**
    PUBLIC void   vAHI_UartSetControl(uint8 u8Uart, bool_t bEvenParity, bool_t bEnableParity,
                                  uint8 u8WordLength, bool_t bOneStopBit, bool_t bRtsValue);
    */
    vAHI_UartSetControl(E_AHI_UART_1, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, FALSE);
}


PUBLIC void vUART_Init_UART0(void) //DATA UART
{
    /* Enable UART 0: 9600-8-N-1 */
    vAHI_UartEnable(E_AHI_UART_0);

    vAHI_UartReset(E_AHI_UART_0, TRUE, TRUE);
    vAHI_UartReset(E_AHI_UART_0, FALSE, FALSE);

    /**
    CONSTANTES QUE TENEMOS PARA CONFIGURAR LA UART (from AppHardwareAPI.h
    #define E_AHI_UART_RATE_4800       0
    #define E_AHI_UART_RATE_9600       1
    #define E_AHI_UART_RATE_19200      2
    #define E_AHI_UART_RATE_38400      3
    #define E_AHI_UART_RATE_76800      4
    #define E_AHI_UART_RATE_115200     5
    #define E_AHI_UART_WORD_LEN_5      0
    #define E_AHI_UART_WORD_LEN_6      1
    #define E_AHI_UART_WORD_LEN_7      2
    #define E_AHI_UART_WORD_LEN_8      3
    #define E_AHI_UART_FIFO_LEVEL_1    0
    #define E_AHI_UART_FIFO_LEVEL_4    1
    #define E_AHI_UART_FIFO_LEVEL_8    2
    #define E_AHI_UART_FIFO_LEVEL_14   3
    #define E_AHI_UART_LS_ERROR        0x80
    #define E_AHI_UART_LS_TEMT         0x40
    #define E_AHI_UART_LS_THRE         0x20
    #define E_AHI_UART_LS_BI           0x10
    #define E_AHI_UART_LS_FE           0x08
    #define E_AHI_UART_LS_PE           0x04
    #define E_AHI_UART_LS_OE           0x02
    #define E_AHI_UART_LS_DR           0x01
    #define E_AHI_UART_MS_DCTS         0x01
    #define E_AHI_UART_INT_MODEM       0
    #define E_AHI_UART_INT_TX          1
    #define E_AHI_UART_INT_RXDATA      2
    #define E_AHI_UART_INT_RXLINE      3
    #define E_AHI_UART_INT_TIMEOUT     6
    #define E_AHI_UART_TX_RESET        TRUE
    #define E_AHI_UART_RX_RESET        TRUE
    #define E_AHI_UART_TX_ENABLE       FALSE
    #define E_AHI_UART_RX_ENABLE       FALSE
    #define E_AHI_UART_EVEN_PARITY     TRUE
    #define E_AHI_UART_ODD_PARITY      FALSE
    #define E_AHI_UART_PARITY_ENABLE   TRUE
    #define E_AHI_UART_PARITY_DISABLE  FALSE
    #define E_AHI_UART_1_STOP_BIT      TRUE
    #define E_AHI_UART_2_STOP_BITS     TRUE
    #define E_AHI_UART_RTS_HIGH        TRUE
    #define E_AHI_UART_RTS_LOW         FALSE
    */

    vAHI_UartSetClockDivisor(E_AHI_UART_0, E_AHI_UART_RATE_38400);

    /**
    PUBLIC void   vAHI_UartSetControl(uint8 u8Uart, bool_t bEvenParity, bool_t bEnableParity,
                                  uint8 u8WordLength, bool_t bOneStopBit, bool_t bRtsValue);
    */
    //vAHI_UartSetControl(E_AHI_UART_0, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, E_AHI_UART_RTS_HIGH );
    vAHI_UartSetControl(E_AHI_UART_0, FALSE, FALSE, E_AHI_UART_WORD_LEN_8, TRUE, FALSE);

}

PUBLIC uint8 uGetC_UART0(void)
{
    uint8 u8RxChar =0;

    /** wait for somthing in rx fifo - Esperamos a que haya algo en la cola fifo de la entrada RX*/
    while ((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_DR  ) == 0);
    u8RxChar = u8AHI_UartReadData(E_AHI_UART_0);

    return u8RxChar;
}

PUBLIC uint8 uGetC_UART1(void)
{
    uint8 u8RxChar =0;

    /** wait for somthing in rx fifo - Esperamos a que haya algo en la cola fifo de la entrada RX*/
    while ((u8AHI_UartReadLineStatus(E_AHI_UART_1) & E_AHI_UART_LS_DR  ) == 0);
    u8RxChar = u8AHI_UartReadData(E_AHI_UART_1);

    return u8RxChar;
}

PUBLIC bool_t bAvaibleC_UART0(void)
{
    return !((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_DR  ) == 0);
}

PUBLIC bool_t bAvaibleC_UART1(void)
{
    return !((u8AHI_UartReadLineStatus(E_AHI_UART_1) & E_AHI_UART_LS_DR  ) == 0);
}

PUBLIC void vPutC_UART0(unsigned char c)
{


    //wait for tx fifo empty (bit 5 set in LSR when empty)
    /**Espera a que la UART este libre*/
    while ((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_THRE ) == 0);
    // ..and send the character
    vAHI_UartWriteData(E_AHI_UART_0,c);



}

PUBLIC void vPutC_UART1(unsigned char c)
{
    //wait for tx fifo empty (bit 5 set in LSR when empty)
    /**Espera a que la UART este libre*/
    while ((u8AHI_UartReadLineStatus(E_AHI_UART_1) & E_AHI_UART_LS_THRE ) == 0);
    // ..and send the character
    vAHI_UartWriteData(E_AHI_UART_1,c);

}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/*
 * vNum2String()
 *  Convert a number to a string
 */
PRIVATE void vNum2String(uint32 u32Number, uint8 u8Base)
{
    char buf[33];
    char *p = buf + 33;
    uint32 c, n;

    *--p = '\0';
    do {
        n = u32Number / u8Base;
        c = u32Number - (n * u8Base);
        if (c < 10) {
            *--p = '0' + c;
        } else {
            *--p = 'a' + (c - 10);
        }
        u32Number /= u8Base;
    } while (u32Number != 0);

    while (*p){
        vPutChar1(*p);
        p++;
    }

    return;
}

/*
 * vNum2String()
 *  Convert a number to a string
 */
PRIVATE void vNum2StringUART0(uint32 u32Number, uint8 u8Base)
{
    char buf[33];
    char *p = buf + 33;
    uint32 c, n;

    *--p = '\0';
    do {
        n = u32Number / u8Base;
        c = u32Number - (n * u8Base);
        if (c < 10) {
            *--p = '0' + c;
        } else {
            *--p = 'a' + (c - 10);
        }
        u32Number /= u8Base;
    } while (u32Number != 0);

    while (*p){
        vPutChar0(*p);
        p++;
    }

    return;
}



PRIVATE void vInteger2StringUART0(uint32 s32Number, uint8 u8Base)
{
	char buf[33];
	char *p = buf+33;	
	uint32 c, n;
	int32 u32Number;
	
	u32Number = s32Number * -1;	
	u32Number++;
	*--p = '\0';
	do {
		n = u32Number / u8Base;
		c = u32Number - (n * u8Base);
		if (c < 10) {
		    *--p = '0' + c;
		} else {
		    *--p = 'a' + (c - 10);
		}
		u32Number /= u8Base;
	} while (u32Number != 0);
	if(p >= buf)
		*--p='-';
	while (*p){
		vPutChar0(*p);
		p++;
	}

	return;
}



/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

