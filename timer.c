#include "device_registers.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"

int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */
int lpit0_ch1_flag_counter = 0; /*< LPIT0 timeout counter */
unsigned int num = 0;

void WDOG_disable (void)
{
  WDOG->CNT=0xD928C520;     /* Unlock watchdog       */
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value    */
  WDOG->CS = 0x00002100;    /* Disable watchdog       */
}

void PORT_init (void)
{
  PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT E */

}

void NVIC_init_IRQs(void)
{
	/*LPIT ch0 overflow set*/
	S32_NVIC->ICPR[1] |= 1 << (48 % 32);
	S32_NVIC->ISER[1] |= 1 << (48 % 32);
	S32_NVIC->IP[48] = 0x00;
	/*LPIT ch1 overflow set*/
	S32_NVIC->ICPR[1] |= 1 << (49 % 32);
	S32_NVIC->ISER[1] |= 1 << (49 % 32);
	S32_NVIC->IP[49] = 0x0B;
}

void LPIT0_init()
{
	
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);   
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; 
	LPIT0->MCR = 0x00000001; 
	LPIT0->MIER = 0x03;  /* TIE0=1: Timer Interrupt Enabled fot Chan 0,1 */
	
	LPIT0->TMR[0].TVAL = 40;      // 1us 마다 인터럽트 발생하도록
	LPIT0->TMR[0].TCTRL = 0x00000001;
	
	LPIT0->TMR[1].TVAL = 40000;      /* Chan 1 Timeout period: 40M clocks */
	LPIT0->TMR[1].TCTRL = 0x00000001;
}


void LPIT0_Ch1_IRQHandler (void)
{	  /* delay counter */
	
}

void LPIT0_Ch0_IRQHandler (void)
{
	num++;
	
	lpit0_ch0_flag_counter++;         /* Increment LPIT0 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK;  /* Clear LPIT0 timer flag 0 */
}

int main(void)
{
   WDOG_disable();/* Disable Watchdog in case it is not done in startup code */
   PORT_init();            /* Configure ports */
   SOSC_init_8MHz();       /* Initialize system oscilator for 8 MHz xtal */
   SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
   NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
   NVIC_init_IRQs();       /* Enable desired interrupts and priorities */
   LPIT0_init();

   while(1);
}
