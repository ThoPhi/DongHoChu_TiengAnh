#include <Time.h>
#include <Wire.h>  
#include <DS1307RTC.h>
#include <SPI.h>

#define IT_IS  22
#define OCLOCK  21
#define MIN_HALF  1
#define MIN_TEN  0
#define MIN_QUARTER  2
#define MIN_TWENTY  3
#define MIN_FIVE  6
#define MINUTE  5
#define TO    4
#define PAST  7
#define ONE  8
#define THREE  9 
#define TWO  10
#define FOUR  11
#define FIVE  12 
#define SIX  13
#define SEVEN  14
#define EIGHT  15
#define NINE  16
#define TEN  17
#define ELEVEN  18
#define TWELVE  19

#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

byte pinCS = 10;
byte storeBuffer[3];
byte countIndex;
byte i;
byte count;
int lastMinute;
byte mode ;
long lastLongTimePeriod;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  pinMode(pinCS,OUTPUT);  
  Serial.print(TIME_REQUEST);  
  delay(1000);
  if(Serial.available()){
    processSyncMessage();
  }
  else setSyncProvider(RTC.get);   // the function to get the time from the RTC    
  //setTime(9,15,58,19, 12, 2014);

}

void loop() {
  long currentTime = millis();
  if(currentTime - lastLongTimePeriod > 1){
    lastLongTimePeriod = currentTime;
    if(mode ==0){
      time_process();
    }
  }  
  // put your main code here, to run repeatedly: 


}

void send_to_clock(){
  digitalWrite(pinCS,LOW);
  for(i=0;i<3;i++){
    SPI.transfer(storeBuffer[i]);
  }
  digitalWrite(pinCS,HIGH);
}

void clear_buffer(){
  for(byte i=0;i<3;i++){
    storeBuffer[i] = 0;
  }
}

void buffer_set(byte ledIndex){
  if (ledIndex >23) return;
  byte byteIndex = ledIndex/8;
  storeBuffer[byteIndex] |= 1<<(ledIndex%8); 
}

void time_process(){
  int currentHour = hour();
  int currentMinute = minute();
  if(lastMinute != currentMinute){
    Serial.print("Now is: ");
    Serial.print(currentHour);
    Serial.print(" ");
    Serial.println(currentMinute);

    clear_buffer();
    //minute display
    if(currentMinute >32){
      if(currentHour<23) currentHour ++;
      else currentHour = 1;

      switch(currentMinute){
      case 33 ... 37:
        buffer_set(MIN_TWENTY);
        buffer_set(MIN_FIVE);
                buffer_set(MINUTE);
        buffer_set(TO);
        break;
      case 38 ... 42:
        buffer_set(MIN_TWENTY);
        buffer_set(MINUTE);
        buffer_set(TO);
        break;
      case 43 ... 47:
        buffer_set(MIN_QUARTER);
        buffer_set(TO);
        break;
      case 48 ... 52:
        buffer_set(MIN_TEN);
        buffer_set(MINUTE);
        buffer_set(TO);
        break;
      case 53 ... 57:
        buffer_set(MIN_FIVE);
        buffer_set(MINUTE);
        buffer_set(TO);
        break;
      }
    }
    else{
      switch(currentMinute){
      case 3 ... 7:
        buffer_set(MIN_FIVE);      
        buffer_set(MINUTE);
        buffer_set(PAST);
        break;
      case 8 ... 12:
        buffer_set(MIN_TEN);
        buffer_set(MINUTE);
        buffer_set(PAST);
        break;
      case 13 ...17:
        buffer_set(MIN_QUARTER);
        buffer_set(PAST);
        break;
      case 18 ... 22:
        buffer_set(MIN_TWENTY);
        buffer_set(MINUTE);
        buffer_set(PAST);
        break;
      case 23 ... 27:
        buffer_set(MIN_TWENTY);
        buffer_set(MIN_FIVE);
        buffer_set(MINUTE);
        buffer_set(PAST);
        break;
      case 28 ... 32:
        buffer_set(MIN_HALF);
        buffer_set(PAST);        
        break;
      }
    }
    //hour display
    buffer_set(OCLOCK);
    currentHour %= 12 ;
    switch(currentHour){
    case 1:
      buffer_set(ONE);
      break;
    case 2:
      buffer_set(TWO);
      break;
    case 3:
      buffer_set(THREE);
      break;
    case 4:
      buffer_set(FOUR);
      break;
    case 5:
      buffer_set(FIVE);
      break;
    case 6:
      buffer_set(SIX);
      break;
    case 7:
      buffer_set(SEVEN);
      break;
    case 8:
      buffer_set(EIGHT);
      break;
    case 9:
      buffer_set(NINE);
      break;
    case 10:
      buffer_set(TEN);
      break;
    case 11:
      buffer_set(ELEVEN);
      break;
    case 0:
      buffer_set(TWELVE);
      break;
    }
    buffer_set(IT_IS);
    buffer_set(OCLOCK);
    send_to_clock();  
  }
  lastMinute = currentMinute;
}

void processSyncMessage() {
 //if time sync available from serial port, update time and return true
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of a header and ten ascii digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      setTime(pctime);   // Sync Arduino clock to the time received on the serial port
      RTC.set(now());
    }  
  }
}

