/*
#include "IOBlink.h"
IOBlink _IoB(LED_BUILTIN);
void setup() {
  _IoB.attach_ms(111,800,200);
  //_IoB.attach_ms(111,1,0); // luôn on
  //_IoB.attach_ms(111,0,1); // luôn off
}

void loop() {
  _IoB.Blink();
}

*/

#ifndef	_IOBLINK_H
#define _IOBLINK_H
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include "TimeOutEvent.h"

#define TaskIoBDelay               0
#define TaskIoBStop                1
#define TaskIoBOn                  2
#define TaskIoBOff                 3

/*
Trong C++

- Cach su dung ham class B trong class D
- Link hay hoc C++ cap cao
- http://vietjack.com/cplusplus/tinh_ke_thua_trong_cplusplus.jsp

file .h
class D : public B, public B, public ...{
};
class D : B, C, ...{	
};
file .cpp
D::D()
:B
{}


- Su khac biet giua co public va khong public: 
- https://stackoverflow.com/questions/1563765/use-of-public-in-a-derived-class-declaration
- Neu co public thi D thua huong tat ca cua B. Nghia la B khai bao sao thi D nhu vay
- Neu khong co public thi B se tro nen private cua D

*/
class IOBlink : public TimeOutEvent {
public:
	IOBlink(short _IO = 2, bool Lever = LOW);
	void attach_ms(uint8_t Count, uint16_t DelayOn, uint16_t DelayOff);
	void Blink(void);	

protected:
	const short _IOPIN;
	uint8_t TaskIoB = TaskIoBStop;
	uint16_t IoBDelayOff = 200;
	uint16_t IoBDelayOn = 800;
	uint8_t IoBPre = 1;
	uint8_t IoBCount = 0;
	bool On_Status = LOW;
	bool Off_Status = HIGH;
};
#endif
