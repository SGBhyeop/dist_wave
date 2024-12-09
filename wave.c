#include "device_registers.h"
#include "clocks_and_modes.h"
// 트리거 핀으로 신호를 내보낼 것임
#define TRIGGER_PIN 3   // Trigger 핀 (PTE3)
#define ECHO_PIN0 0      // Echo 핀0 (PTE1)
#define ECHO_PIN1 1 //Echo 핀1

volatile uint32_t start_time = 0;  // Rising edge 발생 시간
volatile uint32_t end_time = 0;    // Falling edge 발생 시간
volatile uint32_t pulse_width = 0; // Rising to Falling 사이의 시간

void PORT_init(void)
{
    // 특정 핀 (예: PTD0)을 입력, ECHO 핀에 rising, falling 들어온다 가정 
    PCC->PCCn[PCC_PORTE_INDEX] |= PCC_PCCn_CGC_MASK;  // Port D의 클럭을 활성화
    PORTE->PCR[ECHO_PIN0] = PORT_PCR_MUX(1) | (9<16);  // PTD0: GPIO로 설정, rising edge에서 인터럽트 발생
    PORTE->PCR[ECHO_PIN1] = PORT_PCR_MUX(1) | (10<16);  // PTD0: GPIO로 설정, falling edge에서 인터럽트 발생
    PTE->PDDR &= ~(1<<ECHO_PIN0);  // PTD0를 입력으로 설정
    PTE->PDDR &= ~(1<<ECHO_PIN1);  // PTD0를 입력으로 설정
}

void WDOG_disable (void)
{
    WDOG->CNT=0xD928C520;     /* Unlock watchdog 		*/
    WDOG->TOVAL=0x0000FFFF;   /* Maximum timeout value 	*/
    WDOG->CS = 0x00002100;    /* Disable watchdog 		*/
}

void NVIC_init_IRQs(void)
{
    // 포트 D의 인터럽트 활성화
    S32_NVIC->ICPR[1] |= 1<<(61%32); // pending 클리어 
    S32_NVIC->ISER[0] |= (1 << PORTE_IRQn);  // PORTD IRQ를 활성화
    S32_NVIC->IP[PORTD_IRQn] = 0x03;  // 우선순위 설정
}

void PORTE_IRQHandler(void)
{
    PORTE->PCR[ECHO_PIN0] &= ~(0x01000000); // Port Control Register ISF bit '0' set
	PORTE->PCR[ECHO_PIN1] &= ~(0x01000000); // Port Control Register ISF bit '0' set

	//PORTC_Interrupt State Flag Register Read
	if((PORTC->ISFR & (1<<ECHO_PIN0)) != 0){ //rising edge 일떄
	}
	else if((PORTC->ISFR & (1<<ECHO_PIN1)) != 0){ //falling 일때
	}
    // 인터럽트 플래그 클리어
	PORTE->PCR[ECHO_PIN0] |= 0x01000000; // ISF 비트 1
	PORTE->PCR[ECHO_PIN1] |= 0x01000000; //ISF 비트 1
}

void FTM0_init(void)
{
	// FTM0 타이머 클럭 설정
	PCC->PCCn[PCC_FTM0_INDEX] &= ~PCC_PCCn_CGC_MASK;  // 클럭 비활성화
	PCC->PCCn[PCC_FTM0_INDEX] |= PCC_PCCn_PCS(0b010)  // 클럭 소스 설정 (8 MHz)
				| PCC_PCCn_CGC_MASK;  // 클럭 활성화
	
	// FTM0 설정: 16-bit 카운터 모드
	FTM0->SC = FTM_SC_PS(0);  // 프리스케일러 1 (최대 주파수)
	FTM0->MOD = 0xFFFF;  // 16-bit 모드에서 최대 값 (주기 설정)
}

int main(void)
{
	WDOG_disable();  // Watchdog 비활성화
	PORT_init();     // 포트 설정
	FTM0_init();     // FTM0 초기화
	NVIC_init_IRQs();  // 인터럽트 초기화
	
	while(1)
	{
	// 메인 루프에서는 아무 작업도 하지 않고 기다림
	// 인터럽트에서 신호를 처리
	}
}
