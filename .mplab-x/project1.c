void delay(unsigned int milliseconds){
  while(milliseconds > 0){
    delay_ms(1);
    milliseconds--;
  }
}

void reset(){
  rc5_bit = 1;
  rc4_bit = 0;
  re2_bit = 0;
  ra3_bit = 0;
}



void main() {
  trisa = 0;
  trisd = 0;
  trisc = 0;
  trise = 0;
  adcon1 = 15;
  cmcon = 7;   
  unsigned int time = 100; //100 milliseconds

  while(1){
    reset();
    
    portd = 1;
    delay(time);

    rc5_bit = 0;
    rc4_bit = 1;
    portd = 1;
    delay(time);

    rc4_bit = 0;
    re2_bit = 1;
    portd=1;
    delay(time);

    re2_bit = 0;
    ra3_bit = 1;
    portd = 1;
    delay(time);

    portd = 2;
    delay(time);
    portd = 4;
    delay(time);
    portd = 8;
    delay(time);

    ra3_bit = 0;
    re2_bit = 1;
    portd = 8;
    delay(time);

    re2_bit = 0;
    rc4_bit = 1;
    portd = 8;
    delay(time);

    rc4_bit = 0;
    rc5_bit = 1;
    portd = 8;
    delay(time);
    portd = 16;
    delay(time);
    portd = 32;
    delay(time);
}
}