
##사용할 I/O
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

 
