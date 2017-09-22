#define HIGH 1
#define LOW 0

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
int hour, minute, second;
int timer_milliseconds;
char txt[4];
unsigned counter;

void delay(unsigned int milliseconds){
    while(milliseconds > 0){
        delay_ms(1);
        milliseconds--;
    }
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

void display_separator(){
    if(counter > 40)
        lcd_out_cp(":");
    else    
        lcd_out_cp(" ");
}
void display_hour(){
    bytetostr(hour, txt);
    if(hour < 10)
    {
        lcd_out(2, 9, "0");
        lcd_out(2, 10, ltrim(txt));
    }
    else
        lcd_out(2, 9, ltrim(txt));
}
void display_minute(){
    bytetostr(minute, txt);
    if(minute < 10)
    {
        lcd_out(2, 12, "0");
        lcd_out(2, 13, ltrim(txt));
    }
    else
        lcd_out(2, 12, ltrim(txt));
}
void display_second(){
    bytetostr(second, txt);
    if(second < 10) 
    {
        lcd_out(2, 15, "0");
        lcd_out(2, 16, ltrim(txt));
    }
    else
        lcd_out(2, 15, ltrim(txt));
}

void display_time(){
    display_hour();
    display_separator();
    display_minute();
    display_separator();
    display_second();
}
void adjust_clock(){
    //Horas
    if(RB0_bit == LOW){
        delay(100);
        if(RB0_bit == LOW){
            increment_hour();
        }
    }

    //Minutos
    if(RB1_bit == LOW){
        delay(100);
        if(RB1_bit == LOW){
            increment_minute();
        }
    }

    //Segundos
    if(RB2_bit == LOW){
        delay(1);
        if(RB2_bit == LOW){
            increment_second();
        }
    }
}

void reset_timer0(){
    //inicializa os valores de timer zero
    //20 Mhz (frequência PIC) / 4 (prescaler) = 5Mhz
    //5Mhz / 65536 = 76,29.... ~> 5Mhz/ (65536 - X) = [time] (ps: escolhido time = 100)
    //X = 15536 (DEC) => {HEX} 3CB0 => {HEX} bit_H[3C] bit_L[B0] => {DEC} bit_H[60] bit_L[176]
    TMR0H = 60;  //0x3C
    TMR0L = 176; //0xB0
}
void interrupt(){
    GIE_bit = 0;
    if (TMR0IF_bit){                 //overflow em 0-255
        TMR0IF_bit=0;                //
        reset_timer0();
        counter++;                   //para cada interrupção do TIMER1 incrementa a variável contagem
        if (counter == 100){         //counter == time ~> passou 1 segundo
            increment_second();
            counter = 0;
        }
    }
    GIE_bit = 1; 
}

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
    
    //Inicializa todas as portas desligadas
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
void loop(){
    adjust_clock();
    display_time();
}
void main() {
    setup();

    //configura interrupções
    INTCON = 0;     //desabilita interrupções
    T0CON = 136;    //desliga todos os controles do tmr0 (em 16bits)    
    reset_timer0(); //inicializa o timer0
    TMR0IE_bit = 1; //liga interrupção do tmr0 (começa contar)
    GIE_bit = 1;    //liga o sistema de interrupções

    while(1) loop();
}