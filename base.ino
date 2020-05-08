#include <SPI.h>
#include "RF24.h"
#include <TinyGPS++.h>


TinyGPSPlus gps;
RF24 radio(7,8);
byte addresses[][6] = {"1Node","2Node"};

struct GPS_coords {
  float latitude;
  float longitude;
};

GPS_coords mobile_coords;
GPS_coords base_coords;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  radio.begin();
  //radio.setPALevel(RF24_PA_LOW); // for testing
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  radio.startListening();
}

void loop() {
    while (Serial1.available() > 0) gps.encode(Serial1.read());
    
    if( radio.available()){
      while (radio.available()) {
        radio.read( &mobile_coords, sizeof(GPS_coords) );
      }
     
      radio.stopListening();
      radio.write( &base_coords, sizeof(GPS_coords) ); 
      radio.startListening();
      Serial.print("Mobile coords: ");
      Serial.print(mobile_coords.latitude, 6);
      Serial.print(", ");
      Serial.println(mobile_coords.longitude, 6);

        if (gps.location.isUpdated())
  {
    float flat = gps.location.lat();
    float flon = gps.location.lng();
    base_coords.latitude = gps.location.lat();
    base_coords.longitude = gps.location.lng();
    Serial.print("Base coords: ");
    Serial.print(base_coords.latitude, 6);
    Serial.print(", ");
    Serial.println(base_coords.longitude, 6);
  }
    
   }
}
