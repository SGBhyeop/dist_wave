#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"
int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */

/*num is Counting value, num0 is '1', num2 is '10', num2 is '100', num3 is '1000'*/
//unsigned int External_PIN=0; /* External_PIN:SW External input Assignment */
char MODE = 'N';
unsigned int Dtime = 0; /* Delay Time Setting Variable*/

void PORT_init (void)
{
	  //PORTC,D Data Clock Set
	PCC->PCCn[PCC_PORTC_INDEX]|=PCC_PCCn_CGC_MASK;   /* Enable clock for PORTC */
	PCC->PCCn[PCC_PORTD_INDEX]|=PCC_PCCn_CGC_MASK;   /* Enable clock for PORTD */
	
	//PortC,D Data Direction Set
	PTC->PDDR &= ~(1<<11);		/* Port C11 Port Input set, value '0'*/
	PTC->PDDR &= ~(1<<12);		/* Port C12 Port Input set, value '0'*/
	PTC->PDDR &= ~(1<<13);		/* Port C12 Port Input set, value '0'*/
	PTD->PDDR |= 1<<0|1<<15|1<<16;
	
	// PORTC_11 pin GPIO and Falling-edge Set
	PORTC->PCR[11] |= PORT_PCR_MUX(1); // Port C11 mux = GPIO
	PORTC->PCR[11] |=(10<<16); // Port C11 IRQC : interrupt on Falling-edge
	
	// PORTC_12 pin GPIO and Falling-edge Set
	PORTC->PCR[12] |= PORT_PCR_MUX(1); // Port C12 mux = GPIO
	PORTC->PCR[12] |=(10<<16); // Port C12 IRQC : interrupt on Falling-edge
	
	PORTC->PCR[13] |= PORT_PCR_MUX(1); // Port C12 mux = GPIO
	PORTC->PCR[13] |=(10<<16); // Port C12 IRQC : interrupt on Falling-edge
	
	PORTD->PCR[0] |= PORT_PCR_MUX(1); // Port C12 mux = GPIO
	PORTD->PCR[15] |= PORT_PCR_MUX(1); // Port C12 mux = GPIO
	PORTD->PCR[16] |= PORT_PCR_MUX(1); // Port C12 mux = GPIO

  PORTC->PCR[1]|=PORT_PCR_MUX(2);    /* Port D16을 C1로 바꿈 FTM0CH1 */
	PORTC->PCR[0]|=PORT_PCR_MUX(2); /*아마도 포트C 0핀을 출력으로 */
		
}
void WDOG_disable (void)
{
	WDOG->CNT=0xD928C520;     /* Unlock watchdog 		*/
	WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value 	*/
	WDOG->CS = 0x00002100;    /* Disable watchdog 		*/
}

void LPIT0_init (uint32_t delay)
{
	uint32_t timeout;
	/*!
	* LPIT Clocking:
	* ==============================
	*/
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs       */
	
	/*!
	* LPIT Initialization:
	*/
	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
					/* DOZE_EN=0: Timer chans are stopped in DOZE mode */
					/* SW_RST=0: SW reset does not reset timer chans, regs */
					/* M_CEN=1: enable module clk (allows writing other LPIT0 regs) */
	
	timeout=delay* 40000;
	LPIT0->TMR[0].TVAL = timeout;      /* Chan 0 Timeout period: 40M clocks */
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
			      /* T_EN=1: Timer channel is enabled */
			      /* CHAIN=0: channel chaining is disabled */
			      /* MODE=0: 32 periodic counter mode */
			      /* TSOT=0: Timer decrements immediately based on restart */
			      /* TSOI=0: Timer does not stop after timeout */
			      /* TROT=0 Timer will not reload on trigger */
			      /* TRG_SRC=0: External trigger soruce */
			      /* TRG_SEL=0: Timer chan 0 trigger source is selected*/
}

void delay_ms (volatile int ms){
	LPIT0_init(ms);           /* Initialize PIT0 for 1 second timeout  */
	while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK)) {} /* Wait for LPIT0 CH0 Flag */
	lpit0_ch0_flag_counter++;         /* Increment LPIT0 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

void NVIC_init_IRQs(void){
	S32_NVIC->ICPR[1] |= 1<<(61%32); // Clear any pending IRQ61
	S32_NVIC->ISER[1] |= 1<<(61%32); // Enable IRQ61
	S32_NVIC->IP[61] =0xB; //Priority 11 of 15
}

void PORTC_IRQHandler(void){

	PORTC->PCR[11] &= ~(0x01000000); // Port Control Register ISF bit '0' set
	PORTC->PCR[12] &= ~(0x01000000); // Port Control Register ISF bit '0' set
	PORTC->PCR[13] &= ~(0x01000000); // Port Control Register ISF bit '0' set

	//PORTC_Interrupt State Flag Register Read
	if((PORTC->ISFR & (1<<11)) != 0){
		MODE = 'D';
	}
	else if((PORTC->ISFR & (1<<12)) != 0){
		MODE = 'N';
	}
	else if((PORTC->ISFR & (1<<13)) != 0){
		MODE = 'R';
	}

	PORTC->PCR[11] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTC->PCR[12] |= 0x01000000; // Port Control Register ISF bit '1' set
	PORTC->PCR[13] |= 0x01000000; // Port Control Register ISF bit '1' set
}
// PWM 설정
void FTM_init (void){

	//FTM0 clocking
	PCC->PCCn[PCC_FTM0_INDEX] &= ~PCC_PCCn_CGC_MASK;		//Ensure clk diabled for config
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(0b010)		//Clocksrc=1, 8MHz SIRCDIV1_CLK
					| PCC_PCCn_CGC_MASK;		//Enable clock for FTM regs
	FTM0->SC = FTM_SC_PWMEN1_MASK	//Enable PWM channel 1output
		| FTM_SC_PWMEN0_MASK //채널 0 추가 
		| FTM_SC_PS(0);								//TOIE(timer overflow Interrupt Ena) = 0 (deafault)
	FTM0->MOD = 8000-1;									//FTM0 counter final value (used for PWM mode)
	FTM0->CNTIN = FTM_CNTIN_INIT(0);
	FTM0->CONTROLS[1].CnSC |=FTM_CnSC_MSB_MASK;
	FTM0->CONTROLS[1].CnSC |=FTM_CnSC_ELSA_MASK;			/* FTM0 ch1: edge-aligned PWM, low true pulses 	*/
	//채널 0추가
	FTM0->CONTROLS[0].CnSC |=FTM_CnSC_MSB_MASK;
	FTM0->CONTROLS[0].CnSC |=FTM_CnSC_ELSA_MASK;
}

void FTM0_CH1_PWM (int i){//uint32_t i){
	FTM0->CONTROLS[1].CnV = i;//8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	//start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)
	FTM0->SC|=FTM_SC_CLKS(3);
}

void FTM0_CH0_PWM (int i){//uint32_t i){
	FTM0->CONTROLS[0].CnV = i;//8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	//start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)
	FTM0->SC|=FTM_SC_CLKS(3);
}

int main(void)
{
  uint32_t adcResultInMv=0;	/*< ADC0 Result in miliVolts */
	int D=0;
	WDOG_disable();/* Disable Watchdog in case it is not done in startup code */
	PORT_init();            /* Configure ports */
	SOSC_init_8MHz();        /* Initialize system oscilator for 8 MHz xtal */
	SPLL_init_160MHz();     /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz();  /* Init clocks: 80 MHz sysclk & core, 40 MHz bus, 20 MHz flash */
	NVIC_init_IRQs(); /*Interrupt Pending, Endable, Priority Set*/
  FTM_init();
	ADC_init();
  
	Dtime = 500; // Delay Reset Value
  
	while(1){ /* Loop Start*/
    convertAdcChan(13);                   /* Convert Channel AD12 to pot on EVB 	*/
		while(adc_complete()==0){}            /* Wait for conversion complete flag 	*/
		adcResultInMv = read_adc_chx();       /* Get channel's conversion results in mv */
		D=adcResultInMv*1.6;						/* 5000*1.6=8000*/
		if(D>6000) D=8000;
    
		delay_ms(1);
		if(MODE == 'D'){
		PTD->PCOR |= 1<<0; //Blue LED
		PTD->PSOR |= 1<<15|1<<16;
    FTM0_CH1_PWM(D);
		}
		else if(MODE == 'R'){
		PTD->PCOR |= 1<<15; //Red LED
		PTD->PSOR |= 1<<0|1<<16;
    FTM0_CH0_PWM(D);
		}
		else{
		PTD->PCOR |= 1<<16;// Green LED
		PTD->PSOR |= 1<<15|1<<0;
		}
	}
}
