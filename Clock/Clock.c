#define HIGH 1
#define LOW 0

char unidade, dezena, centena, milhar;
void delay(unsigned int milliseconds);
void multiplex();
void adjust_time(int hour);
char decode(char number);
void execute(char times);

void setup(){
    TRISA = 48; //RA4 e RA5 com entrada
    TRISB = 15; //RB0 ..RB3
    TRISC = 0; //saída
    TRISD = 0; //saída
    TRISE = 0; //saída
    INTCON2 = 0; //liga o resistor pull up
    ADCON1 = 15; //Portas digitais
    CMCON = 7; //desliga comparador
    PORTC = 0; //pa
}
void main() {
    setup();

    unidade = 0;
    dezena = 0;
    centena = 0;
    milhar = 0;

    while(1){
       multiplex();
       if(RA4_bit == HIGH){
           while(RA4_bit == HIGH){
               execute(50);//multiplex(); //
               adjust_time(0);
           }
       }
       if(RA5_bit == HIGH){
           while(RA5_bit == HIGH){
               execute(50);//multiplex(); //execute(50);
               adjust_time(1);
           }
       }
    }
}

void multiplex(){
    //RC4, RC5, RE2, RA3
    PORTD = decode(unidade);
    PORTA = 8; // ra3 = a => 2^3
    PORTE = 0;
    PORTC = 0;
    delay(1);

    PORTD = decode(dezena);
    PORTA = 0; //zera unidade
    PORTE = 4; // re2 = e =>  2^2
    delay(1);

    PORTD = decode(centena);
    PORTE = 0; //zera dezena
    PORTC = 16; // rc4 = c => 2^4
    delay(1);

    PORTD = decode(milhar);
    PORTC = 32; // rc5 = c => 2^5
    delay(1);
}

void adjust_time(int hour){
   if(hour != 1){
     unidade++;
     if(unidade == 10){
         unidade = 0;
         dezena++;
     }
     if(dezena == 6){
         unidade = 0;
         dezena = 0;
         centena++;
     }
   }else{
     centena++;
     if(centena == 10){
         centena = 0;
         milhar++;
     }else if(milhar == 2 && centena == 4){
         unidade = 0;
         dezena = 0;
         centena = 0;
         milhar = 0;
     }
   }
}

char decode(char number){
  switch(number){
      case 0: return 63;
      case 1: return 6;
      case 2: return 91;
      case 3: return 79;
      case 4: return 102;
      case 5: return 109;
      case 6: return 125;
      case 7: return 7;
      case 8: return 127;
      case 9: return 111;
  }
}

void execute(char times){
    char i;
    for(i = 0; i < times; i++)
        multiplex();
}

void delay(unsigned int milliseconds){
     while(milliseconds > 0){
         delay_ms(1);
         milliseconds--;
     }
}