#include "device_registers.h"            /* include peripheral declarations S32K144 */
#include "clocks_and_modes.h"

#define TRIGGER_PIN 3
#define ECHO_PIN0 0 
#define ECHO_PIN1 1

int lpit0_ch0_flag_counter = 0; /*< LPIT0 timeout counter */
int lpit0_ch1_flag_counter = 0; /*< LPIT0 timeout counter */
volatile uint32_t start_time = 0;  // Rising edge 발생 시간
volatile uint32_t end_time = 0;    // Falling edge 발생 시간
volatile uint32_t pulse_width = 0;
unsigned int num, count = 0;

void WDOG_disable (void)
{
	WDOG->CNT=0xD928C520;     /* Unlock watchdog       */
	WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value    */
	WDOG->CS = 0x00002100;    /* Disable watchdog       */
}

void PORT_init (void)
{
	// 특정 핀 (예: PTE0)을 입력, ECHO 핀에 rising, falling 들어온다 가정 
	PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK;  // Port E의 클럭을 활성화
	PORTE->PCR[ECHO_PIN0] = PORT_PCR_MUX(1) | (9<16);  // PTE0: GPIO로 설정, rising edge에서 인터럽트 발생
	PORTE->PCR[ECHO_PIN1] = PORT_PCR_MUX(1) | (10<16);  // PTE1: GPIO로 설정, falling edge에서 인터럽트 발생
	PTE->PDDR &= ~(1<<ECHO_PIN0);  // PTE0를 입력으로 설정
	PTE->PDDR &= ~(1<<ECHO_PIN1);  // PTE1를 입력으로 설정
	
	PORTE->PCR[ECHO_PIN1] = PORT_PCR_MUX(1);  // PTE1: GPIO로 설정, falling edge에서 인터럽트 발생
	PTE->PDDR |= (1<<TRIGGER_PIN);  // PTE3를 출력으로 설정
	
}

void NVIC_init_IRQs(void)
{
	S32_NVIC->ICPR[1] |= 1<<(63%32); // Clear any pending IRQ61
	S32_NVIC->ISER[1] |= 1<<(63%32); // Enable IRQ61
	S32_NVIC->IP[61] =0xB; //Priority 11 of 15
	
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
	
	LPIT0->TMR[1].TVAL = 40000;      // 1ms 마다 인터럽트 발생? 
	LPIT0->TMR[1].TCTRL = 0x00000001;
}

void PORTE_IRQHandler(void)
{
	PORTE->PCR[ECHO_PIN0] &= ~(0x01000000); // Port Control Register ISF bit '0' set
	PORTE->PCR[ECHO_PIN1] &= ~(0x01000000); // Port Control Register ISF bit '0' set
	
	//PORTC_Interrupt State Flag Register Read
	if((PORTE->ISFR & (1<<ECHO_PIN0)) != 0){ //rising edge 일떄
		start_time = num;
	}
	else if((PORTE->ISFR & (1<<ECHO_PIN1)) != 0){ //falling 일때
		if(num>start_time)
			pulse_width = num - start_time;
		else
			pulse_width = 0xFFFFFFFF + num - start_time + 1;
	}
	// 인터럽트 플래그 클리어
	PORTE->PCR[ECHO_PIN0] |= 0x01000000; // ISF 비트 1
	PORTE->PCR[ECHO_PIN1] |= 0x01000000; //ISF 비트 1
}

void LPIT0_Ch1_IRQHandler (void)
{	  /* delay counter */
	count++;
	if(count>65){
		PTE->PSOR |= (1<<TRIGGER_PIN);
		int delay = num;
		while(num>delay+10){}
		PTE->PCOR |= (1<<TRIGGER_PIN);
	}

	lpit0_ch1_flag_counter++;         /* Increment LPIT1 timeout counter */
	LPIT0->MSR |= LPIT_MSR_TIF1_MASK;  /* Clear LPIT0 timer flag 1 */
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
	
	while(1){
		if(pulse_width > 0){
			float dist = 0.01715* pulse_width; 
			pulse_width = 0;
		}
	}
	// if (dist<20) // 너무 가까우면 
	// 	;
	// else 
}
