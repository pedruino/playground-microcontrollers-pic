#define HIGH 1
#define LOW 0
#define AUTO_MODE 0
#define SET_MODE 1

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
char txt[4], year_txt[7];
const unsigned short days_of_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char* days_of_week[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Sex", "Sat"};
unsigned char hour, minute, second;
unsigned char day_month, day_week, month;
unsigned short last_month_day;
unsigned year;
unsigned interrupt_counter;
unsigned char MODE = AUTO_MODE;

/* Prototypes */
void loop();
void setup();

void set_clock();
void display_time();
void increment_hour();
void increment_minute();
void increment_second();

void set_date();
void display_date();
void increment_day_of_month();
void increment_day_of_week();
void increment_month();
void increment_year();


void timer0_init();
char format_time(unsigned char time);
int is_leap_year();
void delay(unsigned milliseconds);

void setup(){
    //Configuracao obrigatoria
    INTCON2 = 0;  //Liga o resistor pull up
    ADCON1 = 15;  //Portas digitais
    CMCON = 7;    //Desliga comparadores

    //Setup do PIC (baseado na configuracao da placa)
    TRISA = 48;   //RA4 e RA5 com entrada
    TRISB = 15;   //RB0 ..RB3
    TRISC = 0;    //saida
    TRISD = 0;    //saida
    TRISE = 0;    //saida
    
    //Inicializa todas as portas desligadas
    PORTA = 0; //
    PORTB = 0; //
    PORTC = 0; //
    PORTD = 0; //
    PORTE = 0; //

    //Incializacao do LCD
    lcd_init();
    lcd_cmd(_LCD_CLEAR);
    lcd_cmd(_LCD_CURSOR_OFF);

    //Variaveis
    hour = 0;
    minute = 0;
    second = 0;

    day_week = 0;
    day_month = 24;
    month = 9;
    year = 2017;

    interrupt_counter = 0;


    //Configuracao de interrupcoes
    T0CON = 136;    //desliga todos os controles do tmr0 (em 16bits)
    INTCON = 0;     //desabilita interrupções
    timer0_init(); //inicializa o timer0
    TMR0IE_bit = 1; //liga interrupção do tmr0 (começa contar)
    GIE_bit = 1;    //liga o sistema de interrupções
}
int i = 0;
void loop(){
    set_date();
    set_clock();

    display_date();    
    display_time();
}

void main() {
    setup();
    while(1) loop();
}

void interrupt(){
    GIE_bit = 0;
    if (TMR0IF_bit){
        TMR0IF_bit=0;                
        
        timer0_init();

        interrupt_counter++;                   //para cada interrupcao do TIMER1 incrementa a variavel contagem
        if (interrupt_counter == 100){         //counter == time ~> passou 1 segundo
            increment_second();
            interrupt_counter = 0;
        }
    }
    GIE_bit = 1; 
}

/* Funcoes :: Relogio */
void display_time(){
    //horas
    lcd_out(2, 9, format_time(hour));
    //separador
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //minutos
    lcd_out_cp(format_time(minute));
    //separador
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //segundos
    lcd_out_cp(format_time(second));
}

void set_clock(){
    //Horas
    if(RB2_bit == LOW){
        delay(100);
        if(RB2_bit == LOW){
            MODE = SET_MODE;
            increment_hour();
        }
    }

    //Minutos
    if(RB1_bit == LOW){
        delay(100);
        if(RB1_bit == LOW){
            MODE = SET_MODE;
            increment_minute();
        }
    }

    //Segundos
    if(RB0_bit == LOW){
        delay(100);
        if(RB0_bit == LOW){
            MODE = SET_MODE;
            increment_second();
        }
    }
}

void increment_hour(){
    if(hour < 23)
        hour++;
    else
    {
        hour = 0;
        if(MODE != SET_MODE)
            increment_day_of_month();
    }
}

void increment_minute(){
    if(minute < 59)
        minute++;
    else{
        minute = 0;
        if(MODE != SET_MODE)
            increment_hour();
    }
}

void increment_second(){
    if(second < 59)
        second++;
    else{
        second = 0;
        if(MODE != SET_MODE)
            increment_minute();
    }
}

/* Funcoes :: Data */
void set_date(){    
    if(RA5_bit == HIGH){         //Dia
        delay(100);
        if(RA5_bit == HIGH){
            MODE = SET_MODE;
            increment_day_of_month();
        }
    }
    if(RA4_bit == HIGH){         //Mes
        delay(100);
        if(RA4_bit == HIGH){
            MODE = SET_MODE;
            increment_month();
        }
    }
    if(RB3_bit == LOW){          //Ano
        delay(100);
        if(RB3_bit == LOW){
            MODE = SET_MODE;
            increment_year();
        }
    }    
}

void display_date(){
    //dia
    lcd_out(1, 1, days_of_week[day_week]);    
    lcd_out_cp(" - ");
    lcd_out(1, 7, format_time(day_month));    
    //separador
    lcd_chr_cp(47);    
    //mes
    lcd_out_cp(format_time(month));
    //separador
    lcd_chr_cp(47);
    //ano
    inttostr(year, year_txt);
    lcd_out_cp(ltrim(year_txt));
}

void increment_day_of_month(){    
    last_month_day = days_of_month[month-1];
    if(month == 2 && is_leap_year())
        last_month_day  +=1;
    
    if(day_month < last_month_day)
    {
        day_month++;
        increment_day_of_week();
    }
    else{
        day_month = 1;
        if(MODE != SET_MODE)
            increment_month();
    }
}

void increment_day_of_week(){
    if(day_week < 6)
        day_week++;
    else
        day_week = 0;
}

void increment_month(){
    if(month < 12)
       month++;
    else{
        month = 1;
        if(MODE != SET_MODE)
            increment_year();
    }
}

void increment_year(){
    if(year < 2100)
       year++;
    else
        year = 2017;
}

/* Funcoes auxiliares */
void timer0_init(){    
    //Inicializa os valores de timer zero
    //20 Mhz (frequência PIC) / 4 (prescaler) = 5Mhz
    //5Mhz / 65536 = 76,29.... ~> 5Mhz/ (65536 - X) = [time] (ps: escolhido time = 100)
    //X = 15536 (DEC) => {HEX} 3CB0 => {HEX} bit_H[3C] bit_L[B0] => {DEC} bit_H[60] bit_L[176]
    TMR0H = 60;  //0x3C
    TMR0L = 176; //0xB0
}

char format_time(unsigned char time){
    bytetostr(time, txt);
    if(time < 10){
        txt[1] = '0';        
    }
    return ltrim(txt);
}

int is_leap_year(){
    if(year % 4 == 0){
        if(year % 100 == 0)
            return (year % 400 == 0);
        
        return 1;
    }
    return 0;
}

void delay(unsigned int milliseconds){
    while(milliseconds > 0){
        delay_ms(1);
        milliseconds--;
    }
}