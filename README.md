## 사용할 I/O
1. DC 모터
2. potentiometer
3. switch(button)
4. 7-seg
5. 초음파센서
6. 부저(혹은 피에조 부저)

## 코드 구성(pseudo)
potentiometer 값 읽기 함수

IRQ핸들러 함수

main()
while()
- if([Drive] 모드 & (전방 거리> ??)Z){
  - poten 값만큼 모터에 인가
  }
- 7-seg 표기

```c
void PORT_init(){
}
// function for changing mode
void PORTC_IRQHandler(void){
}
void Seg_out(int number){
}
void FTM0_CH1_PWM (int i){//uint32_t i){
}
void delay_us(volatile int ms){
}
//read potentiometer value and covert A to D 
int DC_val(){
}
```
## 사용 포트 번호
7 segment
- 포트D 1~7 7seg FND 연결
- 포트D 8~11 FND select 연결

인터럽트
- 포트C 11~13 스위치 연결
  - 드라이브 모드 설정할 예정 D N R

DC모터
- 전진, 후진부 두 핀 사용할 예정
- 아마 ground도 연결할듯?

초음파 센서
- VCC, GND, TRIG, ECHO 네 핀 사용

potentiometer 
- ADC 13 채널(포트C 15번 핀)로 변환 예정,,,

부저
- VCC, GND 
피에조부저
- VCC, GND 연결하고 PWM으로 해야하나? 아니면 부저만 사용하기 


 
