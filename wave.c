#include "device_registers.h"
#include "MKL25Z4.h"  // MCU에 맞는 헤더 파일을 포함해야 합니다.

#define TRIG_PIN 0  // 트리거 핀, GPIO 포트 및 핀 번호에 맞게 설정
#define ECHO_PIN 1  // 에코 핀, GPIO 포트 및 핀 번호에 맞게 설정

// 트리거 핀에 10us 펄스를 보내 초음파 발사
void send_trigger_pulse(void) {
    GPIOB->PDOR |= (1 << TRIG_PIN); // 트리거 핀 High (1)
    for (volatile int i = 0; i < 100; i++) {} // 짧은 딜레이
    GPIOB->PDOR &= ~(1 << TRIG_PIN); // 트리거 핀 Low (0)
}

// 에코 핀에서 높을 때의 시간을 측정하여 거리 계산
uint32_t measure_distance(void) {
    uint32_t pulse_width = 0;

    // 에코 핀이 High로 변할 때까지 대기
    while (!(GPIOB->PDIR & (1 << ECHO_PIN))) {} // 에코 핀이 High로 변할 때까지 대기
    uint32_t start_time = SysTick->VAL; // 시작 시간 저장

    // 에코 핀이 Low로 변할 때까지 대기
    while (GPIOB->PDIR & (1 << ECHO_PIN)) {} // 에코 핀이 Low로 변할 때까지 대기
    uint32_t end_time = SysTick->VAL; // 끝 시간 저장

    // 초음파의 왕복 시간 계산 (SysTick을 사용하여 시간 측정)
    pulse_width = start_time - end_time;

    // 왕복 시간을 이용하여 거리를 계산 (343 m/s, 1cm = 1us에서 0.0343cm/us)
    uint32_t distance = (pulse_width * 0.0343) / 2; // cm 단위로 계산

    return distance;
}

int main(void) {
    uint32_t distance = 0;

    // 포트 및 핀 설정 (예: GPIO 포트 설정)
    // GPIO 설정을 사용해 TRIG_PIN을 출력, ECHO_PIN을 입력으로 설정해야 함

    while (1) {
        send_trigger_pulse();  // 초음파 발사
        distance = measure_distance();  // 거리 측정
        // 측정된 거리 출력 (예: UART, LCD, LED 등으로 출력)
    }

    return 0;
}
