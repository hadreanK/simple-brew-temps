#define N_TO_AVERAGE 10
#define P_LED1 12
#define P_LED2 13
#define P_SEL 4
#define P_TARG 4
#include <LiquidCrystal.h>      // for LCD
#include <OneWire.h>            // required for thermistor
#include <DallasTemperature.h>  // required for thermistor
#include <avrTimers328.h>
//LCD
//LiquidCrystal Lcd(48,49,50,51,52,53);
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

  // LCD
  Lcd.begin(16, 2);           // set up LCD with 16 columns and 2 rows

  //Thermostat...............................................................................................................
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(P_SEL, INPUT);
  pinMode(P_LED1, OUTPUT);
  pinMode(P_LED2, OUTPUT);
  digitalWrite(P_LED1,HIGH);
  digitalWrite(P_LED2,HIGH);  
  delay(1500);
  digitalWrite(P_LED1,LOW);
  digitalWrite(P_LED2,HIGH);
  delay(1500);
  digitalWrite(P_LED1,HIGH);
  digitalWrite(P_LED2,LOW);  
  delay(1500);

  sensors1.begin();
  sensors2.begin();

  // Timer
  atMega328P_T1(0b101,15625-1);

}

void loop() {
  //variables
  int T1, T2, intTarget, heatOn, takeAPause;
  float T, target;
  target = 60+20*analogRead(P_TARG)/1024;
  //intTarget = analogRead(P_TARG);
  if(Selector!=digitalRead(P_SEL)){Lcd.clear();}
  Selector = digitalRead(P_SEL);

  sensors1.requestTemperatures();
  T1 = sensors1.getTempCByIndex(0) * 9 / 5 + 32;
   

  sensors2.requestTemperatures();
  T2 = sensors2.getTempCByIndex(0) * 9 / 5 + 32;
  // Heat Control
  //T1 = T1 * (T1 < 100);
  //T2 = T2 * (T2 < 100);
  T = (max(T1, 0) + max(T2, 0)) / ((T1 > 0) + (T2 > 0)) + 0.0;
  heatOn = (T < target) && (T > 0);
  takeAPause = (heatOn == digitalRead(12));
  digitalWrite(P_LED1, 1 - heatOn); // Putting th pin low turns the LED on
  digitalWrite(P_LED2,(T>(target-2.0)) & (T<(target+2.0))); // putting the pin low turns the LED on 
  delay(3000 * takeAPause);

// AVERAGING
    if(LoopCount<N_TO_AVERAGE){ // If it's not time to average the temperatures yet
    AvgTs[0] = AvgTs[0] + T;
    TimeStamps[0] = t;
    }else{ // If it's time to average the temperatures
        AvgTs[0] = AvgTs[0]/N_TO_AVERAGE;
        for(int i = 8; i>=0; i=i-1){
             AvgTs[i] = AvgTs[i-1];
        }
        for(int i = 8; i>=0; i=i-1){
             TimeStamps[i] = TimeStamps[i-1];
        } 
        LoopCount = -1;        
    }

// DISPLAY
  // If Display Temperature History
  if (Selector == 0) {
        Lcd.clear();
        Lcd.setCursor(12,0);    Lcd.print((int) (TimeStamps[1] - TimeStamps[8])/60);
            
            //((TimeStamps[1]-TimeStamps[8])/60));
        Lcd.setCursor(12,1);    Lcd.print(AvgTimeStamp/60);
//Lcd.setCursor(12,0);    Lcd.print((int) AvgTs[0]);
//Lcd.setCursor(12,1);    Lcd.print((int) AvgTs[1]);
        for(int i = 0; i<4; i++){
             Lcd.setCursor(i*3,0); Lcd.print((int) AvgTs[i+1]);
             Lcd.setCursor(i*3,1); Lcd.print((int) AvgTs[i+4]);
//             Lcd.setCursor(i*3,0); Lcd.print((int) TimeStamps[i+1]);
//             Lcd.setCursor(i*3,1); Lcd.print((int) TimeStamps[i+4]);             
        }
  } else { // Otherwise show current temps and stuff
        // Show T1
        Lcd.setCursor(0, 0);    Lcd.print("1:   ");
        Lcd.setCursor(2, 0);    Lcd.print(T1);
        //Show T2
        Lcd.setCursor(0, 1);    Lcd.print("2:   ");
        Lcd.setCursor(2, 1);    Lcd.print(T2);
  
        Lcd.setCursor(7, 0);     Lcd.print("G:    ");
        Lcd.setCursor(9, 0);    Lcd.print((int) target);
        Lcd.setCursor(7, 1);    Lcd.print("T:   ");
        Lcd.setCursor(9, 1);    Lcd.print((int) T);
        Lcd.setCursor(13, 1);    Lcd.print((int) t/60);  }
  
LoopCount++;
} // loop

ISR(TIMER1_COMPA_vect){ // put interrupt code here
    t++;
    }
