// TODO:
//Webbedienung
//Schalttensiometer

#include <Wire.h> 
#include "RTClib.h"
//#include <Adafruit_MCP23017.h> //Portexpander

RTC_DS3231 rtc;

//Uhrzeit/ Tag
char daysOfTheWeek[7][12] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
bool syncOnFirstStart = false; // true, falls die Zeitinformationen der RTC mit dem PC synchronisiert werden sollen.                            
unsigned long previousMillis = 0;       
const long interval = 10000;  // 10s - Intervall der Uhrzeitausgabe ueber den Serialport
//Bewaesserung
int timer = 600;  // in Sekunden - Dauer der Bewaesserung
#define frequency 2 
int zeitpunkt[frequency] = {8,20}; // in volle Uhrzeit - Start der Bewaesserung
int timerManuell = 600; //in Sekunden - Dauer der Bewaesserung bei manueller Betaetigung (Button)
//Variablen für die Verarbeitung
int x = 0;
int water = 0;
int yesterday[frequency];
unsigned long manuell = 0;
unsigned long automatik = 0; 
int aButtonPressed = 0;   

//MCP23017 Portexpander
//Adafruit_MCP23017 mcp1;

// Pinbelegung
int relais = 2;
int button = 13;
                               
void setup () {
  pinMode(button, INPUT);
  pinMode(relais, OUTPUT);

  Serial.begin(9600);

  delay(3000); // Warte auf Terminal

  if (! rtc.begin()) {
    Serial.println("Kann RTC nicht finden");
    while (1);
  }

  if (rtc.lostPower() || syncOnFirstStart) {
    Serial.println("Die RTC war vom Strom getrennt. Die Zeit wird neu synchronisiert.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

/*
  mcp1.begin();
  for(byte m = 0; m < 16; m++){
    mcp1.pinMode(m, OUTPUT); // Define m on MCP1 as Output
  }
  */
}
void loop () {
  //Ausgabe des Datums und Uhrzeit über SerialPort alle $interval Millisekunden
  DateTime now = rtc.now();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you when it runs
    previousMillis = currentMillis;
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
  
  /*
   * Relaissteuerung
   */ 
  // Taegliche Bewässerungsroutine
  // Einschalten zu $zeitpunkt Uhr
  if(x < frequency){
  if(now.day() > yesterday[x] and now.hour() == zeitpunkt[x]){
    yesterday[x] = now.day();
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    Serial.println("Wasser an");
    automatik = now.unixtime(); 
    water = 1; 
  } 
    x = x+1;
  }else{
    x = 0;
  }
  //Ausschalten nach $timer Minuten
  if (now.unixtime() >= (automatik+timer) and water == 1 and manuell == 0){
    Serial.println("Wasser aus");
    water = 0;
  }
  
  //Manuelle Steuerung - Knopf am Kasten  
  if (digitalRead(button) == HIGH){
    Serial.println("Manuelle Bewässerung");
    aButtonPressed = 1;    
  }
  if(digitalRead(button) == HIGH and aButtonPressed  == 1){
    aButtonPressed = 0;
    delay(10);
     // Einschalten
    if(water == 0){
      water = 1;
      manuell = now.unixtime();
      Serial.println("Wasser an - manuell");
    }else{
      //Ausschalten
      water = 0;
      manuell = 0;
      Serial.println("Wasser aus - manuell");
    }
  }
 
  //Ausschalten der Manuellen Bewässerung nach $timerManuell sekunden 
  if(now.unixtime() >= (manuell+timerManuell) and manuell != 0){
    Serial.println("Wasser aus - Timer manuell");
    water = 0;
    manuell = 0;
  }

  // Relais schalten
  if(water == 1){
    //mcp1.digitalWrite(pin, HIGH);
    Serial.println("Wasser an");
    digitalWrite(relais, HIGH);
      
  }
  if(water == 0){
    digitalWrite(relais, LOW);
    //mcp1.digitalWrite(pin, LOW); 
    Serial.println("Wasser aus"); 
  }
  delay(500);
  
}
