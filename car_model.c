#include "device_registers.h"
#include "clocks_and_modes.h"
#include "ADC.h"

char MODE = 'N';
unsigned int Dtime = 0; /* Delay Time Setting Variable*/
unsigned int j=0; /*FND select pin index */
unsigned int FND_DATA[10]={0x7E, 0x0C,0xB6,0x9E,0xCC,0xDA,0xFA,0x4E,0xFE,0xCE};
unsigned int FND_SEL[4]={0x0100,0x0200,0x0400,0x0800}; 

void PORT_init(){ // 7-seg 입출력설정, 부저, DC모터, 인터럽트, 초음파 설정 
    PCC->PCCn[PCC_PORTC_INDEX]|=PCC_PCCn_CGC_MASK;
    PCC->PCCn[PCC_PORTD_INDEX]|=PCC_PCCn_CGC_MASK; 

    /* 7-seg setting */
    //set PDDR
    PTD->PDDR |= 1<<1|1<<2|1<<3|1<<4|1<<5|1<<6|1<<7 		/* Port D1~7: 7 seg FND Data Direction = output */
				     |1<<8|1<<9|1<<10|1<<11; //FND select 

    PORTD->PCR[1]  = PORT_PCR_MUX(1);	/* Port D1: MUX = GPIO  */
    PORTD->PCR[2]  = PORT_PCR_MUX(1);	/* Port D2: MUX = GPIO  */
    PORTD->PCR[3]  = PORT_PCR_MUX(1);	/* Port D3: MUX = GPIO  */
    PORTD->PCR[4]  = PORT_PCR_MUX(1);	/* Port D4: MUX = GPIO  */
    PORTD->PCR[5]  = PORT_PCR_MUX(1);	/* Port D5: MUX = GPIO  */
    PORTD->PCR[6]  = PORT_PCR_MUX(1);	/* Port D6: MUX = GPIO  */
    PORTD->PCR[7] = PORT_PCR_MUX(1); 	/* Port D7: MUX = GPIO */

    PORTD->PCR[8] = PORT_PCR_MUX(1); 	/* Port D8: MUX = GPIO */
    PORTD->PCR[9] = PORT_PCR_MUX(1); 	/* Port D9: MUX = GPIO */
    PORTD->PCR[10] = PORT_PCR_MUX(1); 	/* Port D10: MUX = GPIO */
    PORTD->PCR[11] = PORT_PCR_MUX(1); 	/* Port D11: MUX = GPIO */
    /* potentiometer setting */
    PTD->PCR[16] |= PORT_PCR_MUX(2); // PWM으로 설정
    /* dc motor setting */
    PTC->PDDR |= 1<<1; 
    PORTC->PCR[1] = PORT_PCR_MUX(1);
    /* drive mode inturrupt setting */
    PTC->PDDR &= ~(1<<11|1<<12|1<<13); //11,12,13번핀 input 설정 D N R 모드 설정 
    PORTC->PCR[11] = PORT_PCR_MUX(1); //GPIO로 설정
    PORTC->PCR[12] = PORT_PCR_MUX(1);
    PORTC->PCR[13] = PORT_PCR_MUX(1); 
    PORTC->PCR[11] |=(10<<16); //1010 으로 IRQC 조절, falling-edge 트리거로
    PORTC->PCR[12] |=(10<<16);
    PORTC->PCR[13] |=(10<<16);
    /* ultrasonic sensor setting */
}
// function for changing mode
void PORTC_IRQHandler(void){

	PORTC->PCR[11] &= ~(0x01000000); // 플래그 비트 0으로 만들기 Port Control Register ISF bit '0' set
    PORTC->PCR[12] &= ~(0x01000000);
    PORTC->PCR[13] &= ~(0x01000000);

	//PORTC_Interrupt State Flag Register Read
	if((PORTC->ISFR & (1<<11)) != 0){
		MODE='D';
	}
    else if((PORTC->ISFR & (1<<12)) != 0){
		MODE='N';
	}
    else if((PORTC->ISFR & (1<<13)) != 0){
		MODE='R';
	}

	PORTC->PCR[11] |= 0x01000000; // Port Control Register ISF bit '1' set
    PORTC->PCR[12] |= 0x01000000;
    PORTC->PCR[13] |= 0x01000000;
}
void Seg_out(int number){
    Dtime = 1000;
	j = 0;

	num3=(number/1000)%10; num2=(number/100)%10; num1=(number/10)%10; num0= number%10;

	// 1000
	PTD->PSOR = FND_SEL[j]; PTD->PCOR =0x7f; PTD->PSOR = FND_DATA[num3];
    delay_us(Dtime);
	PTD->PCOR = 0xfff; j++;

	// 100
	PTD->PSOR = FND_SEL[j]; PTD->PCOR =0x7f; PTD->PSOR = FND_DATA[num2];
    delay_us(Dtime);
	PTD->PCOR = 0xfff; j++;

	// 10
	PTD->PSOR = FND_SEL[j]; PTD->PCOR =0x7f; PTD->PSOR = FND_DATA[num1];
    delay_us(Dtime);
    PTD->PCOR = 0xfff; j++;

	// 1
	PTD->PSOR = FND_SEL[j]; PTD->PCOR =0x7f; PTD->PSOR = FND_DATA[num0];
    delay_us(Dtime);
	PTD->PCOR = 0xfff; j=0;
}

//buzzer
void Set_buzzer(){
    // pin 두 개


}
//p-buzzer, when reverse driving
void Set_pbuzzer(int i){
    if(i){
        // 
    }

}

//read from ultrasonic sensor
void dist_val(){
}

/* PWM setting */
void FTM_init (void){

	//FTM0 clock 선택
	PCC->PCCn[PCC_FTM0_INDEX] &= ~PCC_PCCn_CGC_MASK;		
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(0b010)| PCC_PCCn_CGC_MASK;		

    //FTM0 Initialization
	FTM0->SC = FTM_SC_PWMEN1_MASK|FTM_SC_PS(0);								
	FTM0->MOD = 8000-1;	
	FTM0->CNTIN = FTM_CNTIN_INIT(0);
	FTM0->CONTROLS[1].CnSC |=FTM_CnSC_MSB_MASK;
	FTM0->CONTROLS[1].CnSC |=FTM_CnSC_ELSA_MASK;	
}
void FTM0_CH1_PWM (int i){//uint32_t i){ 전진부 연결
	FTM0->CONTROLS[1].CnV = i;//8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	//start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)
	FTM0->SC|=FTM_SC_CLKS(3);
}
void FTM0_CH0_PWM (int i){ //후진부 연결
    FTM0->CONTROLS[0].CnV = i;//8000~0 duty; ex(7200=> Duty 0.1 / 800=>Duty 0.9)
	//start FTM0 counter with clk source = external clock (SOSCDIV1_CLK)
	FTM0->SC|=FTM_SC_CLKS(3);
}

void delay_us(volatile int ms){
    
}

int main(){
    uint32_t adcResultInMv=0;	/*< ADC0 Result in miliVolts */
    int vel;
    int dist;
    SOSC_init_8MHz();      /* Initialize system oscillator for 8 MHz xtal */
	SPLL_init_160MHz();    /* Initialize SPLL to 160 MHz with 8 MHz SOSC */
	NormalRUNmode_80MHz(); /* Init clocks: 80 MHz SPLL & core, 40 MHz bus, 20 MHz flash */
	FTM_init();
	ADC_init();            /* Init ADC resolution 12 bit			*/
	PORT_init(); 

    while(1){
        //read potentiometer value and covert A to D 
        convertAdcChan(12);                   /* Convert Channel AD12 to pot on EVB 	*/
        while(adc_complete()==0){}            /* Wait for conversion complete flag 	*/
        adcResultInMv = read_adc_chx();
        dist = dist_val();        

        if(dist<??){
            Set_buzzer(1);
            FTM0_CH0_PWM(0);
            FTM0_CH1_PWM(0);
            Seg_out(0);
        }
        else {
            Set_buzzer(0);
            switch(MODE){
                case 'D':
                    FTM0_CH1_PWM(adcResultInMv*1.6);
                    FTM0_CH0_PWM(0);
                    // applying vel to forward side of DC motor (maybe with PWM)
                    Seg_out(adcResultInMv);
                    break;
                case 'N':
                    FTM0_CH1_PWM(0);
                    // stop the DC motor
                    Seg_out(0);
                    break;
                case 'R':
                    FTM0_CH0_PWM(adcResultInMv*1.6); // need to change pwm channel
                    FTM0_CH1_PWM(0);
                    // applying vel to reverse side of DC motor 
                    Seg_out(adcResultInMv);
                    break;

            }
        }
        
    }
}
