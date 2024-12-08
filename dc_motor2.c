#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"
int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */

void LPIT0_init (uint32_t delay)
{
	uint32_t timeout;
	
	PCC->PCCn[PCC_LPIT_INDEX] = PCC_PCCn_PCS(6);    /* Clock Src = 6 (SPLL2_DIV2_CLK)*/
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clk to LPIT0 regs       */

	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;  /* DBG_EN-0: Timer chans stop in Debug mode */
	
	timeout=delay* 40;
	LPIT0->TMR[0].TVAL = timeout;      /* Chan 0 Timeout period: 40M clocks */
	LPIT0->TMR[0].TCTRL |= LPIT_TMR_TCTRL_T_EN_MASK;
}

void delay_us (volatile int us){
	LPIT0_init(us);           /* Initialize PIT0 for 1 second timeout  */
	while (0 == (LPIT0->MSR & LPIT_MSR_TIF0_MASK)) {} /* Wait for LPIT0 CH0 Flag */
	lpit0_ch0_flag_counter++;         /* Increment LPIT0 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; /* Clear LPIT0 timer flag 0 */
}

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

void PORT_init (void)
{
	/*D16 -> C1 change  */
	PCC->PCCn[PCC_PORTC_INDEX ]|=PCC_PCCn_CGC_MASK;   /* Enable clock for PORTD */
	PORTC->PCR[1]|=PORT_PCR_MUX(2);    /* Port D16을 C1로 바꿈 FTM0CH1 */
	PORTC->PCR[0]|=PORT_PCR_MUX(2); /*아마도 포트C 0핀을 출력으로 */
}

int main(void)
{
	uint32_t adcResultInMv=0;	/*< ADC0 Result in miliVolts */
	int D=0;
	SOSC_init_8MHz();      /* Initialize system oscillator for 8 MHz xtal */
	SPLL_init_160MHz();    /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz(); /* Init clocks: 80 MHz SPLL & core, 40 MHz bus, 20 MHz flash */
	FTM_init();
	ADC_init();            /* Init ADC resolution 12 bit			*/
	PORT_init();           /* Configure ports */
	for(;;)
	{
		convertAdcChan(13);                   /* Convert Channel AD12 to pot on EVB 	*/
		while(adc_complete()==0){}            /* Wait for conversion complete flag 	*/
		adcResultInMv = read_adc_chx();       /* Get channel's conversion results in mv */
		D=adcResultInMv*1.6;						/* 5000*1.6=8000*/
		if(D>6000) D=8000;
		FTM0_CH1_PWM(D);
		//FTM0_CH0_PWM(D);
	}
}
