//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS1302.h>

ThreeWire myWire(4, 5, 2);  // DATA, CLOCK, RESET
RtcDS1302<ThreeWire> Rtc(myWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const char* Months[] = {
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};
const char* Days[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

unsigned long currentMillis;
unsigned long startMillis;
unsigned long period = 1000;  // in milliseconds
bool showDay = false;
short runningPeriod = 20;
short count = 0;
short hourCounter = 0;
void setup() {
  Serial.begin(57600);
  Rtc.Begin();
  lcd.init();  // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...!");
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);

  if (!Rtc.IsDateTimeValid()) {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  // lcd.clear();
  currentMillis = millis();
  startMillis = millis();
}


void loop() {
  currentMillis = millis();
  if (currentMillis - startMillis >= period) {
    count++;
    getDateTime();
    startMillis = currentMillis;
  }
  if (count >= 20) {
    count = 0;
    showDay = true;
  }
  if (showDay && count >= 5) {
    count = 0;
    showDay = false;
  }
}

void getDateTime() {
  RtcDateTime now = Rtc.GetDateTime();
  printDateTimeToLCD(now);
  Serial.println();
  if (Serial.available() > 0) {
    const String incomingByte = Serial.readString();
    RtcDateTime newDateAndTime = RtcDateTime(__DATE__, incomingByte.c_str());
    Rtc.SetDateTime(newDateAndTime);
    Serial.print("Set Time: ");  //Format for time 00:00:00
    Serial.println(incomingByte);
  }
  if (!now.IsValid()) {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }
}

void printDateTime(const RtcDateTime& dt) {
  char datestring[26];

  snprintf_P(datestring,
             sizeof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second());
  Serial.print(datestring);
}

void printDateTimeToLCD(const RtcDateTime& dt) {
  char datestring[16];
  char timestring[16];
  char dateTimeData[20];
  char monthInWord[10];
  char dayInWord[10];
  char meridiem[3];

  unsigned int hour = dt.Hour();
  unsigned int minute = dt.Minute();
  unsigned int second = dt.Second();
  unsigned int month = dt.Month();
  unsigned int day = dt.Day();
  unsigned int year = dt.Year();
  unsigned int dayOfWeek = dt.DayOfWeek();

  if (hour < 12) {
    strcpy(meridiem, "AM");  // Copy "AM" into meridiem
  } else {
    strcpy(meridiem, "PM");  // Copy "PM" into meridiem
  }
  if (hour < 1 || hour > 23) {
    // midnight is 12am
    hour = 12;
    strcpy(meridiem, "AM");
  } else if (hour < 12) {
    strcpy(meridiem, "AM");
  } else if (hour > 12) {
    hour = hour - 12;
    strcpy(meridiem, "PM");
  } else {
    // noon is 12pm
    hour = 12;
    strcpy(meridiem, "PM");
  }

  if (minute == 59 && second == 59) {
    hourCounter++;
  }
  //This is to reduced 3 seconds of time every 6 hours
  //As observed with rtc 1302, it is 3s ahead for every 6 hours
  if (hourCounter >= 6 && second >= 6) {
    char newTimeString[16];
    hourCounter = 0;
    second -= 3;
    snprintf(newTimeString, sizeof(newTimeString), "%02u:%02u:%02u", hour, minute, second);
    RtcDateTime newDateAndTime = RtcDateTime(__DATE__, newTimeString);
    Rtc.SetDateTime(newDateAndTime);
  }

  strncpy(monthInWord, Months[month - 1], sizeof(monthInWord) - 1);
  strncpy(dayInWord, Days[dayOfWeek], sizeof(dayInWord) - 1);

  snprintf(dateTimeData, sizeof(dateTimeData), "%s %02u, %04u", monthInWord, day, year);
  snprintf(timestring, sizeof(timestring), "%02u:%02u:%02u %s", hour, minute, second, meridiem);
  String upperText = showDay ? String(dayInWord) + "         " : dateTimeData;

  lcd.setCursor(0, 0);
  lcd.print(upperText);
  lcd.setCursor(0, 1);
  lcd.print(timestring);
}
