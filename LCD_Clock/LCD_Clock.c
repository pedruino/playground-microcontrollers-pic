#define HIGH 1
#define LOW 0
#define SEPARATOR ':'
//LCD Setup
sbit LCD_RS at RE1_bit;
sbit LCD_EN at RE0_bit;
sbit LCD_D4 at RB4_bit;
sbit LCD_D5 at RB5_bit;
sbit LCD_D6 at RB6_bit;
sbit LCD_D7 at RB7_bit;

sbit LCD_RS_Direction at TRISE1_bit;
sbit LCD_EN_Direction at TRISE0_bit;
sbit LCD_D4_Direction at TRISB4_bit;
sbit LCD_D5_Direction at TRISB5_bit;
sbit LCD_D6_Direction at TRISB6_bit;
sbit LCD_D7_Direction at TRISB7_bit;

//vars
int hour, minute, second = 0;
int timer_milliseconds =0;
unsigned int blink = 1;
char txt[4];

void delay(unsigned int milliseconds);

void setup(){
    //SETUP microcontrolador
    TRISA = 48; //RA4 e RA5 com entrada
    TRISB = 15; //RB0 ..RB3
    TRISC = 0; //saída
    TRISD = 0; //saída
    TRISE = 0; //saída
    //obrigatório
    INTCON2 = 0; //liga o resistor pull up
    ADCON1 = 15; //Portas digitais
    CMCON = 7; //desliga comparadores
    //coeficiente de cagaço (garantir que tudo incia desligado)
    PORTA = 0; //
    PORTB = 0; //
    PORTC = 0; //
    PORTD = 0; //
    PORTE = 0; //

    //LCD initialize
    lcd_init();
    lcd_cmd(_LCD_CLEAR);
    lcd_cmd(_LCD_CURSOR_OFF);
    
    //inicializa vars
    hour = 0;
    minute = 0;
    second = 0;
    timer_milliseconds = 0;
}



void increment_hour(){
    if(hour < 23){
        hour++;
    }else{
        hour = 0;
    }
}
void increment_minute(){
    if(minute < 59){
        minute++;
    }else{
        minute = 0;
        increment_hour();
    }
}

void increment_second(){
    if(second < 59){
        second++;
    }else{
        second = 0;
        increment_minute();
    }
}

void loop(){
    if(RB0_bit == LOW){
        delay(100);
        if(RB0_bit == LOW){
            increment_hour();
        }
    }
    if(RB1_bit == LOW){
        delay(100);
        if(RB1_bit == LOW){
            increment_minute();
        }
    }
    if(RB2_bit == LOW){
        delay(1);
        if(RB2_bit == LOW){
            increment_second();
        }
    }

    bytetostr(second, txt);
    if(second < 10) 
    {
        lcd_out(2, 15, "0");
        lcd_out(2, 16, ltrim(txt));
    }
    else
        lcd_out(2, 15, ltrim(txt));

    //separador
    lcd_out(2, 14, ":");

    bytetostr(minute, txt);
    if(minute < 10)
    {
        lcd_out(2, 12, "0");
        lcd_out(2, 13, ltrim(txt));
    }
    else
        lcd_out(2, 12, ltrim(txt));

    //separador
    lcd_out(2, 11, ":");
    
    bytetostr(hour, txt);
    if(hour < 10)
    {
        lcd_out(2, 9, "0");
        lcd_out(2, 10, ltrim(txt));
    }
    else
        lcd_out(2, 9, ltrim(txt));
    /*
    if(timer_milliseconds < 100){
        timer_milliseconds++;
    }else{
        timer_milliseconds = 0;
        increment_second();
        if(blink == 1)
            blink = 0;
        else
            blink = 1;
    }
    */
}















void main() {
     setup();
     while(1) loop();
}

void delay(unsigned int milliseconds){
     while(milliseconds > 0){
         delay_ms(1);
         milliseconds--;
     }
}