/*
 * sht-sensor.h
 *  Library for managing the sensor SHT11 of temperature and humidity.
 *
 *  Created on: 11-may-2012
 *      Author: Rafael Marin Perez.
 */

#ifndef SHT11_H_
#define SHT11_H_

//#include "../BajoNivel/pins.h"
//#include "../BajoNivel/tipos.h"
#include "types.h"

// Constantes para saber si se ha recibido un ACK o un NACK
#define SHT11_NACK 	0
#define SHT11_ACK 	1

// Constante para indicar si se activa el calentador interno del sensor o no
// Sirve para verificar que el sensor funciona correctamente
#define SHT11_CALENTADOR_ON 	1
#define SHT11_CALENTADOR_OFF 	0

// Resolución para el sensor de temperatura y de humedad (por defecto)
// Usados en función SetMeasureResolution
#define SHT11_RESOLUCION_14_12_BITS 	0
#define SHT11_RESOLUCION_12_8_BITS 		1

// Direcciones de los registros del SHT11 para leer y escribir en él.
#define SHT11_STATUS_REG_W 	0x06	// Escritura del registro de estado
#define SHT11_STATUS_REG_R 	0x07	// Lectura del registro de estado
#define SHT11_MEASURE_TEMP 	0x03	// Medición de la temperatura
#define SHT11_MEASURE_HUMI 	0x05	// Medición de la humedad
#define SHT11_RESET 		0x1E	// Reset


// Pins donde está conectado el sensor.
/*#ifndef SHT11_CLK
#define SHT11_CLK	RC0		//Pin de reloj
#define SHT11_CLK_TRIS	TRISC0
#endif

#ifndef SHT11_DATA
#define SHT11_DATA	RC1		//Pin de datos
#define SHT11_DATA_TRIS	TRISC1
#endif*/


// Pins donde está conectado el sensor.     
#define HTS11_SCL_DIO   E_AHI_DIO15_INT     // Jennic-Sensor-Prototype.  OK. SCL multiple pulse of 24 us
#define HTS11_DATA_DIO  E_AHI_DIO16_INT     // Jennic-Sensor-Prototype.  OK. DATA 1 pulse 122 us.
//#define HTS11_SCL_DIO   E_AHI_DIO13_INT   // Jennic-USB.    SCL multiple pulse of 24 us
//#define HTS11_DATA_DIO  E_AHI_DIO12_INT   // Jennic-USB.    DATA 1 pulse 122 us

// Set pins to Output direction.    void vAHI_DioSetDirection(uint32 u32Inputs, uint32 u32Outputs);
#define vSetSCLDirectionOutput()    vAHI_DioSetDirection(0, HTS11_SCL_DIO);    // SCL  Output.        // SHT11_CLK_TRIS = 0;
#define vSetDATADirectionOutput()   vAHI_DioSetDirection(0, HTS11_DATA_DIO);    // DATA Output.        // SHT11_DATA_TRIS = 0;


// Set pins to Input direction.     void vAHI_DioSetDirection(uint32 u32Inputs, uint32 u32Outputs);
#define vSetSCLDirectionInput()     vAHI_DioSetDirection(HTS11_SCL_DIO, 0);    // SCL  Input.         // SHT11_CLK_TRIS = 1;
#define vSetDATADirectionInput()    vAHI_DioSetDirection(HTS11_DATA_DIO, 0);    // DATA Input.         // SHT11_DATA_TRIS = 1;

// Set pins to bit 0.               void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off); 
#define vSetSCL_0()                 vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  // SCL-OFF     // SHT11_CLK = 0;
#define vSetDATA_0()                vAHI_DioSetOutput(0 , HTS11_DATA_DIO);  // DATA-OFF     // SHT11_DATA = 0;

// Set pins to bit 1.               void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off); 
#define vSetSCL_1()                 vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  // SCL-ON       // SHT11_CLK = 0;
#define vSetDATA_1()                vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  // DATA-ON       // SHT11_DATA = 1;

// Read pins.                       
#define vReadSCL()                  (u32AHI_DioReadInput() & HTS11_SCL_DIO)   // SCL-Read     SHT11_CLK
#define vReadDATA()                 (u32AHI_DioReadInput() & HTS11_DATA_DIO)  // DATA-Read    SHT11_DATA


//-------------------------------------------------------------------------------------------------------

//#define SHT11_VCC_5V
#define SHT11_VCC_3V3
#define SHT11_USE_RESOLUTION_14_12BITS

// Constantes para el calculo de la temperatura y humedad real
#ifdef SHT11_VCC_5V
#define D1	40
#endif

#ifdef SHT11_VCC_4V
#define D1	39.75
#endif

#ifdef SHT11_VCC_3V5
#define D1	39.66
#endif

#ifdef SHT11_VCC_3V3
#define D1	39.6575
#endif

#ifdef SHT11_VCC_3V
#define D1	39.60
#endif

#ifdef SHT11_VCC_2V5
#define D1	39.55
#endif

//---------------------------------------------------------------------------------------------------------



#ifdef SHT11_USE_RESOLUTION_14_12BITS

#define C1	-4.0
#define C2	 0.0405
#define C3	-0.0000028
#define T1	 0.01
#define T2	 0.00008
#define D2	 0.01

#else

#define C1	-4.0
#define C2	 0.648
#define C3	-0.00072
#define T1	 0.01
#define T2	 0.00128
#define D2   0.04

#endif

/**
 * Inicialica las comunicaciones con el sensor.
 */
void SHT11_Init(void);

/**
 * Reseteo e inicialización del sensor
 */
void SHT11_ResetAndInit(void);

/**
 * Modifica la resolución de las medidas.
 * Por defecto: 14 bits temepratura y 12 bits humedad.
 * Opcionalmente:  12 bits temperatura y 8 bits humedad
 *
 * @param res	Resolución a usar.
 * @return		Código de error.
 */
BYTE SHT11_SetMeasureResolution(BYTE res);

/**
 * Funcion para activar/desactivar el calentador del SHT11
 * Al activarlo autocalienta el sensor en unos 5ºC.
 *
 * @param onOff	 Encender o apagar el calentador.
 * @return		 Código de error.
 */
BYTE SHT11_OnOffCalentador(BYTE onOff);

/**
 * Obtiene el valor de la temperatura y de la humedad.
 *
 * @param t		La temperatura actual.
 * @param h		La humedad actual.
 * @return		Código de error.
 */
BYTE SHT11_MeasureTemperatureAndHumidity(double *t, double *h);

/**
 * Calcula el punto de rocio en base a la temperatura y la humedad.
 *
 * @param t		La temperatura.
 * @param h		La humedad.
 * @return		El punto de rocio.
 */
double SHT11_CalcPuntoRocio(double t, double h);

BYTE SHT11_ReadStatusRegister(BYTE *valor, BYTE *checksum);

BYTE SHT11_WriteStatusRegister(BYTE *valor);


BYTE SHT11_Init_MeasureTemperatureAndHumidity(double *t, double *h);


#endif /* SHT11_H_ */
