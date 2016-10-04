#ifndef SEV_SEG_H_H_INCLUDED
#define SEV_SEG_H_H_INCLUDED

#include <inttypes.h>

#define EN 	(0x80)

extern void SEVSEG_mpx();
extern void SEVSEG_putdigit(uint8_t val, uint8_t n);
extern void SEVSEG_putnumber(uint16_t val);


#endif // SEV_SEG_H_H_INCLUDED
