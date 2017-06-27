
#define  BLACK           0x00
#define BLUE            0xE0
#define RED             0x03
#define GREEN           0x1C
#define DGREEN           0x0C
#define YELLOW          0x1F
#define WHITE           0xFF
#define ALPHA           0xFE
#define BROWN           0x32

  
#include <TinyScreen.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include "DSRTCLib.h"

//Logger Setup
const int chipSelect = 10;

//DSRTC Setup
int ledPin =  13;    // LED connected to digital pin 13
int INT_PIN = 3; // INTerrupt pin from the DSRTC. On Arduino Uno, this should be mapped to digital pin 2 or pin 3, which support external interrupts
int int_number = 1; // On Arduino Uno, INT0 corresponds to pin 2, and INT1 to pin 3

DS1339 DSRTC = DS1339();

int counter = 1;

const int hallPin = 5;  // Input pin for touch state
int statePin = LOW;

// Global Variables
int buttonState = 0;             // Variable for reading button

//Screen Setup 
TinyScreen display = TinyScreen(TinyScreenPlus); 

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);    
  digitalWrite(ledPin, LOW);
  pinMode(hallPin,INPUT);
  SerialUSB.begin(9600);
  Wire.begin();
  display.begin();
  //Initialize SD
  SerialUSB.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    SerialUSB.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  SerialUSB.println("card initialized.");

    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile){
    dataFile.println("......................");
    dataFile.println("Date, Time, Count");
    dataFile.close(); 
  }
  else
  {
    SerialUSB.println("Couldn't open data file");
    return;
  }

  //Initialize DSRTC

  DSRTC.start(); 
  //set_time();  // **COMMENT OUT THIS LINE AFTER YOU SET THE TIME** 
}

int read_int(char sep)
{
  static byte c;
  static int i;

  i = 0;
  while (1)
  {
    while (!SerialUSB.available())
    {;}
 
    c = SerialUSB.read();
    // SerialUSB.write(c);
  
    if (c == sep)
    {
      // SerialUSB.print("Return value is");
      // SerialUSB.println(i);
      return i;
    }
    if (isdigit(c))
    {
      i = i * 10 + c - '0';
    }
    else
    {
      SerialUSB.print("\r\nERROR: \"");
      SerialUSB.write(c);
      SerialUSB.print("\" is not a digit\r\n");
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
    while (!SerialUSB.available())
    {;}
 
    c = SerialUSB.read();
    num++;
    // SerialUSB.write(c);
  
    if (isdigit(c))
    {
      i = i * 10 + c - '0';
    }
    else
    {
      SerialUSB.print("\r\nERROR: \"");
      SerialUSB.write(c);
      SerialUSB.print("\" is not a digit\r\n");
      return -1;
    }
    if (num == numbytes)
    {
      // SerialUSB.print("Return value is");
      // SerialUSB.println(i);
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
   DSRTC.readTime();
   nowDate = String(int(DSRTC.getMonths())) + "/" + String (int(DSRTC.getDays())) + "/" + String(DSRTC.getYears()-2000);
   nowTime = String(int(DSRTC.getHours())) + ":" + String(int(DSRTC.getMinutes())) + ":" + String(int(DSRTC.getSeconds()));  
}  
  

void loop() {
  // put your main code here, to run repeatedly:
  String ahoraDate;
  String ahoraTime;
  currentTime(ahoraDate, ahoraTime);
  String ahora = ahoraDate + "  " + ahoraTime;
  
  display.setCursor(0,0);
  display.fontColor(YELLOW,BLACK);
  display.print("Last Used:");
  display.setCursor(0,10);
  display.setFont(liberationSans_8ptFontInfo);
  display.setCursor(0,30);
  display.fontColor(YELLOW,BLACK);
  display.print("Counter: ");
  
  statePin = digitalRead(hallPin);
  SerialUSB.println("State: ");
  SerialUSB.print(statePin);
  
  if (statePin == LOW){

    //Logging
    String dataString = "";
    dataString = String(ahoraDate)  + ", " + String(ahoraTime)  + ", "  + String(counter);
    
    // if the file is available, write to it:
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if(dataFile)
    {
      dataFile.println(dataString);
      dataFile.close();
      SerialUSB.println(dataString);
      display.setCursor(0,0);
      display.fontColor(YELLOW,BLACK);
      display.print("Last Used:");
      display.setCursor(0,10);
      display.print(ahora);
      display.setFont(liberationSans_8ptFontInfo);
      display.setCursor(0,30);
      display.fontColor(YELLOW,BLACK);
      display.print("Counter: ");
      display.print(counter);
    }
    else
    {
      SerialUSB.println("Couldn't access file");
    }
    counter++;
    delay(7000);
    
  }

}

void set_time()
{
    SerialUSB.println("Enter date and time (YYYYMMDD HH:MM:SS)");
    int year, month, day, hour, minute, second;
    int result = read_date(&year, &month, &day, &hour, &minute, &second);
    if (result != 0) {
      SerialUSB.println("Date not in correct format!");
      return;
    } 
    
                             
    DSRTC.setSeconds(second);
    DSRTC.setMinutes(minute);
    DSRTC.setHours(hour);
    DSRTC.setDays(day);
    DSRTC.setMonths(month);
    DSRTC.setYears(year);
    DSRTC.writeTime();
    read_time();
}

void read_time() 
{
  SerialUSB.print ("The current time is ");
  DSRTC.readTime();                                          
  printTime(0);
  SerialUSB.println();
  
}


void printTime(byte type)
{
                                                          
  if(!type)
  {
    SerialUSB.print(int(DSRTC.getMonths()));
    SerialUSB.print("/");  
    SerialUSB.print(int(DSRTC.getDays()));
    SerialUSB.print("/");  
    SerialUSB.print(DSRTC.getYears());
  }
  else
  {
                                                                                                                 
    {
      SerialUSB.print(int(DSRTC.getDayOfWeek()));
      SerialUSB.print("th day of week, ");
    }
          
    {
      SerialUSB.print(int(DSRTC.getDays()));
      SerialUSB.print("th day of month, ");      
    }
  }
  
  SerialUSB.print("  ");
  SerialUSB.print(int(DSRTC.getHours()));
  SerialUSB.print(":");
  SerialUSB.print(int(DSRTC.getMinutes()));
  SerialUSB.print(":");
  SerialUSB.print(int(DSRTC.getSeconds()));  
}



