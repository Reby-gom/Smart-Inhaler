
//include necessary libraries
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "DSRTCLib.h"

//Logger Setup
const int chipSelect = 10;

//RTC Setup
int ledPin =  13;    // LED connected to digital pin 13
int INT_PIN = 3;     // INTerrupt pin from the RTC. On Arduino Uno, this should be mapped to digital pin 2 or pin 3, which support external interrupts
int int_number = 1; // On Arduino Uno, INT0 corresponds to pin 2, and INT1 to pin 3
DS1339 RTC = DS1339();

//Screen Setup 
TinyScreen display = TinyScreen(TinyScreenDefault); 
int counter = 0;

// Global Variables
int buttonState = 0;           // Variable for reading button
const int inahlerCap_pin = 5;  // Input pin for touch state

void setup() {
  
  pinMode(ledPin, OUTPUT);    
  digitalWrite(ledPin, LOW);
  pinMode(inahlerCap_pin, INPUT_PULLUP);
  Serial.begin(9600);
  Wire.begin();

  // Initialize SD
  Serial.print("Initializing SD card...");

  // Check if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // Open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  
  if (dataFile){
    dataFile.println("......................");
    dataFile.println("Date, Time, Count");
    dataFile.close(); 
  }
  else
  {
    Serial.println("Couldn't open data file");
    return;
  }

  // Initialize RTC
  RTC.start(); 
  set_time();  // **COMMENT OUT THIS LINE AFTER YOU SET THE TIME** 
}

int read_int(char sep)
{
  static byte c;
  static int i;

  i = 0;
  while (1)
  {
    while (!Serial.available())
    {;}
 
    c = Serial.read();
    // Serial.write(c);
  
    if (c == sep)
    {
      // Serial.print("Return value is");
      // Serial.println(i);
      return i;
    }
    if (isdigit(c))
    {
      i = i * 10 + c - '0';
    }
    else
    {
      Serial.print("\r\nERROR: \"");
      Serial.write(c);
      Serial.print("\" is not a digit\r\n");
      return -1;
    }
  }
}

int read_int(int numbytes)
{
  static byte c;
  static int i;
  int num = 0;

  i = 0;
  while (1)
  {
    while (!Serial.available())
    {;}
 
    c = Serial.read();
    num++;
    // Serial.write(c);
  
    if (isdigit(c))
    {
      i = i * 10 + c - '0';
    }
    else
    {
      Serial.print("\r\nERROR: \"");
      Serial.write(c);
      Serial.print("\" is not a digit\r\n");
      return -1;
    }
    if (num == numbytes)
    {
      // Serial.print("Return value is");
      // Serial.println(i);
      return i;
    }
  }
}

int read_date(int *year, int *month, int *day, int *hour, int* minute, int* second)
{

  *year = read_int(4);
  *month = read_int(2);
  *day = read_int(' ');
  *hour = read_int(':');
  *minute = read_int(':');
  *second = read_int(2);

  return 0;
}


void currentTime(String &nowDate, String &nowTime){
   RTC.readTime();
   nowDate = String(int(RTC.getMonths())) + "/" + String (int(RTC.getDays())) + "/" + String(RTC.getYears()-2000);
   nowTime = String(int(RTC.getHours())) + ":" + String(int(RTC.getMinutes())) + ":" + String(int(RTC.getSeconds()));  
}  
  

void loop() {
  
  // Getting date and time
  String ahoraDate;
  String ahoraTime;
  currentTime(ahoraDate, ahoraTime);
  String ahora = ahoraDate + "  " + ahoraTime;
  
  // Read inhaler cap pin
  buttonState = digitalRead(inahlerCap_pin);
  
  if (buttonState == LOW){

    // Generate a combined String from Date, Time, and Sensor Count
    String dataString = "";
    dataString = String(ahoraDate)  + ", " + String(ahoraTime)  + ", "  + String(counter);
    
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    // Write the saved String to the microSD card
    if(dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println(dataString);
    }
    else
    {
      Serial.println("Couldn't access file");
    }
    
    // delay controls how long it takes the controller to loop back through the code (in ms)
    delay(7000);
    counter++;
  }

}

void set_time()
{
    Serial.println("Enter date and time (YYYYMMDD HH:MM:SS)");
    int year, month, day, hour, minute, second;
    int result = read_date(&year, &month, &day, &hour, &minute, &second);
    
    if (result != 0) {
      Serial.println("Date not in correct format!");
      return;
    } 
                           
    RTC.setSeconds(second);
    RTC.setMinutes(minute);
    RTC.setHours(hour);
    RTC.setDays(day);
    RTC.setMonths(month);
    RTC.setYears(year);
    RTC.writeTime();
    read_time();
}

void read_time() 
{
  Serial.print ("The current time is ");
  RTC.readTime();                                          
  printTime(0);
  Serial.println();
  
}


void printTime(byte type)
{
                                                          
  if(!type)
  {
    Serial.print(int(RTC.getMonths()));
    Serial.print("/");  
    Serial.print(int(RTC.getDays()));
    Serial.print("/");  
    Serial.print(RTC.getYears());
  }
  else
  {
                                                                                                                 
    {
      Serial.print(int(RTC.getDayOfWeek()));
      Serial.print("th day of week, ");
    }
          
    {
      Serial.print(int(RTC.getDays()));
      Serial.print("th day of month, ");      
    }
  }
  
  Serial.print("  ");
  Serial.print(int(RTC.getHours()));
  Serial.print(":");
  Serial.print(int(RTC.getMinutes()));
  Serial.print(":");
  Serial.print(int(RTC.getSeconds()));  
}



