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
}
void loop(){
    lcd_out(1, 1, "Programa do Pedro");
    delay(1000);
    lcd_out(2, 1, "¬¬'");
    delay(1000);
    lcd_cmd(_LCD_CLEAR);
    delay(1000);
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
