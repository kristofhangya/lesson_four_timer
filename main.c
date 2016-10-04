#define F_CPU (8000000UL)

#include <avr/io.h>
#include <avr/iom128.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#include "lcd.h"
#include "FIFO.h"
#include "USART.h"
#include "ad.h"
#include "sev_seg_h.h"

#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define I_SPEED 5

///GLOBAL VAR's
uint8_t buffer[16]  =   {0};
uint8_t L_SHIFT_en  =   0;
uint8_t ADC_en      =   0;
uint8_t PWM_en      =   0;
uint16_t PWM_duty0   =  50;
uint16_t PWM_duty1   =  50;
uint16_t PWM_duty2   =  50;

uint8_t glob_en     =   0;
uint8_t tick        =   0;
uint8_t tmp;

///FGV_DECLAR
void menu();
void cmd();
void adc();
void pwm();
void shift();

///INIT
void SYSTEM_init()
{

    ///PORT_init
    DDRA    =   0xFF;
    DDRB    =   0xF0;
    DDRG    =   0xFF;
    ///Timer0 OVERFLOW mode
    TCCR0   =   (1<<CS02) | (1<<CS01) | (1<<CS00);    ///~30Hz

    ///Timer1 PWM mode
    TCCR1A  =   (0<<COM1A0)     | (1<<COM1A1) |
                (0<<COM1B0)     | (1<<COM1B1) |
                (0<<COM1C0)     | (1<<COM1C1) |
                (0<<WGM10)      | (1<<WGM11);


    TCCR1B  =   (0 << ICNC1)    | (0 << ICES1) |
                (1 << WGM13)    | (1 << WGM12) |
                (1<<CS11)       | (1<<CS00);    ///F_CPU/64

    TCCR1C  =   0;
    ICR1    =   2499;                       ///F_CPU/64/2499 50Hz
    OCR1A   =   PWM_duty0;                  ///duty 0-2499
    OCR1B   =   PWM_duty1;                  ///duty 0-2499
    OCR1C   =   PWM_duty2;                  ///duty 0-2499

    ///Timer3 COMPARE mode
    TCCR3A  =   0;
    TCCR3B  =   (1<<WGM32) | (1<<CS32) | (1<<CS30);  ///F_CPU/1024 =~7812 Hz
    TCCR3C  =   0;
    OCR3A   =   31;                                   ///7812/31 = ~250Hz
    TIMSK   =   (1<<TOIE0);
    ETIMSK  =   (1<<OCIE3A);
}

int main()
{

    SYSTEM_init();
    USART_Init(MYUBRR);
    ADC_init();
    LCD_init();

    LCD_Puts("LCD_is_ON");
    sei();
    menu();

    while(1)
    {

        if(has_sen())
        {
            USART_get_sen(buffer);
            cmd();
        }
        if(glob_en)
        {
            shift();
            adc();
            pwm();
            glob_en =   0;
        }
        if(PING == 1)
        {
            L_SHIFT_en=~L_SHIFT_en;
            LCD_cmd(0x02);
            LCD_goto(0,0);
            LCD_Puts("Menu-shift   ");
            while(PING);
        }
        if(PING == 2)
        {
            PWM_en=~PWM_en;
            LCD_goto(0,0);
            LCD_Puts("PWM_on/off   ");
            while(PING);
        }
        if(PING == 8)
        {
            ADC_en=~ADC_en;
            LCD_goto(0,0);
            LCD_Puts("ADC_EN/DIS   ");
            while(PING);
        }
    }
    return 0;
}

ISR(TIMER0_OVF_vect)
{
    if(!glob_en)
    {
        if(tick == I_SPEED)
        {
            glob_en =   1;
            tick    =   0;
        }
        tick++;
    }
}

ISR(TIMER3_COMPA_vect)
{

    SEVSEG_mpx();
}

void menu()
{

    USART_Transmit(12);
    USART_Transmit_Puts("****MENU****");
    USART_Transmit_Puts("");            ///only /enter
    USART_Transmit_Puts("1. first");
    USART_Transmit_Puts("2. second");
    USART_Transmit_Puts("3. third");
    USART_Transmit_Puts("4. fourth");
    USART_Transmit_Puts("5. help");
    USART_Transmit('>');
}

void cmd()
{
    uint8_t element =   0;
    USART_Transmit(12);

    if(!(strcmp(buffer,"first"))    || (!(strcmp(buffer,"1"))))
    {
        element = 1;
    }
    if(!(strcmp(buffer,"second"))   || (!(strcmp(buffer,"2"))))
    {
        element = 2;
    }
    if(!(strcmp(buffer,"third"))    || (!(strcmp(buffer,"3"))))
    {
        element = 3;
    }
    if(!(strcmp(buffer,"fourth"))   || (!(strcmp(buffer,"4"))))
    {
        element = 4;
    }
    if(!(strcmp(buffer,"help"))     || (!(strcmp(buffer,"h"))))
    {
        element = 5;
    }

    switch(element)
    {
    case 1:
        USART_Transmit_Puts(" detected->LCD_shift_on/off");
        L_SHIFT_en=~L_SHIFT_en;
        LCD_cmd(0x02);                                          ///lcd home adress
        LCD_goto(0,0);
        LCD_Puts("Menu-shift   ");
        break;
    case 2:
        USART_Transmit_Puts(" detected->PWM_on/off");
        PWM_en=~PWM_en;
        LCD_goto(0,0);
        LCD_Puts("PWM_on/off   ");
        break;
    case 3:
        USART_Transmit_Puts(" detected->empty3");

        LCD_goto(0,0);
        LCD_Puts("Menu-third   ");
        break;
    case 4:
        USART_Transmit_Puts(" detected->ADC_enable/disable");
        ADC_en=~ADC_en;
        LCD_goto(0,0);
        LCD_Puts("ADC_EN/DIS   ");
        break;
    case 5:
        USART_Transmit_Puts(" detected->help");
        menu();
        LCD_goto(0,0);
        LCD_Puts("Help_menu    ");
        break;
    default:
        USART_Transmit_Puts(" undetected->error");
        LCD_goto(0,0);
        LCD_Puts("Fail_cmd     ");
        break;
    }
    USART_Transmit('>');
}

void adc()
{

    uint16_t temp;
    char str[15]    =   {0};

    if(ADC_en)
    {
        ADC_measure(0,&temp);
        itoa(temp/4,str,10);
        LCD_goto(3,0);
        LCD_Puts("Temp: ...C :");
        LCD_Puts(str);
        SEVSEG_putnumber(temp/4);
    }
    else
    {
        LCD_goto(3,0);
        LCD_Puts("               ");
    }
}

void pwm()
{
    if(PWM_en)
    {
        OCR1A   =   PWM_duty0;                  ///duty 0-2499
        OCR1B   =   PWM_duty1;                  ///duty 0-2499
        OCR1C   =   PWM_duty2;                  ///duty 0-2499
    }
    else
    {
        OCR1A   =   0;                  ///duty 0-2499
        OCR1B   =   0;                  ///duty 0-2499
        OCR1C   =   0;                  ///duty 0-2499
    }
}
void shift()
{
    if(L_SHIFT_en)
    {
        LCD_shift();
    }
}

