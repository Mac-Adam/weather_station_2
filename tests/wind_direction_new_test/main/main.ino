// Code for testing the new wind direction meter (with encoder)

#include "AS5600.h"


AS5600 as5600; 


void setup()
{
  Serial.begin(115200);
  Wire.begin();

  as5600.begin();// not using the direction pin
  if (as5600.isConnected()){
    Serial.print("Connected");
  }
  else{
    Serial.print("couldn't connect");
  }
}


void loop()
{
  Serial.print(as5600.readAngle());
  Serial.print("\t");
  Serial.println(as5600.rawAngle() * AS5600_RAW_TO_DEGREES);

  delay(1000);
}
