#define N_TO_AVERAGE 10
#define P_LED1 12 // Green LED and heater
#define P_LED2 13 // Red LED
#define P_SEL 4
#define P_TARG 4
#define T_PAUSE 1000 // pause in milliseconds

#include <LiquidCrystal.h>      // for LCD
#include <OneWire.h>            // required for thermistor
#include <DallasTemperature.h>  // required for thermistor
#include <avrTimers328.h>
#include <avr/wdt.h>

//LCD
LiquidCrystal Lcd(5, 6, 7, 8, 9, 10);
// Thermostat
OneWire oneWire1(2); // Set up oenWire instance to the thermometer
DallasTemperature sensors1(&oneWire1); // Pass the oneWire reference to Dallas Temperature.
OneWire oneWire2(3); // Set up oenWire instance to the thermometer
DallasTemperature sensors2(&oneWire2); // Pass the oneWire reference to Dallas Temperature.

// Timer
unsigned long t=0;

// Temperature Recording/Displaying
int Selector=0;
float AvgTs[9];
unsigned long TimeStamps[9];
unsigned long AvgTimeStamp=0;
int LoopCount=0;



void setup() {
// WATCHDOG
    wdt_reset();
    WDTCSR |= 0b00011000;
    WDTCSR = 0b01101001; //15.9.2, page 80
    wdt_reset();
// LCD
    Lcd.begin(16, 2);           // set up LCD with 16 columns and 2 rows
// PIN SETUP
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(P_SEL, INPUT);
    pinMode(P_LED1, OUTPUT);
    pinMode(P_LED2, OUTPUT);
// CYCLE THE LED'S TO MAKE SURE THEY WORK    
    digitalWrite(P_LED1,HIGH);    digitalWrite(P_LED2,HIGH);  
    delay(200);
    digitalWrite(P_LED1,HIGH);    digitalWrite(P_LED2,LOW);  
    delay(200);
    digitalWrite(P_LED1,HIGH);    digitalWrite(P_LED2,HIGH); 
    delay(800);
    digitalWrite(P_LED1,LOW);    digitalWrite(P_LED2,HIGH);
    delay(200);
    digitalWrite(P_LED1,HIGH);    digitalWrite(P_LED2,HIGH); 
// THERMOMETERS
    sensors1.begin();
    sensors2.begin();
  // TIMER
    atMega328P_T1(0b101,15625-1);
    t = 0;
    wdt_reset();
} // SETUP

void loop() {
// WATCHDOG
    if(t<(60*10)){
    wdt_reset();}

    int T1, T2, intTarget, heatOn, takeAPause;
    float T, target;
// GET INPUTS AND ACT ACCORDINGLY  
    if(Selector!=digitalRead(P_SEL)){
        Lcd.clear();
        Lcd.setCursor(4,0);
        if(Selector){ Lcd.print("FERMENTING");}
        else {Lcd.print("BREWING");}
        delay(1200);
        Lcd.clear();
        }
    Selector = digitalRead(P_SEL);
    target = 60+(20.0+Selector*132.0)*analogRead(P_TARG)/1023; //60-80 if fermenting, 60-212 if brewing
// GET TEMPERATURES
    sensors1.requestTemperatures();
    sensors2.requestTemperatures();
    T1 = sensors1.getTempCByIndex(0) * 9 / 5 + 32;
    T2 = sensors2.getTempCByIndex(0) * 9 / 5 + 32;

// HEATER CONTROL
    T = (max(T1, 0) + max(T2, 0)) / ((T1 > 0) + (T2 > 0)) + 0.0;
    heatOn = ((T < target) && (T > 0));
    takeAPause = (heatOn == digitalRead(12)); // If the heater changes state, we shold wait for a sec so we don't click a bunch
    digitalWrite(P_LED1, 1 - heatOn); // Putting th pin low turns the LED on. This pin doubles as a heater & the LED
    digitalWrite(P_LED2,(T>(target-2.0)) & (T<(target+2.0))); // putting the pin low turns the LED on 
    delay(T_PAUSE * takeAPause);
    Lcd.setCursor(14,0); Lcd.print(T);

// DISPLAY
        // Show T1
        Lcd.setCursor(0, 0);    Lcd.print("1:   ");
        if(T1<-195){Lcd.setCursor(2,0); Lcd.print("ERR");}
        else{Lcd.setCursor(2+(T1<=100), 0);    Lcd.print(T1);}
        //Show T2
        Lcd.setCursor(0, 1);    Lcd.print("2:   ");
        if(T2<-195){Lcd.setCursor(2,1); Lcd.print("ERR");}
        else{Lcd.setCursor(2+(T2<=100), 1);    Lcd.print(T2);}
  
        Lcd.setCursor(7, 0);     Lcd.print("G:    ");
        Lcd.setCursor(9+(target<=100), 0);    Lcd.print((int) target);
        Lcd.setCursor(7, 1);    Lcd.print("T:   ");
        Lcd.setCursor(9+(T<=100), 1);    Lcd.print((int) T);
        Lcd.setCursor(13, 1);    Lcd.print((int) t);  
}
 
ISR(TIMER1_COMPA_vect){ // put interrupt code here
    t++;
    }
