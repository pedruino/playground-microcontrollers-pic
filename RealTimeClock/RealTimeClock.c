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

//Types
typedef enum {
  CLOCK_DISPLAY = 0,
  ADJUSTING_ALARM = 1
} ModeClock;

typedef enum {
  OFF = 0,
  ON = 1
} Status;

typedef struct{
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
} Time;

typedef struct{
  unsigned char day_week;
  unsigned char day_month;
  unsigned char month;
  unsigned int year;
} Calendar;

Status ALARM_STATUS = OFF;
ModeClock DISPLAY_MODE = CLOCK_DISPLAY;
Time clock;
Time alarm;
Calendar date;
//vars
char txt[7];
const unsigned short days_of_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char* months[12] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};
const char* days_of_week[7] = {"Domingo", "Segunda", "Terca  ", "Quarta ", "Quinta ", "Sexta  ", "Sabado "};
char can_triggers_alarm, resto, inteiro;
unsigned int last_month_day;
//unsigned date.year;
unsigned interrupt_counter;
/* Prototypes */
void loop();
void setup();

void set_clock();
void display_time();
void increment_time();

void set_date();
void display_date();

void set_alarm();
void display_alarm();
void check_alarm();
void triggers_alarm();
void display_alarm_icon();

void read_temperature();

void timer0_init();
char format_number(unsigned char number);
char format_day(unsigned char dayweek);
unsigned short get_last_day();
int is_leap_year();
void delay(unsigned milliseconds);
int vandermonde(int read_value);

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

    //Inicializacao do ADC
    adc_init();

    //Variaveis
    clock.hour = 12;
    clock.minute = 00;
    clock.second = 00;

    alarm.hour = 12;
    alarm.minute = 01;
    alarm.second = 00;

    date.day_week = 4;
    date.day_month = 28;
    date.month = 8;
    date.year = 2017;

    can_triggers_alarm = 0;
    interrupt_counter = 0;

    //Configuracao de interrupcoes
    T0CON = 136;    //desliga todos os controles do tmr0 (em 16bits)
    INTCON = 0;     //desabilita interrupções
    timer0_init();  //inicializa o timer0
    TMR0IE_bit = 1; //liga interrupção do tmr0 (começa contar)
    GIE_bit = 1;    //liga o sistema de interrupções
}

void loop(){
  if(RB0_bit == LOW){
      char time_commit = clock.second + 1;

      switch (DISPLAY_MODE) {
        case ADJUSTING_ALARM:
          while (RB0_bit == LOW){
            if(clock.second == time_commit){
              DISPLAY_MODE = CLOCK_DISPLAY;
            }
          }
          if(clock.second < time_commit)
          {
            ALARM_STATUS = !ALARM_STATUS;
            can_triggers_alarm = 0;
            RC0_bit = LOW;//desliga led0
            RC1_bit = LOW;//desliga led1
            RC2_bit = LOW;//desliga led2
            RC3_bit = LOW;//desliga led3
          }
        break;

        default:
            while(RB0_bit == LOW) delay(1);
            DISPLAY_MODE = ADJUSTING_ALARM;
            lcd_cmd(_LCD_CLEAR);
      }
  }

  if(DISPLAY_MODE == CLOCK_DISPLAY){
    set_date();
    set_clock();
    read_temperature();
    display_alarm_icon();
    display_date();
    display_time();
  }else{
    set_alarm();
    display_alarm();
  }

  check_alarm();
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
    lcd_out(2, 9, format_number(clock.hour));
    //separador
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //minutos
    lcd_out_cp(format_number(clock.minute));
    //separador
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //segundos
    lcd_out_cp(format_number(clock.second));
}

void set_clock(){
    //Horas
    if(RB1_bit == LOW){
        delay(100);
        if(RB1_bit == LOW){
            if(clock.hour < 23)
                clock.hour++;
            else
                clock.hour = 0;
        }
    }

    //Minutos
    if(RB2_bit == LOW){
        delay(100);
        if(RB2_bit == LOW){
            if(clock.minute < 59)
                clock.minute++;
            else
                clock.minute = 0;
        }
    }
}

void increment_time(){
    //seconds
    if(clock.second < 59)
        clock.second++;
    else{
        clock.second = 0;
        //minutes
        if(clock.minute < 59)
            clock.minute++;
        else{
            clock.minute = 0;
            //hours
            if(clock.hour < 23)
                clock.hour++;
            else{
                clock.hour = 0;
                //days
                if(date.day_month < get_last_day()){
                    date.day_month++;
                    if(date.day_week < 6)
                        date.day_week++;
                    else
                        date.day_week = 0;
                }else{
                    date.day_month = 1;
                    //months
                    if(date.month < 12)
                        date.month++;
                    else{
                        date.month = 1;
                        //years
                        if(date.year < 2100)
                            date.year++;
                        else
                            date.year = 2017;
                    }
                }
            }

        }
    }
}

/* Funcoes :: Data */
void set_date(){
    //Day
    if(RB3_bit == LOW){
        delay(100);
        if(RB3_bit == LOW){
            if(date.day_month < get_last_day()){
                date.day_month++;
                if(date.day_week < 6)
                    date.day_week++;
                else
                    date.day_week = 0;
            }
            else
                date.day_month = 1;
        }
    }
    //Month
    if(RA4_bit == HIGH){
        delay(100);
        if(RA4_bit == HIGH){
            if(date.month < 11){
                date.month++;
            }else
                date.month = 0;
        }
    }
    //Year
    if(RA5_bit == HIGH){
        delay(100);
        if(RA5_bit == HIGH){
            if(date.year < 2100){
              date.year++;
              if(is_leap_year())
                date.day_week = (date.day_week + 2) % 7;
              else
                date.day_week = (date.day_week + 1) % 7;
            }else
                date.year = 2017;
        }
    }
}

void display_date(){
    //dia da semana
    lcd_out(2, 1, format_day(date.day_week));
    //dia do mes
    lcd_out(1, 1, format_number(date.day_month));
    //separador
    lcd_chr_cp(47);
    //mes
    lcd_out_cp(ltrim(months[date.month]));
    //separador
    lcd_chr_cp(47);
    //ano
    inttostr(date.year, txt);
    lcd_out_cp(ltrim(txt));
}

/* Funcoes :: Alarm */
void set_alarm(){
  //Horas
  if(RB1_bit == LOW){
      delay(100);
      if(RB1_bit == LOW){
          if(alarm.hour < 23)
              alarm.hour++;
          else
              alarm.hour = 0;
      }
  }

  //Minutos
  if(RB2_bit == LOW){
      delay(100);
      if(RB2_bit == LOW){
          if(alarm.minute < 59)
              alarm.minute++;
          else
              alarm.minute = 0;
      }
  }
}

void display_alarm(){
  lcd_out(1, 1, "Alarme");
  //icon
  display_alarm_icon();
  //horas
  lcd_out(2, 9, format_number(alarm.hour));
  //separador
  if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
  //minutos
  lcd_out_cp(format_number(alarm.minute));
  //separador
  if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
  //segundos
  lcd_out_cp(format_number(alarm.second));
}

void check_alarm(){
  if(!can_triggers_alarm)
     can_triggers_alarm = (clock.hour == alarm.hour && clock.minute == alarm.minute);

  if(can_triggers_alarm && ALARM_STATUS == ON)
    triggers_alarm();
}

void triggers_alarm(){
  if(clock.second % 2 == 0){
    RC0_bit = HIGH;//liga led0
    RC1_bit = HIGH;//liga led1
    RC2_bit = HIGH;//liga led2
    RC3_bit = HIGH;//liga led3
  }else{
    RC0_bit = LOW;//desliga led0
    RC1_bit = LOW;//desliga led1
    RC2_bit = LOW;//desliga led2
    RC3_bit = LOW;//desliga led3
  }
}

void display_alarm_icon(){
  if(ALARM_STATUS == ON){
    const char alarm_icon[] = {0,27,14,17,21,17,14,0};
    char i;
    lcd_cmd(72);
    for (i = 0; i<=7; i++) lcd_chr_cp(alarm_icon[i]);
    lcd_cmd(_LCD_RETURN_HOME);
    lcd_chr(1, 16, 1);
  }else
    lcd_chr(1, 16, ' ');
}

/* Funcoes :: Temperatura*/
void read_temperature(){
  char i;
  const char temp_icon[] = {28,20,28,0,7,8,8,7};
  int raw_value = adc_read(1);          //le a entrada adc_0
  int temperature = vandermonde(raw_value);

  lcd_out(1, 13, format_number(temperature % 100));
  //icon
  lcd_cmd(64);
  for (i = 0; i<=7; i++) lcd_chr_cp(temp_icon[i]);
  lcd_cmd(_LCD_RETURN_HOME);
  lcd_chr(1, 15, 0);
}

/* Funcoes :: Frescura */
void display_time_icon(){
    const char icon[4][8] =
    {
      {31,31,14,4,4,10,17,31},
      {31,17,14,4,4,10,17,31},
      {31,17,10,4,4,14,17,31},
      {31,17,10,4,4,14,31,31}
    };

    char i, pos_row, pos_char;
    pos_row = 1;
    pos_char = 13;

    Lcd_Cmd(64);
    if(clock.second % 2 == 0){
        if(interrupt_counter < 50){
            for (i = 0; i<=7; i++) lcd_chr_cp(icon[0][i]);
        }
        else{
            for (i = 0; i<=7; i++) lcd_chr_cp(icon[1][i]);
        }
    }
    else{
        if(interrupt_counter < 50){
            for (i = 0; i<=7; i++) lcd_chr_cp(icon[2][i]);
        }
        else{
            for (i = 0; i<=7; i++) lcd_chr_cp(icon[3][i]);
        }
    }

    lcd_cmd(_LCD_RETURN_HOME);
    lcd_chr(pos_row, pos_char, 0);
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
    last_month_day = days_of_month[date.month];
    if(date.month == 1 && is_leap_year())
        last_month_day  +=1;
    return last_month_day;
}

int is_leap_year(){
    if(date.year % 4 == 0){
        if(date.year % 100 == 0)
            return (date.year % 400 == 0);

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

int vandermonde(int read_value){
  int yield_value, converted_value;
  yield_value = read_value / 42;
  converted_value = read_value - yield_value;
  converted_value = converted_value / 2;
  return converted_value;
}
