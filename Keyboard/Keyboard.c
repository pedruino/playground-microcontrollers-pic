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
    TRISA = 48;//ra4 e ra5 como entrada
    TRISB = 15;//rb0, rb1, rb2 e rb3 como entrada
    TRISC = 0;//portc como saída (leds)
    INTCON2 = 0;//liga resistor pull up do portb
    ADCON1 = 15;//portas digitais
    CMCON = 7;//desliga os comparadores
    PORTC = 0;//desliga todas as saídas
    while(1){
        if(RA4_bit == HIGH){//verifica se ra4 foi acionado
            delay(time);//delay para evitar ruídos
            if(RA4_bit == HIGH){//verifica novamente para evitar  acionamento indesejado
                RC0_bit = HIGH;//liga led0
            }
        }else if(RA5_bit == HIGH){//verifica se ra4 foi acionado
            while(RA5_bit == HIGH)delay(time);
            
            RC3_bit = HIGH;//liga led 1
            RC1_bit = HIGH;//liga led 2
        }
        if(RB0_bit == LOW){//verifica se ra4 foi acionado
            delay(time);//delay para evitar ruídos
            if(RB0_bit == LOW){//verifica novamente para evitar  acionamento indesejado
                rc2_bit = HIGH;//liga led 3
            }
        }else if(rb1_bit == LOW){//verifica se ra4 foi acionado
            while(RB1_bit == LOW)delay(time);
            rc0_bit = LOW;//desliga led 0
        }
        if(rb2_bit == LOW){//verifica se ra4 foi acionado
            while(rb2_bit == LOW)delay(time);
                rc3_bit = LOW;//desliga led 1
                RC1_bit = LOW;//desliga led 2
                RC2_bit = LOW;//desliga led 3
        }
        if(RB3_bit == LOW){//verifica se ra4 foi acionado
            while(RB3_bit == 0)delay(time);
            
            RC0_bit = HIGH;//liga led 0
            RC3_bit = HIGH;//liga led 1
            RC1_bit = HIGH;//liga led 2
            RC2_bit = HIGH;//liga led 3
        }
    }//fim do while(1)
}//fim do main