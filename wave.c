#include "device_registers.h"
#include "clocks_and_modes.h"

#define TRIGGER_PIN 0   // Trigger 핀 (PTD0)
#define ECHO_PIN 1      // Echo 핀 (PTD1)

volatile uint32_t start_time = 0;  // 초음파 발사 후 Echo 핀 rising edge 시점
volatile uint32_t end_time = 0;    // Echo 핀 falling edge 시점
volatile uint32_t pulse_duration = 0;  // Echo 핀에서의 high 상태 지속 시간
volatile uint32_t last_trigger_time = 0; // 마지막 초음파 발사 시간 (60ms 대기)

void PORT_init(void)
{
    // Trigger 핀 출력, Echo 핀 입력
    PTD->PDDR |= (1 << TRIGGER_PIN);  // Trigger 핀 설정 (출력)
    PTD->PDDR &= ~(1 << ECHO_PIN);    // Echo 핀 설정 (입력)

    // Trigger 핀 초기화 (low 상태)
    PTD->PCOR |= (1 << TRIGGER_PIN);  // Trigger 핀 low로 설정

    // Echo 핀의 Interrupt 설정 (rising edge 및 falling edge)
    PORTD->PCR[ECHO_PIN] |= PORT_PCR_MUX(1);  // GPIO 모드로 설정
    PORTD->PCR[ECHO_PIN] |= (10 << 16);  // Interrupt on rising and falling edge
}

void PORTD_IRQHandler(void)
{
    if (PORTD->ISFR & (1 << ECHO_PIN)) {
        if ((PTD->PDIR & (1 << ECHO_PIN)) != 0) {  // rising edge
            start_time = LPIT0->TMR[0].TCR;  // Timer를 사용하여 시간을 측정
        } else {  // falling edge
            end_time = LPIT0->TMR[0].TCR;
            pulse_duration = end_time - start_time;  // 초음파 왕복 시간 계산
            // 초음파 거리를 계산 (340 m/s = 0.034 cm/μs)
            uint32_t distance = (pulse_duration * 0.034) / 2;
            // 거리 계산 후 처리 (예: 결과를 출력하거나 저장)
        }
    }
    PORTD->ISFR |= (1 << ECHO_PIN);  // Interrupt Flag 클리어
}

void trigger_pulse(void)
{
    // 60ms 이상의 시간 간격을 두고 초음파 발사
    uint32_t current_time = LPIT0->TMR[0].TCR;
    if (current_time - last_trigger_time >= 60000) {  // 60ms 경과 후
        PTD->PSOR |= (1 << TRIGGER_PIN);  // Trigger 핀을 high로 설정
        delay_us(10);  // 10us 동안 high로 유지
        PTD->PCOR |= (1 << TRIGGER_PIN);  // Trigger 핀을 low로 설정
        last_trigger_time = current_time;  // 마지막 발사 시간 기록
    }
}

void delay_us(uint32_t us)
{
    // 대기 시간: 마이크로초 단위
    uint32_t count = us * 40;  // 40MHz 시스템 클럭을 기준으로 대기
    while (count--) {
        __asm("NOP");  // No operation (빈 명령어)
    }
}

int main(void)
{
    WDOG_disable();     // Watchdog 비활성화
    PORT_init();        // 포트 초기화
    SOSC_init_8MHz();   // 8MHz 시스템 오실레이터 초기화
    SPLL_init_160MHz(); // SPLL을 160MHz로 초기화
    NormalRUNmode_80MHz();  // 시스템 클럭 80MHz, 코어 클럭 80MHz 설정

    while (1) {
        trigger_pulse();  // 초음파 발사
        delay_ms(50);     // 50ms 대기 후 반복
    }
}
