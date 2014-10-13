/*
 * sht-sensor.c
 *  Library for managing the sensor SHT11 of temperature and humidity.
 *
 *  Created on: 11-may-2012
 *      Author: Rafael Marin Perez.
 */

#include "sht-sensor.h"
#include <math.h>




//#define DEBUG_SHT11 1

#ifdef DEBUG_SHT11
#include "juart.h"
#define PRINTF(...) vPrintf(__VA_ARGS__)
#define ERROR(...) vPrintf(__VA_ARGS__)
#else
#define PRINTF(...)
#define ERROR(...) 
#endif


#ifdef DEBUG_ERROR
#define ERROR(...) vPrintf(__VA_ARGS__)
#endif


/**
 * Genera el retardo
 */
void Delay(void) {
    int i = 0;
    for(i=0; i < 5000; i++);
}

/**
 * Escribe un byte en el sensor y chequear si es reconocido
 *
 */
BYTE SHT11_WriteByte(BYTE valor) {

	BYTE i,error=0;

	vAHI_DioSetDirection(0, HTS11_DATA_DIO);    	// DATA  Output.        SHT11_DATA_TRIS = 0;
	vAHI_DioSetOutput(0, HTS11_DATA_DIO);  	// DATA-OFF      SHT11_DATA = 0;

	for (i = 0x80; i > 0; i /= 2) {
		if (i & valor)
			vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  	// DATA-ON      SHT11_DATA = 1;
		else
			vAHI_DioSetOutput(0, HTS11_DATA_DIO);  	// DATA-OFF      SHT11_DATA = 0;

		vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;

		// Delay_us(5);
        Delay(); Delay(); Delay();

		vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;
	}


	//configuramos como entrada
	vAHI_DioSetDirection(HTS11_DATA_DIO, 0);    	// DATA  Input.        SHT11_DATA_TRIS = 1;
	vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;

	error = (u32AHI_DioReadInput() & HTS11_DATA_DIO);  // DATA-Read    SHT11_DATA
	vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;

	vAHI_DioSetDirection(0, HTS11_DATA_DIO);    	// DATA  Output.        SHT11_DATA_TRIS = 0;

	return error;
}

/**
 *  Lee un byte en el sensor y da el reconocimeinto si ack es igual a 1
 *
 *  @param	ack
 *  @return
 */
BYTE SHT11_ReadByte(BYTE ack) {
	BYTE i , valor = 0;

	vAHI_DioSetDirection(HTS11_DATA_DIO, 0);    	// DATA  Input.        SHT11_DATA_TRIS = 1;

	for (i = 0x80; i > 0; i /= 2) {
		vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;

		if ((u32AHI_DioReadInput() & HTS11_DATA_DIO))  // DATA-Read    SHT11_DATA
			valor = (valor | i);
		vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;
	}

	vAHI_DioSetDirection(0, HTS11_DATA_DIO);    	// DATA  Output.        SHT11_DATA_TRIS = 0;

//	SHT11_DATA = !ack;
    if(ack == 1) {
        // Set pins to bit 0.               void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off); 
        vAHI_DioSetOutput(0 , HTS11_DATA_DIO);  // DATA-OFF     // SHT11_DATA = 0;
    } else {
        // Set pins to bit 1.               void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off); 
        vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  // DATA-ON       // SHT11_DATA = 1;
    }

	vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;

	//Delay_us(5);
    Delay(); Delay(); Delay();

	vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;

	vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  	// DATA-ON      SHT11_DATA = 1;
	vAHI_DioSetDirection(0, HTS11_DATA_DIO);    	// DATA  Output.        SHT11_DATA_TRIS = 0;

	return valor;
}

/**
 * Rutina de inicializacion del sensor, utilizar despues del reset.
 *
 * Genera un comienzo de trasmision.
 *       ______         _______
 * DATOS:      |_______|
 *            ___     ___
 * SCK :  ___|   |___|   |_____
 */
void SHT11_InitTransmision(void) {

	vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  	// DATA-ON      SHT11_DATA = 1;

	vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;

	Delay();    // Delay_us(2);

	vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;

	Delay();    // Delay_us(2);

	vAHI_DioSetOutput(0, HTS11_DATA_DIO);  	// DATA-OFF      SHT11_DATA = 0;

	Delay();    // Delay_us(2);

	vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;

	Delay();    // Delay_us(2);

	vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;

	Delay();    // Delay_us(2);

	vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  	// DATA-ON      SHT11_DATA = 1;

	Delay();    // Delay_us(2);

	vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;
}

/**
 * Reseteo del sensor; linea de datos a 1 seguido de 9 ciclos de reloj y de
 * la funcion SHT11_InitTransmision()
 *
 *
 *        _____________________________________________________           ________
 * DATA:                                                       |_________|
 *          _    _    _    _    _    _    _    _    _        _____     _____
 * SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|     |___|     |___
 *
 */
void SHT11_ResetAndInit(void) {
	BYTE i;

	vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  	// DATA-ON      SHT11_DATA = 1;
	vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;

	for(i = 0; i < 9; i++) {
		vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;
		Delay();    // Delay_us(2);
		vAHI_DioSetOutput(0 , HTS11_SCL_DIO);  	// SCL-OFF      SHT11_CLK = 0;
		Delay();    // Delay_us(2);
	}

    SHT11_InitTransmision();
}

/**
 * Resetea el sensor
 *
 * @return
 */
BYTE SHT11_SoftReset(void) {
    BYTE error = 0;

    SHT11_ResetAndInit();

	error += SHT11_WriteByte(SHT11_RESET);

	return error;
}

/**
 * Lectura del registro de estado con checksum de 8 bits
 *
 * @param valor		Valor del registro de estado.
 * @param checksum	Checksum.
 *
 * @return			Código de error.
 */
BYTE SHT11_ReadStatusRegister(BYTE *valor, BYTE *checksum) {

	BYTE error = 0;

	SHT11_InitTransmision();

	error = SHT11_WriteByte(SHT11_STATUS_REG_R);

	*valor = SHT11_ReadByte(SHT11_ACK);

	*checksum = SHT11_ReadByte(SHT11_NACK);

	return error;

}

/**
 * Escribimos en el registro de estado con un checksum de 8 bits
 *
 * @param valor		Valor del registro de estado.
 * @return			Código de error.
 */
BYTE SHT11_WriteStatusRegister(BYTE *valor) {

	BYTE error = 0;

	SHT11_InitTransmision();

    error += SHT11_WriteByte(SHT11_STATUS_REG_W);

    error += SHT11_WriteByte(*valor);

    return error;
}

/**
 * Realiza la medicion de temperatura y humedad con checksum incluido.
 * Todavia no es la medicion real, se debe hacer la compensacion.
 *
 *@param valor		Valor de humedad o temeperatura.
 *@param checksum	Checksum leido.
 *@param modo		Indica si vamos a leer la humedad o la temperatura.
 *@return			Código de error.
 */
BYTE SHT11_Measure(BYTE *valor, BYTE *checksum, BYTE modo) {

	BYTE error = 0;

	WORD i;

	SHT11_InitTransmision();

	error += SHT11_WriteByte(modo);

	// Configuramos como entrada para leer
	vAHI_DioSetDirection(HTS11_DATA_DIO, 0);    	// DATA  Input.        SHT11_DATA_TRIS = 1;

	for (i = 0; i < 65535; i++) {
		if((u32AHI_DioReadInput() & HTS11_DATA_DIO) == 0)   // DATA-Read    SHT11_DATA
			break;
	}

	if(u32AHI_DioReadInput() & HTS11_DATA_DIO) {   // DATA-Read    SHT11_DATA
		error += 1;
	}

	// Volvemos a dejar la linea de datos como salida
	//vAHI_DioSetDirection(0, HTS11_DATA_DIO);    	// DATA  Output.        SHT11_DATA_TRIS = 0;

	*(valor)= SHT11_ReadByte(SHT11_ACK);	    // MSB = most significant byte
	*(valor+1)	= SHT11_ReadByte(SHT11_ACK);	// LSB = least significant byte
	*checksum 	= SHT11_ReadByte(SHT11_NACK);

    PRINTF("SHT11_Measure: valor[0] = %x, valor[1] = %x, checksum %x, error %x\n", *(valor), *(valor +1), checksum, error);

	return error;
}

/**
 * Calculo de la temperatura en ºC y humedad en %
 * Entrada proviniente del sensor:
 *	 Humedad - 12 bits (por defecto)
 *   Temperatura - 14 bits (por defecto)
 * Salida:
 *   Humedad - RH%
 *   Temperatura - ºC
 *
 * @param temperatura   La Temperatura.
 * @param humedad		La humedad.
 */
void SHT11_CalcRealTemperaturaAndHumidity(double *temperatura, double *humedad) {
	double rh;
	double t;
	double rh_lin;
	double rh_true;
	double t_C;

	rh = *humedad;
	t =  *temperatura;

	t_C = (t * D2) - D1;
	rh_lin = (C3 * rh * rh) + (C2 * rh) + C1;
	rh_true = (t_C - 25)*(T1 + T2 * rh) + rh_lin;

	if(rh_true > 100)
		rh_true = 100;

	if(rh_true < 0.1)
		rh_true = 0.1;

	*temperatura = t_C;
	*humedad = rh_true;
}


/**
 * Obtiene el valor de la temperatura y de la humedad.
 *
 * @param t		La temperatura actual.
 * @param h		La humedad actual.
 * @return		Código de error.
 */
BYTE SHT11_MeasureTemperatureAndHumidity(double *t, double *h) {
	uint16_t temp, hum;     // IMPORTANT:  2 bytes variables.
	BYTE checksum, error = 0;

	error = SHT11_Measure((BYTE  *)&temp, &checksum, SHT11_MEASURE_TEMP);
	error += SHT11_Measure((BYTE  *)&hum, &checksum, SHT11_MEASURE_HUMI);

	PRINTF("SHT11_MeasureTemperatureAndHumidity: temp %x, hum %x\n", temp, hum);

	*t = (double)temp;
	*h = (double)hum;

	SHT11_CalcRealTemperaturaAndHumidity(t, h);

	return error;
}

/**
 * Calcula el punto de rocio.
 *
 * @param t		La Temperatura.
 * @param h		La humedad.
 * @return		El punto de rocio.
 */
/*double SHT11_CalcPuntoRocio(double t, double h) {

	double logEx, punto_rocio;

	logEx = 0.66077 + 7.5 * t / (237.3 + t) + (log10(h) - 2);
	punto_rocio = (logEx - 0.66077) * 237.3 / (0.66077 + 7.5 - logEx) ;

	return punto_rocio;
}*/


/**
 * Funcion para activar/desactivar el calentador del SHT11
 * Al activarlo autocalienta el sensor en unos 5ºC.
 *
 * @param onOff	 Encender o apagar el calentador.
 * @return		 Código de error.
 */
BYTE SHT11_OnOffCalentador(BYTE onOff) {
	BYTE valor, checksum;
	BYTE error = 0;


	SHT11_ResetAndInit();

	error += SHT11_ReadStatusRegister(&valor, &checksum);

	SHT11_ResetAndInit();

	if (!error && (((valor >> 2) & 0x01) != onOff)) {
		if(onOff)
			valor |= 0x04;	//activa el calentador
		else
			valor &= 0xFA;	//desactiva el calentador

	  error += SHT11_WriteStatusRegister(&valor);
	}

	return error;
}

/**
 * Modifica la resolución de las medidas.
 * Por defecto: 14 bits temepratura y 12 bits humedad.
 * Opcionalmente:  12 bits temperatura y 8 bits humedad
 *
 * @param res	Resolución a usar.
 * @return		Código de error.
 */
BYTE SHT11_SetMeasureResolution(BYTE res) {

	BYTE valor, checksum;
	BYTE error = 0;

	SHT11_ResetAndInit();

	error += SHT11_ReadStatusRegister(&valor, &checksum);

	SHT11_ResetAndInit();

	if (!error && ((valor & 0x01) != res))
	{
	  if(res)
		  valor |= 0x01;	// 12 bits temperatura y 8 bits humedad
	  else
		  valor &= 0xFE;	//14 bits temperatura y 12 bits humedad

	  error += SHT11_WriteStatusRegister(&valor);
	}

    PRINTF("SHT11_SetMeasureResolution: error:%x\n", error);

	return error;
}

/**
 * Inicialica las comunicaciones con el sensor.
 */
void SHT11_Init(void){

	vAHI_DioSetOutput(HTS11_SCL_DIO, 0);  	// SCL-ON      SHT11_CLK = 1;
	vAHI_DioSetOutput(HTS11_DATA_DIO, 0);  	// DATA-ON      SHT11_DATA = 1;

	// Inicializamos los pins como salidas
	vAHI_DioSetDirection(0, HTS11_SCL_DIO);    	// SCL  Output.        SHT11_CLK_TRIS = 0;
	vAHI_DioSetDirection(0, HTS11_DATA_DIO);    	// DATA  Output.        SHT11_DATA_TRIS = 0;

	// Reseteamos e inicializamos el sensor
	SHT11_ResetAndInit();
}


BYTE SHT11_Init_MeasureTemperatureAndHumidity(double *t, double *h) {
    BYTE error = 0;

    // 1. Initialize. 
    SHT11_Init();

    // 2. Set the resolution.
    SHT11_SetMeasureResolution(SHT11_RESOLUCION_14_12_BITS);

    // 3. Measure the temperature and humidity.
    error = SHT11_MeasureTemperatureAndHumidity(t, h);

    ERROR("[SHT11_Init_MeasureTemperatureAndHumidity] error:%i  temp:%i (RH) humi:%i(oC)\n",error, (int)*t, (int)*h);

    return error;
}

