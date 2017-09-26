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

//icon
const char icon_0[] = {31,31,14,4,4,10,17,31};
const char icon_1[] = {31,17,14,4,4,10,17,31};
const char icon_2[] = {31,17,10,4,4,14,17,31};
const char icon_3[] = {31,17,10,4,4,14,31,31};
//vars
char txt[7];
const unsigned short days_of_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char* months[12] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};
const char* days_of_week[7] = {"Domingo", "Segunda", "Terca  ", "Quarta ", "Quinta ", "Sexta  ", "Sabado "};
unsigned char hour, minute, second;
unsigned char day_month, day_week, month;
unsigned short last_month_day;
unsigned year;
unsigned interrupt_counter;

/* Prototypes */
void loop();
void setup();

void set_clock();
void display_time();
void increment_time();

void set_date();
void display_date();

void display_icon(char pos_row, char pos_char);
void display_temperature();  


void timer0_init();
char format_number(unsigned char number);
char format_day(unsigned char dayweek);
unsigned short get_last_day();
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

    day_week = 1;
    day_month = 25;
    month = 8;
    year = 2017;

    interrupt_counter = 0;

    //Configuracao de interrupcoes
    T0CON = 136;    //desliga todos os controles do tmr0 (em 16bits)
    INTCON = 0;     //desabilita interrupções
    timer0_init(); //inicializa o timer0
    TMR0IE_bit = 1; //liga interrupção do tmr0 (começa contar)
    GIE_bit = 1;    //liga o sistema de interrupções
}

void loop(){
    set_date();
    set_clock();

    display_date();  
    display_temperature();  
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
            increment_time();
            interrupt_counter = 0;
        }
    }
    GIE_bit = 1; 
}

/* Funcoes :: Relogio */
void display_time(){
    //horas
    lcd_out(2, 9, format_number(hour));
    //separador
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //minutos
    lcd_out_cp(format_number(minute));
    //separador
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //segundos
    lcd_out_cp(format_number(second));
}

void set_clock(){
    //Horas
    if(RB1_bit == LOW){
        delay(100);
        if(RB1_bit == LOW){
            if(hour < 23)
                hour++;
            else            
                hour = 0;
        }
    }

    //Minutos
    if(RB2_bit == LOW){
        delay(100);
        if(RB2_bit == LOW){
            if(minute < 59)
                minute++;
            else
                minute = 0;
        }
    }
}

void increment_time(){
    //seconds
    if(second < 59)
        second++;
    else{
        second = 0;
        //minutes        
        if(minute < 59)
            minute++;
        else{
            minute = 0;
            //hours
            if(hour < 23)
                hour++;
            else{
                hour = 0;
                //days
                if(day_month < get_last_day()){
                    day_month++;
                    if(day_week < 6)
                        day_week++;
                    else
                        day_week = 0;
                }else{
                    day_month = 1;
                    //months                    
                    if(month < 12) 
                        month++; 
                    else{ 
                        month = 1;
                        //years
                        if(year < 2100) 
                            year++; 
                        else 
                            year = 2017;
                    }
                }
            }            
                
        }            
    }
}

/* Funcoes :: Data */
void set_date(){    
    if(RB3_bit == LOW){         //Dia
        delay(100);
        if(RB3_bit == LOW){                        
            if(day_month < get_last_day()){
                day_month++;
                if(day_week < 6)
                    day_week++;
                else
                    day_week = 0;
            }
            else
                day_month = 1;                
        }
    }
    if(RA4_bit == HIGH){         //Mes
        delay(100);
        if(RA4_bit == HIGH){
            if(month < 11) 
                month++; 
            else 
                month = 0;
        }
    }
    if(RA5_bit == HIGH){         //Ano
        delay(100);
        if(RA5_bit == HIGH){
            if(year < 2100) 
                year++; 
            else 
                year = 2017;
        }
    }        
}

void display_date(){
    //dia da semana
    lcd_out(2, 1, format_day(day_week)); 
    //dia do mes
    lcd_out(1, 1, format_number(day_month));
    //separador
    lcd_chr_cp(47);    
    //mes
    lcd_out_cp(ltrim(months[month]));
    //separador
    lcd_chr_cp(47);
    //ano
    inttostr(year, txt);
    lcd_out_cp(ltrim(txt));

    //frescura
    display_icon(1, 13);
    //CustomChar(1, 13);
}

void display_icon(char pos_row, char pos_char){
    char i;
    Lcd_Cmd(64);
    if(second % 2 == 0){
        if(interrupt_counter < 50){
            for (i = 0; i<=7; i++) lcd_chr_cp(icon_0[i]);
        }
        else{
            for (i = 0; i<=7; i++) lcd_chr_cp(icon_1[i]);
        }
    }
    else{
        if(interrupt_counter < 50){
            for (i = 0; i<=7; i++) lcd_chr_cp(icon_2[i]);
        }
        else{
            for (i = 0; i<=7; i++) lcd_chr_cp(icon_3[i]);
        }        
    }    
    
    Lcd_Cmd(_LCD_RETURN_HOME);
    Lcd_Chr(pos_row, pos_char, 0);
}

void display_temperature(){

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

char format_number(unsigned char number){
    bytetostr(number, txt);
    if(number < 10){
        txt[1] = '0';        
    }
    return ltrim(txt);
}

char format_day(unsigned char dayweek){
    static char day_txt[8];
    const char *day_name = days_of_week[dayweek];
    char i;
    for(i=0; day_txt[i] = day_name[i]; i++);

    return day_txt;
}

unsigned short get_last_day(){
    last_month_day = days_of_month[month];
    if(month == 2 && is_leap_year())
        last_month_day  +=1;
    return last_month_day;
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
