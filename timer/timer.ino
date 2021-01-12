#include <IRremote.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <Wire.h>

#define SWT_0     0x1FEE01F
#define SWT_1     0x1FE50AF
#define SWT_2     0x1FED827
#define SWT_3     0x1FEF807
#define SWT_4     0x1FE30CF
#define SWT_5     0x1FEB04F
#define SWT_6     0x1FE708F
#define SWT_7     0x1FE00FF
#define SWT_8     0x1FEF00F
#define SWT_9     0x1FE9867
#define SWT_MODE  0x1FE58A7
#define SWT_LEFT  0x1FE40BF
#define SWT_RIGHT 0x1FEC03F
#define SWT_RPT   0x1FE10EF
#define SWT_SAVE  0x1FE807F

typedef enum system_state
{
  NORMAL,
  SHOW_SAVED_TIMER,
  ON_TIME_SET,
  OFF_TIME_SET
} system_state;

system_state state = NORMAL;

#define EEPROM_DATA_ADD     0
#define EEPROM_STATUS_ADD  10

#define IR          8
#define RELAY       9

#define rs          12
#define en          11
#define d4          5
#define d5          4
#define d6          3
#define d7          2

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
IRrecv IrReceiver(IR);
decode_results results;
DS3231 Clock;

int time_arr[] = {0, 0, 0, 0};
long int IrData = 0;
int relay_status;

int set_time_1_hour = 0;
int set_time_1_minu = 0;
int set_time_2_hour = 0;
int set_time_2_minu = 0;

int on_time_ittr = 0;
int off_time_ittr = 0;

int tmp = 0;

int get_swt_val(long int value)
{
  //Serial.println(value);
  switch (value)
  {
    case SWT_0:
      return 0;
      break;
    case SWT_1:
      return 1;
      break;
    case SWT_2:
      return 2;
      break;
    case SWT_3:
      return 3;
      break;
    case SWT_4:
      return 4;
      break;
    case SWT_5:
      return 5;
      break;
    case SWT_6:
      return 6;
      break;
    case SWT_7:
      return 7;
      break;
    case SWT_8:
      return 8;
      break;
    case SWT_9:
      return 9;
      break;
    default:
      return -1;
  }
}

void print_lcd(String str, int row, int col)
{
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(col, row);
  lcd.print(str);
}

String padding(int num)
{
  if (num < 10)
    return "0" + String(num);
  else
    return String(num);
}

void set_EEPROM()
{
  for (int i = 0; i < 4; i++)
  {
    EEPROM.update(EEPROM_DATA_ADD + i, time_arr[i]);
  }
}

void setup()
{
  pinMode(RELAY, OUTPUT);

  IrReceiver.enableIRIn();  // Start the receiver
  IrReceiver.blink13(true); // Enable feedback LED
  lcd.begin(16, 2);
  Wire.begin();

  //  set_EEPROM(12, 22, 10, 43);

  relay_status = EEPROM.read(EEPROM_STATUS_ADD);

  for (int i = EEPROM_DATA_ADD; i <= EEPROM_DATA_ADD + 4; i++)
  {
    int tmp = EEPROM.read(i);
    if (i == EEPROM_DATA_ADD && i == (EEPROM_DATA_ADD + 2))
    {
      if (tmp >= 0 && tmp <= 23)
        time_arr[i] = tmp;
      else
        time_arr[i] = 0;
    }
    else
    {
      if (tmp >= 0 && tmp <= 59)
        time_arr[i] = tmp;
      else
        time_arr[i] = 0;
    }
  }

  Serial.begin(9600);
}

void loop()
{
  bool Century = false;
  bool h12;
  bool PM;
  int t_hour = Clock.getHour(h12, PM);
  int t_min = Clock.getMinute();
  int t_date = Clock.getDate();
  int t_month = Clock.getMonth(Century);
  int t_sec = Clock.getSecond();

  if (IrReceiver.decode(&results))
  {
    IrData = results.value;
    if (IrData == SWT_MODE)
    {
      if (state == OFF_TIME_SET)
        state = NORMAL;
      else
        state = system_state(state + 1);
    }


    IrReceiver.resume();
  }

  switch (state)
  {
    case NORMAL:
      {
        print_lcd("Camera Timer", 0, 2);
        String time_str = "";
        time_str = padding(t_date) + "/" +
                   padding(t_month) + " " +
                   padding(t_hour) + ":" +
                   padding(t_min) + ":" +
                   padding(t_sec);
        print_lcd(time_str, 1, 1);
        break;
      }
    case SHOW_SAVED_TIMER:
      {
        print_lcd("ON time:  " + padding(time_arr[0]) + ":" + padding(time_arr[1]), 0, 0);
        print_lcd("OFF time: " + padding(time_arr[2]) + ":" + padding(time_arr[3]), 1, 0);
        break;
      }
    case ON_TIME_SET:
      {
        print_lcd("Set ON time:", 0, 2);
        print_lcd(padding(set_time_1_hour) + "::" + padding(set_time_1_minu), 1, 5);

        tmp = get_swt_val(IrData);
        //        Serial.println(tmp);
        //        Serial.println(padding(set_time_1_hour) + "::" + padding(set_time_1_minu) + "   " + padding(on_time_ittr));
        if (tmp != -1)
        {
          if (on_time_ittr == 0)
          {
            set_time_1_hour = (tmp * 10) + (set_time_1_hour % 10);
          }
          else if (on_time_ittr == 1)
          {
            set_time_1_hour = (set_time_1_hour / 10) * 10 + tmp;
          }
          else if (on_time_ittr == 2)
          {
            set_time_1_minu = (tmp * 10) + (set_time_1_minu % 10);
          }
          else if (on_time_ittr == 3)
          {
            set_time_1_minu = (set_time_1_minu / 10) * 10 + tmp;
          }
          on_time_ittr++;
          delay(300);
          if (on_time_ittr == 4)
            on_time_ittr = 0;
          tmp = -1;
        }
        if (set_time_1_hour > 23)
        {
          set_time_1_hour = 0;
          on_time_ittr = 0;
        }
        if (set_time_1_minu > 59)
        {
          set_time_1_minu = 0;
          on_time_ittr = 2;
        }

        if (IrData == SWT_SAVE)
        {
          time_arr[0] = set_time_1_hour;
          time_arr[1] = set_time_1_minu;
          set_EEPROM();
          set_time_1_hour = 0;
          set_time_1_minu = 0;
        }
        break;
      }
    case OFF_TIME_SET:
      {
        print_lcd("Set OFF time:", 0, 2);
        print_lcd(padding(set_time_2_hour) + "::" + padding(set_time_2_minu), 1, 5);
        tmp = get_swt_val(IrData);
        //        Serial.println(tmp);
        //        Serial.println(padding(set_time_1_hour) + "::" + padding(set_time_1_minu) + "   " + padding(on_time_ittr));
        if (tmp != -1)
        {
          if (off_time_ittr == 0)
          {
            set_time_2_hour = (tmp * 10) + (set_time_2_hour % 10);
          }
          else if (off_time_ittr == 1)
          {
            set_time_2_hour = (set_time_2_hour / 10) * 10 + tmp;
          }
          else if (off_time_ittr == 2)
          {
            set_time_2_minu = (tmp * 10) + (set_time_2_minu % 10);
          }
          else if (off_time_ittr == 3)
          {
            set_time_2_minu = (set_time_2_minu / 10) * 10 + tmp;
          }
          off_time_ittr++;
          delay(300);
          if (off_time_ittr == 4)
            off_time_ittr = 0;
          tmp = -1;
        }
        if (set_time_2_hour > 23)
        {
          set_time_2_hour = 0;
          off_time_ittr = 0;
        }
        if (set_time_2_minu > 59)
        {
          set_time_2_minu = 0;
          off_time_ittr = 2;
        }
        if (IrData == SWT_SAVE)
        {
          time_arr[2] = set_time_2_hour;
          time_arr[3] = set_time_2_minu;
          set_EEPROM();
          set_time_2_hour = 0;
          set_time_2_minu = 0;
        }
        break;
      }
    default:
      // statements
      break;
  }
  IrData = 0;

  if(t_hour == time_arr[0] && t_min == time_arr[1])
  {
    relay_status = 1;
    EEPROM.update(EEPROM_STATUS_ADD, relay_status);
  }
  if(t_hour == time_arr[2] && t_min == time_arr[3])
  {
    relay_status = 0;
    EEPROM.update(EEPROM_STATUS_ADD, relay_status);
  }

  digitalWrite(RELAY, relay_status);
}
