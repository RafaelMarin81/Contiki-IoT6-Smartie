#include "lightdriver.h"


int status = 0;
uint32_t luminity = 0;

void init_Light(){
	vSetDIO13_OutputDirection();
}

void set_on(){
	vSetDIO13_1();
	status = 1;
}

void set_off(){
	vSetDIO13_0();
	status = 0;
}

int get_light_status(){
	return status;
}

int switch_light(){
	if (get_light_status()){
		status = 0;
	} else {
		status = 1;
	}
	return status;
}

int get_luminity(){
	return luminity;
}

int set_luminity(uint32_t value){
	set_on();
	//~ luminity = luminity+value;
	luminity = value;
	if (luminity<0) luminity=0;
	if (luminity>255) luminity=255;
	
    vTimerConfig(luminity);
	
	return luminity;
}



// 16-MHz system clock sourced.
// Prescale divides the system clock by a factor of 2^prescale, frequency, 
// prescale = 3, factor = 8, frequency = 2-MHz = 
#define PRESCALE 8         // integer value in the range 0 to 16 
// 16us

//#define PULSE_PER_SECOND    244     // 
//#define PULSE_COUNT (PULSE_PER_SECOND*15)   // uint16.  15 seconds. because  vAHI_Watchdog = 16 seconds.


static uint8_t currentCount = 0;
static uint8_t dutycycle = 0;
/**
 * \brief     Function handler the interruption of Timer0.
 */
void vTimer0_handler(uint32 u32DeviceId, uint32 u32ItemBitmap) {
  
  if(currentCount == dutycycle) {
    vSetDIO13_0();
  } else if (currentCount == 0) {
    vSetDIO13_1();
  }

  currentCount++;   // IMPORTANT: Use variable of 8bits to set 0 automatically.

}


/**
 * \brief     Function initialize Jennic. Watchdog-Timer and Timer0.
 */
void vTimerConfig(uint8_t duty)  {
  
    // Initialize the DIO13.
    init_Light();
    set_on();
    dutycycle = duty;
    currentCount = 0;
  
    // Disable before TimerDIOControl
    vAHI_TimerDisable(E_AHI_TIMER_0);
    // void vAHI_TimerDIOControl(uint8 u8Timer, bool_t bDIOEnable);
    vAHI_TimerDIOControl(E_AHI_TIMER_0, FALSE);     // FALSE to disable (so available as GPIOs = Input/Output DIOs)

    //void vAHI_TimerEnable(uint8 u8Timer, uint8 u8Prescale, bool_t bIntRiseEnable, bool_t bIntPeriodEnable, bool_t bOutputEnable);
    // Set up timer0 for timer-mode. with no prescale, yes interrupt, no output.
    vAHI_TimerEnable(E_AHI_TIMER_0, PRESCALE, FALSE, TRUE, FALSE);

    // void vAHI_TimerClockSelect(uint8 u8Timer, bool_t bExternalClock, bool_t bInvertClock);
    // NO use external clock, No invert clock.
    vAHI_TimerClockSelect(E_AHI_TIMER_0,FALSE,FALSE);

//    void vAHI_TimerConfigureOutputs(uint8 u8Timer,bool_t bInvertPwmOutput, bool_t bGateDisable);

    //uint8 u8AHI_TimerFired(uint8 u8Timer);
    u8AHI_TimerFired(E_AHI_TIMER_0);

    // Register Timer0 interrupt 
    vAHI_Timer0RegisterCallback(vTimer0_handler);

    // void vAHI_TimerStartSingleShot(uint8 u8Timer, uint16 u16Hi, uint16 u16Lo);
    // Generate interruption for rising edge after PULSE_COUNT lo-hi events
    vAHI_TimerStartRepeat(E_AHI_TIMER_0, 0, 1); // Interrupt in 16us.
//    vAHI_TimerStartSingleShot(E_AHI_TIMER_0, 0x0000,       // null value   
//          0x1000);      // number of pulses to count ( 1000hex = 4096dec )   

}

/*void set_timer(int n){
  
  // Disable before TimerDIOControl
  vAHI_TimerDisable(E_AHI_TIMER_1);
  // void vAHI_TimerDIOControl(uint8 u8Timer, bool_t bDIOEnable);
  //vAHI_TimerDIOControl(E_AHI_TIMER_1, FALSE);     // FALSE to disable (so available as GPIOs = Input/Output DIOs)
  vAHI_TimerClockSelect(E_AHI_TIMER_1, FALSE, TRUE);  */ /* Configure the clock. */
  /* Set DIO12 as output and low (just to show how to use timer IO's) */
 // vAHI_DioSetDirection(0, 1 << 12);
 // vAHI_DioSetOutput(0, 1 << 12);
  
  /* Enable Timer1: 16MHz / 2^10 = 15625Hz 
  //vAHI_TimerEnable(E_AHI_TIMER_1, 10, FALSE, FALSE, TRUE);  
  vAHI_TimerEnable(E_AHI_TIMER_1, 10, FALSE, FALSE, TRUE);
  
  //vAHI_TimerConfigure(E_AHI_TIMER_1, FALSE, TRUE);
  
  uint16 count = 15625 / 100;
  
  vAHI_TimerStartRepeat(E_AHI_TIMER_1, n, count);
  
  
}*/


