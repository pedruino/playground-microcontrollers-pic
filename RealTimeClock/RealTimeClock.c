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

/* Types */
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

/* Variables */
Status ALARM_STATUS = OFF;
ModeClock DISPLAY_MODE = CLOCK_DISPLAY;
Time clock;
Time alarm;
Calendar date;
char txt[7];
const unsigned short days_of_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char* months[12] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"};
const char* days_of_week[7] = {"Domingo", "Segunda", "Terca  ", "Quarta ", "Quinta ", "Sexta  ", "Sabado "};
char can_triggers_alarm;
unsigned int last_month_day;
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
int get_day_of_week(Calendar date);
void delay(unsigned milliseconds);
int vandermonde(int read_value);

/* Program */
void setup(){
    //Required settings
    INTCON2 = 0;  //Turn on pull up resistor
    ADCON1 = 15;  //Digital ports
    CMCON = 7;    //Turn off comparers

    //Setup base on PIC18F4520 (UniFtec)
    TRISA = 48;   //RA4 & RA5 as INPUT
    TRISB = 15;   //RB0 ..RB3
    TRISC = 0;    //output
    TRISD = 0;    //output
    TRISE = 0;    //output

    //Init alls ports off
    PORTA = LOW; //safe
    PORTB = LOW; //safe
    PORTC = LOW; //safe
    PORTD = LOW; //safe
    PORTE = LOW; //safe

    //Init LCD
    lcd_init();
    lcd_cmd(_LCD_CLEAR);
    lcd_cmd(_LCD_CURSOR_OFF);

    //Init ADC
    adc_init();

    //Init others variables
    clock.hour = 12;
    clock.minute = 00;
    clock.second = 00;

    alarm.hour = 06;
    alarm.minute = 00;
    alarm.second = 00;

    date.day_month = 30;
    date.month = 8;
    date.year = 2017;
    date.day_week = get_day_of_week(date);

    can_triggers_alarm = 0;
    interrupt_counter = 0;

    //Setup interrupt
    T0CON = 136;    //turn off all TMR0 controls (16bits)
    INTCON = 0;     //disable interruptions
    timer0_init();  //initialize TMR0
    TMR0IE_bit = 1; //enable TMR0 (start counter)
    GIE_bit = 1;    //enable global interruptions
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
            RC0_bit = LOW;//turn off led0
            RC1_bit = LOW;//turn off led1
            RC2_bit = LOW;//turn off led2
            RC3_bit = LOW;//turn off led3
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

        interrupt_counter++;            //for each TMR0 interrupt
        if (interrupt_counter == 100){  //counter == {time} ~> 1sec elapsed
            increment_time();
            interrupt_counter = 0;
        }
    }
    GIE_bit = 1;
}

/* Functions :: Clock */
void display_time(){
    //hours
    lcd_out(2, 9, format_number(clock.hour));
    //separator
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //minutes
    lcd_out_cp(format_number(clock.minute));
    //separator
    if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
    //seconds
    lcd_out_cp(format_number(clock.second));
}

void set_clock(){
    //Hour
    if(RB1_bit == LOW){
        delay(100);
        if(RB1_bit == LOW){
            if(clock.hour < 23)
                clock.hour++;
            else
                clock.hour = 0;
        }
    }

    //Minute
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

/* Functions :: Date */
void set_date(){
    //Day
    if(RB3_bit == LOW){
        delay(100);
        if(RB3_bit == LOW){
            if(date.day_month < get_last_day())
                date.day_month++;
            else
                date.day_month = 1;

            date.day_week = get_day_of_week(date);
        }
    }
    //Month
    if(RA4_bit == HIGH){
        delay(100);
        if(RA4_bit == HIGH){
            if(date.month < 11){
                date.month++;
                while (date.day_month > get_last_day())
                  date.day_month--;
            }else
              date.month = 0;
            date.day_week = get_day_of_week(date);
        }
    }
    //Year
    if(RA5_bit == HIGH){
        delay(100);
        if(RA5_bit == HIGH){
            if(date.year < 2100){
              date.year++;
              if(!is_leap_year() && date.day_month > get_last_day())
                  date.day_month--;
            }else
                date.year = 2017;
            date.day_week = get_day_of_week(date);
        }
    }
}

void display_date(){
    //Day of Week
    lcd_out(2, 1, format_day(date.day_week));
    //Day of Month
    lcd_out(1, 1, format_number(date.day_month));
    //separator
    lcd_chr_cp(47);
    //Month
    lcd_out_cp(ltrim(months[date.month]));
    //separator
    lcd_chr_cp(47);
    //Year
    inttostr(date.year, txt);
    lcd_out_cp(ltrim(txt));
}

/* Functions :: Alarm */
void set_alarm(){
  //Hour
  if(RB1_bit == LOW){
      delay(100);
      if(RB1_bit == LOW){
          if(alarm.hour < 23)
              alarm.hour++;
          else
              alarm.hour = 0;
      }
  }
  //Minute
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
  //hours
  lcd_out(2, 9, format_number(alarm.hour));
  //separator
  if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
  //minutes
  lcd_out_cp(format_number(alarm.minute));
  //separator
  if(interrupt_counter > 40)  lcd_chr_cp(':');    else    lcd_chr_cp(' ');
  //seconds
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
    RC0_bit = HIGH;//turn ON led0
    RC1_bit = HIGH;//turn ON led1
    RC2_bit = HIGH;//turn ON led2
    RC3_bit = HIGH;//turn ON led3
  }else{
    RC0_bit = LOW;//turn OFF led0
    RC1_bit = LOW;//turn OFF led1
    RC2_bit = LOW;//turn OFF led2
    RC3_bit = LOW;//turn OFF led3
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

/* Functions :: Temperature*/
void read_temperature(){
  char i, temperature;
  const char temp_icon[] = {28,20,28,0,7,8,8,7};

  if(interrupt_counter == 0){
    int raw_value = adc_read(1);
    int converted_value = vandermonde(raw_value);
    temperature = converted_value % 100;
  }

  lcd_out(1, 13, format_number(temperature));
  //icon
  lcd_cmd(64);
  for (i = 0; i<=7; i++) lcd_chr_cp(temp_icon[i]);
  lcd_cmd(_LCD_RETURN_HOME);
  lcd_chr(1, 15, 0);
}

/* Functions :: Helpers */
void timer0_init(){
    //Initialize TMR0 with values
    //20 Mhz (freq PIC) / 4 (prescaler) = 5Mhz
    //5Mhz / 65536 = 76,29.... ~> 5Mhz/ (65536 - X) = [time] (ps: choosed time = 100)
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

/* Sakamoto's method */
int get_day_of_week(Calendar date){//1 <= m <= 12,  y > 1752 (in the U.K.)
  char m = date.month+1;
  char d = date.day_month;
  int y = date.year;

  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  y -= m < 3;
  return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
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
