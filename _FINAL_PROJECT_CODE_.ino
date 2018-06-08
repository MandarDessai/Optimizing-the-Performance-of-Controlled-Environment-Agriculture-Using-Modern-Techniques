#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <SoftwareSerial.h>

#include <TimeLib.h>
#include <TimeAlarms.h>
SoftwareSerial mySerial(3,2);                              // pin 2 = TX, pin 3 = RX (unused)
#include <Wire.h>

#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#define DHTPIN1 2                                          //The digital pin to which sensor1 is connected
//#define DHTPIN2 24                                         //The digital pin of mcp to which sensor2 is connected
#define DHTTYPE DHT22
DHT dht1(DHTPIN1, DHTTYPE);
//DHT dht2(DHTPIN2, DHTTYPE);

//Analog pins to take moisture and temperature sensor inputs
int moistPin1 = A0;
int moistPin2 = A1;
int moistPin3 = A2;
int tempPin1 = A3; 
int tempPin2 = A4; 

//const int ledPin=13;

//thresholds for irrigation,fogger and cooling systems
int threshold_moisture = 400;
int threshold_humidity = 30;
int threshold_temperature = 20;

//timer variables
long timer;
int flag=0;
int c;

//to start the timer
void start_timer()
{
  timer=millis()/60000;
  flag=1;
}

//to check if the delay interval has elapsed
int check_timer(long long x)
{
  long a,b;
  if(flag==1)
  {
    a=millis()/60000;
    b=a-timer;
    if(b<x)
      return (0);
    else
      return (1);
   }
}

//to clear the timer after the delay interval has elapsed
void clear_timer()
{
  timer=0;
  flag=0;
}

//to open
void MorningAlarm()
{
 digitalWrite(8,HIGH);
 digitalWrite(10,HIGH);
 Alarm.delay(3000);         //find this duration by hit and trial
 digitalWrite(8,LOW);
 digitalWrite(10,LOW);    
}

//to close
void EveningAlarm()
{
  digitalWrite(9,HIGH);
  digitalWrite(11,HIGH);
  Alarm.delay(3000);          //find this duration by hit and trial
  digitalWrite(9,LOW);
  digitalWrite(11,LOW);         
} 

//to supply fertilizers
void fertigation()
{
  digitalWrite(3,HIGH);         //power supply
  digitalWrite(4,HIGH);         //pump
  digitalWrite(5,HIGH);         //moisture solenoid relay
  digitalWrite(12,HIGH);        //fertigation solenoid relay
  start_timer();
  while(1)
  {
    
    c=check_timer(20);            //ask the fertilizer duration
    if(c==1)
    {
      digitalWrite(12,LOW);       //fertigation solenoid relay
      digitalWrite(5,LOW);        //moisture solenoid relay
      digitalWrite(4,LOW);        //pump 
      digitalWrite(3,LOW);        //power supply 
      clear_timer();
      break;
    }
  }
}

void setup() 
{
 Serial.begin(9600);
 dht1.begin();
 
 pinMode(ledPin,OUTPUT);
 
 pinMode(3,OUTPUT);         //230v main supply relay
 pinMode(4,OUTPUT);         //pump relay
 pinMode(5,OUTPUT);         //moisture solenoid relay
 pinMode(6,OUTPUT);         //fan relay
 pinMode(7,OUTPUT);         //fogger solenoid relay
 pinMode(8,OUTPUT);         //motors
 pinMode(9,OUTPUT);
 pinMode(10,OUTPUT);
 pinMode(11,OUTPUT);
 pinMode(12,OUTPUT);        //fertigation solenoid relay
 pinMode(13,OUTPUT);        //all sensors 5v adapter relay
 
 //taking current time from RTC
 DateTime now = rtc.now();
 int a1=now.year();
 int a2=now.month();
 int a3=now.day();
 int a4=now.hour();
 int a5=now.minute();
 int a6=now.second();

 //setting current time as the inputs from rtc
 setTime(a4,a5,a6,a2,a3,a1);                        
 
 Alarm.alarmRepeat(7,30,00, MorningAlarm);      // 7:30am every day-day night control open
 Alarm.alarmRepeat(17,45,0,EveningAlarm);       // 5:45pm every day-day night control close
 Alarm.alarmRepeat(8,30,0,fertigation);         //supplying fertilizer everyday at 8:30am
}

void loop() 
{
 //make a pin high to put on the sensors
  digitalWrite(13,HIGH);

 //reading the 2 humidity sensors
 float humidity_value1;
 humidity_value1= dht1.readHumidity();
// float humidity_value2;
 //humidity_value2= dht2.readHumidity();
 //To print on serial monitor
 Serial.print ("Humidity: ");
 Serial.print (humidity_value1);
 Serial.print (" %");
 Serial.println();

//reading the 2 temperature sensors
 float temp_value1;
 float temp_value2;
 temp_value1=analogRead(tempPin1);                           //Reading the sensor1 input
 temp_value1=temp_value1*0.48828125;                         //Converting voltage to temperature of sensor1
 temp_value2=analogRead(tempPin2);                           //Reading the sensor2 input
 temp_value2=temp_value2*0.48828125;                         //Converting voltage to temperature of sensor2
 //To print on serial monitor
 Serial.print("Temperature=");               
 Serial.print(temp_value1);
 Serial.print("*C");
 Serial.println();

 //reading the 3 moisture sensors
 int moist_value1;
 int moist_value2;
 int moist_value3;
 moist_value1 = analogRead(moistPin1);
 moist_value2 = analogRead(moistPin2);
 moist_value3 = analogRead(moistPin3);
 Serial.print ("Moisture ");
 Serial.print (moist_value1);
 Serial.print (moist_value2);
 Serial.print (moist_value3);
 Serial.print (" %");
 Serial.println();

//make the pin low to put off the sensors
digitalWrite(13,LOW);

//irrigation system
int moist_value=(moist_value1+moist_value2+moist_value3)/3;
if (moist_value < threshold_moisture)
{
  //on solenoid and the pump
  digitalWrite(3,HIGH);               //main supply
  digitalWrite(4,HIGH);               //pump
  digitalWrite(5,HIGH);               //solenoid
  Serial.print ("Moisture is less ");
  Serial.println();
  digitalWrite(ledPin,HIGH);
  start_timer();
  while(1)
  {
    c=check_timer(5);                 //water on for 5 minutes
    if(c==1)
    {
      //close solenoid and off pump
      digitalWrite(5,LOW);               //solenoid
      digitalWrite(4,LOW);               //pump
      digitalWrite(3,LOW);               //supply
      digitalWrite(ledPin,LOW);
      clear_timer();
      break;
    }
  }
}

//cooling system
int temp_value=(temp_value1+temp_value2)/2;
if (temp_value > threshold_temperature)
{
  //on the fan
  digitalWrite(3,HIGH);                  //main supply
  digitalWrite(6,HIGH);                  //on fan
  Serial.print ("Temperature is high ");
  Serial.println();
  digitalWrite(ledPin,HIGH);
  start_timer();
  while(1)
  {
    c=check_timer(10);              //fan on for 10 minutes
    if(c==1)
    {
      //off the fan
      digitalWrite(6,LOW);          //off fan
      digitalWrite(3,LOW);          //main supply
      digitalWrite(ledPin,LOW);
      clear_timer();
      break;
      }
  }
}

//fogger system
//int humidity_value=(humidity_value1+humdity_value2)/2;
if (humidity_value1 < threshold_humidity)
{
  //on solenoid and the pump
  Serial.print ("humidity is less ");
  Serial.println();
  digitalWrite(ledPin,HIGH);
  digitalWrite(3,HIGH);             //power supply
  digitalWrite(4,HIGH);             //pump
  digitalWrite(7,HIGH);             //fogger relay
  start_timer();
  while(1)
  {
    c=check_timer(2);               //fogger on for 2 minutes
    if(c==1)
    {
      //close solenoid and off pump
      digitalWrite(ledPin,LOW);
      digitalWrite(7,LOW);             //fogger relay
      digitalWrite(4,LOW);             //pump
      digitalWrite(3,LOW);             //power supply
      clear_timer();
      break;
    }
  }
}

 //2 hours delay
 start_timer();
 while(1)
  {
    
    c=check_timer(120);
    /*Serial.print (c);
    Serial.println();*/
    if(c==1)
    {
      clear_timer();
      break;
    }
  }
 
}
