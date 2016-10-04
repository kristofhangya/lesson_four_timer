#include "sev_seg_h.h"
#include <avr/io.h>

static uint8_t digit[5] = {0};
static uint8_t digit_cnt =0;

void SEVSEG_mpx(){

	 PORTA = (EN|(digit_cnt<<4)|digit[digit_cnt]);
	 digit_cnt = (digit_cnt+1)%5;

}

void SEVSEG_putdigit(uint8_t val, uint8_t n)
{
	if (n>3) return;
	digit[n] = val;

}
void SEVSEG_putnumber(uint16_t val){

	digit[0]=val%10;
	digit[1]=(val/10)%10;
	digit[2]=(val/100)%10;
	digit[3]=(val/1000)%10;

}
void SEVSEG_digitshift(uint8_t val){

	if (val>=10) return;
	uint8_t i = 3;
	while (i) { digit[i] = digit[i-1]; i--;};
	digit[0] = val ;

}
