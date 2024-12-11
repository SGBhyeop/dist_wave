#include "device_registers.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"

int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */
int lpit0_ch1_flag_counter = 0; /*< LPIT0 timeout counter */
unsigned int num,num0,num1,num2,num3,count = 0;
unsigned int FND_DATA[10]={0x7E, 0x0C,0xB6,0x9E,0xCC,0xDA,0xFA,0x4E,0xFE,0xCE};// 0~9 number
unsigned int FND_SEL[4]={0x0100,0x0200,0x0400,0x0800};
unsigned int j = 0; /*FND select pin index */
int start_time, end_time, time_width, dist = 0;

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

  PTD->PDDR |= 1<<8|1<<9|1<<10|1<<11;
  PORTD->PCR[8] = PORT_PCR_MUX(1); /* Port D8: MUX = GPIO */
  PORTD->PCR[9] = PORT_PCR_MUX(1); /* Port D9: MUX = GPIO */
  PORTD->PCR[10] = PORT_PCR_MUX(1); /* Port D10: MUX = GPIO */
  PORTD->PCR[11] = PORT_PCR_MUX(1); /* Port D11: MUX = GPIO */

    // 인터럽트 받을 포트
    PCC-> PCCn[PCC_PORTE_INDEX] = PCC_PCCn_CGC_MASK;
    PTE->PDDR &= ~(1<<0);   // E0 echo 핀 상승
    PTE->PDDR &= ~(1<<1); //E1핀 echo 핀 하강 입력으로
    PORTE->PCR[0] = PORT_PCR_MUX(1);
    PORTE->PCR[1] = PORT_PCR_MUX(1);
    PORTE->PCR[0] |= (9<<16); //rising edge
    PORTE->PCR[1] |= (10<<16); //falling edge

    //트리거 신호 보낼 포트
    PTD->PDDR |= 1<<0; //D0핀 trig 핀으로 
    PORTD->PCR[0] = PORT_PCR_MUX(1);
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
    //포트 E 인터럽트
    S32_NVIC->ICPR[1] |= 1 << (63 % 32);
	S32_NVIC->ISER[1] |= 1 << (63 % 32);
	S32_NVIC->IP[63] = 0x0B;
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

	LPIT0->TMR[0].TVAL = 40000000;      /* 1 초마다 인터럽트 */
  LPIT0->TMR[0].TCTRL = 0x00000001;
	  	  	  	  	  	  	  	  /* T_EN=1: Timer channel is enabled */
	                              /* CHAIN=0: channel chaining is disabled */
	                              /* MODE=0: 32 periodic counter mode */
	                              /* TSOT=0: Timer decrements immediately based on restart */
	                              /* TSOI=0: Timer does not stop after timeout */
	                              /* TROT=0 Timer will not reload on trigger */
	                              /* TRG_SRC=0: External trigger soruce */
	                              /* TRG_SEL=0: Timer chan 0 trigger source is selected*/

	LPIT0->TMR[1].TVAL = 40000;      /* 1us마다 인터럽트 */
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
//포트 E 인터럽트 함수
void PORTE_IRQHandler(void){

	PORTE->PCR[0] &= ~(0x01000000); // Port Control Register ISF bit '0' set
    PORTE->PCR[1] &= ~(0x01000000);
	//PORTC_Interrupt State Flag Register Read
	if((PORTE->ISFR & (1<<0)) != 0){ //rising edge에서
		start_time = count;
	}
    if((PORTE->ISFR & (1<<1)) != 0){
        end_time = count;
        time_width = end_time - start_time;
    }

	PORTC->PCR[0] |= 0x01000000; // Port Control Register ISF bit '1' set
    PORTC->PCR[1] |= 0x01000000;
}

void LPIT0_Ch1_IRQHandler (void)
{	  /* delay counter */
	unsigned int k;
    count ++;
	dist = time_width / 58;
	k = j++ & 0x00000003;
	
	PTD->PCOR = 0xffe;
	num3=(dist/1000)%10;
	num2=(dist/100)%10;
	num1=(dist/10)%10;
	num0= dist%10;
	
  switch(k)
  {
		case 0:
		// 1000
			PTD->PSOR = FND_SEL[k];
			PTD->PSOR = FND_DATA[num3];
			break;
		
		case 1:
		// 100
			PTD->PSOR = FND_SEL[k];
			PTD->PSOR = FND_DATA[num2];
			break;
  
		case 2:
		// 10
			PTD->PSOR = FND_SEL[k];
			PTD->PSOR = FND_DATA[num1];
			break;
	
		case 3:
		// 1
			PTD->PSOR = FND_SEL[k];
			PTD->PSOR = FND_DATA[num0];
			break;
  }
	
	lpit0_ch1_flag_counter++;         /* Increment LPIT1 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF1_MASK;  /* Clear LPIT0 timer flag 1 */
}

void LPIT0_Ch0_IRQHandler (void)
{
	num++;
	if(num % 2 ==0)
	PTD->PTOR |= 1<<0;
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

   while(1){

   }
}
