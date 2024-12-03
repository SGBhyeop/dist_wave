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

L9110 DC모터
- 전진, 후진부 두 핀 사용할 예정
- 아마 ground도 연결할듯?
- PWM을 출력으로 내야하는데 어디 연결해야하지? 부록9 확인하기
- 포트C 0번핀 하면 FTM0 CH0 사용
- 포트C 1번핀 하면 FTM0 CH1 사용 (기존 포트D 16번핀 FTM0 CH1)

초음파 센서
- VCC, GND, TRIG, ECHO 네 핀 사용
- HC-SR04 초음파 센서를 이용하려고 해. 센서의 Trigger 를 10us 이상 high로 유지시키면 초음파가 발사되고, 이를 반복하며 초음파를 발사해야해. 주의할 점은 60ms 이상의 시간 후에 초음파를 발사하도록 해야해. 센서의 Echo 에서는 초음파를 발사하는때에 high로 올라가고 초음파가 돌아왔을 때 low로 떨어져. 때문에 rising과 falling일 때 시간을 획득하도록 설계할거야. 
이후에 소리의 속도는 340m/s 임을 이용해 거리를 계산하도록 할거야.

potentiometer 
- ADC 13 채널(포트C 15번 핀)로 변환 예정,,,

부저
- VCC, GND
- 포트C 5번핀
피에조부저
- VCC, GND 연결하고 PWM으로 해야하나? 아니면 부저만 사용하기 

![image](https://github.com/user-attachments/assets/bb0c5724-33f6-4896-b18c-b35626b1e3a8)

 
