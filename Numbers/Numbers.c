#define HIGH 1
#define LOW 0

void delay(unsigned int milliseconds){
     while(milliseconds > 0){
         delay_ms(1);
         milliseconds--;
     }
}
unsigned int time = 100;

void main() {
     TRISA = 48;  //ra4 e ra5 como entrada
     TRISB = 15;  //rb0, rb1, rb2 e rb3 como entrada
     TRISD = 0;   //set as output
     INTCON2 = 0; //liga resistor pull up do portb
     ADCON1 = 15; //portas digitais
     CMCON = 7;   //desliga os comparadores
     PORTD = 0;   //inicia o portd zerado

     RA3_bit = HIGH;
     while(1){
        if(RA5_bit == HIGH)
        {
            while(RA5_bit == HIGH)delay(time);
            portd = 109;
        }
        if(RA4_bit == HIGH)
        {
            while(RA4_bit == HIGH)delay(time);
            portd = 102;
        }
        if(RB3_bit == LOW){
            while(RB3_bit == LOW)delay(time);
            portd = 79;
        }
        if(RB2_bit == LOW)
        {
            while(RB2_bit == LOW)delay(time);
            portd = 91;
        }
        if(RB1_bit == LOW)
        {
            while(RB1_bit == LOW)delay(time);
            portd = 6;
        }
        if(RB0_bit == LOW)
        {
            while(RB0_bit == LOW)delay(time);
            portd = 63;
        }
     }
}