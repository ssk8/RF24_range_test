#include <SPI.h>
#include "RF24.h"
#include <TinyGPS++.h>
#include <U8g2lib.h>
#include <Wire.h>

RF24 radio(7,8);
TinyGPSPlus gps;
byte addresses[][6] = {"1Node","2Node"};

struct GPS_coords {
  float latitude;
  float longitude;
};

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

GPS_coords mobile_coords;
GPS_coords base_coords;
bool comms;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  u8g2.begin();
  radio.begin();
  //radio.setPALevel(RF24_PA_LOW); // for testing
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();
}


void loop() {
    
    if (gps.location.isUpdated())
    {
      mobile_coords.latitude = gps.location.lat();
      mobile_coords.longitude = gps.location.lng();

      double distanceToBase =
        TinyGPSPlus::distanceBetween(
          mobile_coords.latitude,
          mobile_coords.longitude,
          base_coords.latitude, 
          base_coords.longitude);
      double courseToBase =
        TinyGPSPlus::courseTo(
          mobile_coords.latitude,
          mobile_coords.longitude,
          base_coords.latitude, 
          base_coords.longitude);
      
      u8g2.setFont(u8g2_font_helvB14_tf);
      u8g2.firstPage();
      do {
        u8g2.setCursor(0, 20);
        u8g2.print(distanceToBase);
        u8g2.print(F(" m "));
        u8g2.print(TinyGPSPlus::cardinal(courseToBase));
        u8g2.setCursor(0, 40);
        if(comms){
          u8g2.print(F("comms up"));
        }
        else{
          u8g2.print(F("comms down"));
        }
        u8g2.setCursor(0, 60);
        u8g2.print(gps.speed.mps());
        u8g2.print(F(" m/s "));
        u8g2.print(TinyGPSPlus::cardinal(gps.course.deg()));
          } while ( u8g2.nextPage() );
    }

    radio.stopListening();
    Serial.print(F("Now sending: "));
    Serial.print(mobile_coords.latitude, 6);
    Serial.print(", ");
    Serial.println(mobile_coords.longitude, 6);
    if (!radio.write( &mobile_coords, sizeof(GPS_coords) )){
       Serial.println(F("failed"));
     }
        
    radio.startListening();
    
    unsigned long started_waiting_at = micros();
    boolean timeout = false;
    while ( ! radio.available() ){
      if (micros() - started_waiting_at > 200000 ){
          timeout = true;
          break;
      }      
    }
        
    if ( timeout ){
        Serial.println(F("Failed, response timed out."));
        digitalWrite(LED_BUILTIN, LOW);
        comms = false;
    }else{
        radio.read( &base_coords, sizeof(GPS_coords));
        digitalWrite(LED_BUILTIN, HIGH);
        comms = true;
        Serial.print(F("Got response "));
        Serial.print(base_coords.latitude, 6);
        Serial.print(", ");
        Serial.print(base_coords.longitude, 6);
        Serial.println();
        
    }
    smartDelay(1000);
  }




static void smartDelay(unsigned long ms)
  {
    unsigned long start = millis();
    do 
    {
      while (Serial1.available())
        gps.encode(Serial1.read());
    } while (millis() - start < ms);
  }
