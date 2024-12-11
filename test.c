#include "device_registers.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"

int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */
int lpit0_ch1_flag_counter = 0; /*< LPIT0 timeout counter */
unsigned int num,num0,num1,num2,num3 = 0;
unsigned int FND_DATA[10]={0x7E, 0x0C,0xB6,0x9E,0xCC,0xDA,0xFA,0x4E,0xFE,0xCE};// 0~9 number
unsigned int FND_SEL[4]={0x0100,0x0200,0x0400,0x0800};
unsigned int j = 0; /*FND select pin index */


void WDOG_disable (void)
{
  WDOG->CNT=0xD928C520;     /* Unlock watchdog       */
  WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value    */
  WDOG->CS = 0x00002100;    /* Disable watchdog       */
}

void PORT_init (void)
{
  /*
   * ===============PORTE SEGMENT=====================
   */
  PCC-> PCCn[PCC_PORTD_INDEX] = PCC_PCCn_CGC_MASK; /* Enable clock for PORT E */
  PTD->PDDR |= 1<<1|1<<2|1<<3|1<<4|1<<5|1<<6|1<<7;   /* Port E0: Data Direction= output (default) */
  PORTD->PCR[1] = PORT_PCR_MUX(1); /* Port D1: MUX = GPIO */
  PORTD->PCR[2] = PORT_PCR_MUX(1); /* Port D2: MUX = GPIO */
  PORTD->PCR[3] = PORT_PCR_MUX(1); /* Port D3: MUX = GPIO */
  PORTD->PCR[4] = PORT_PCR_MUX(1); /* Port D4: MUX = GPIO */
  PORTD->PCR[5] = PORT_PCR_MUX(1); /* Port D5: MUX = GPIO */
  PORTD->PCR[6] = PORT_PCR_MUX(1); /* Port D6: MUX = GPIO */
  PORTD->PCR[7] = PORT_PCR_MUX(1); /* Port D7: MUX = GPIO */  

  PTD->PDDR |= 1<<8|1<<9|1<<10|1<<11|1<<16;
  PORTD->PCR[8] = PORT_PCR_MUX(1); /* Port D8: MUX = GPIO */
  PORTD->PCR[9] = PORT_PCR_MUX(1); /* Port D9: MUX = GPIO */
  PORTD->PCR[10] = PORT_PCR_MUX(1); /* Port D10: MUX = GPIO */
  PORTD->PCR[11] = PORT_PCR_MUX(1); /* Port D11: MUX = GPIO */
  PORTD->PCR[16] = PORT_PCR_MUX(1); /* Port D11: MUX = GPIO */
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
   /*!
    * LPIT Clocking:
    * ==============================
    */
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs 		*/
	  /*!
	   * LPIT Initialization:
	   */
	LPIT0->MCR = 0x00000001;  /* DBG_EN-0: Timer chans stop in Debug mode */
	                              	  	  /* DOZE_EN=0: Timer chans are stopped in DOZE mode */
	                              	  	  /* SW_RST=0: SW reset does not reset timer chans, regs */
	                              	  	  /* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
	LPIT0->MIER = 0x03;  /* TIE0=1: Timer Interrupt Enabled fot Chan 0,1 */

	LPIT0->TMR[0].TVAL = 40000000;      /* Chan 0 Timeout period: 40M clocks */
  LPIT0->TMR[0].TCTRL = 0x00000001;
	  	  	  	  	  	  	  	  /* T_EN=1: Timer channel is enabled */
	                              /* CHAIN=0: channel chaining is disabled */
	                              /* MODE=0: 32 periodic counter mode */
	                              /* TSOT=0: Timer decrements immediately based on restart */
	                              /* TSOI=0: Timer does not stop after timeout */
	                              /* TROT=0 Timer will not reload on trigger */
	                              /* TRG_SRC=0: External trigger soruce */
	                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/

	LPIT0->TMR[1].TVAL = 40000;      /* Chan 1 Timeout period: 40M clocks */
  LPIT0->TMR[1].TCTRL = 0x00000001;
	  	  	  	  	  	  	  	  /* T_EN=1: Timer channel is enabled */
	                              /* CHAIN=0: channel chaining is disabled */
	                              /* MODE=0: 32 periodic counter mode */
	                              /* TSOT=0: Timer decrements immediately based on restart */
	                              /* TSOI=0: Timer does not stop after timeout */
	                              /* TROT=0 Timer will not reload on trigger */
	                              /* TRG_SRC=0: External trigger soruce */
	                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}


void LPIT0_Ch1_IRQHandler (void)
{	  /* delay counter */
	
	lpit0_ch1_flag_counter++;         /* Increment LPIT1 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF1_MASK;  /* Clear LPIT0 timer flag 1 */
}

void LPIT0_Ch0_IRQHandler (void)
{
	num++;
	while(num % 2 == 0){
        PTD->PSOR |= 1<<16;
    }
    while(num % 2 == 1){
        PTD->PCOR |= 1<<16;
    }
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
