// This program tests the rain meter heater

#include <OneWire.h>
#include <DallasTemperature.h>

#define RAIN_TEMP_PIN 14
#define HEATER_PIN 12

#define TARGET_T 34
#define HIST 0.2

OneWire oneWire(RAIN_TEMP_PIN);
DallasTemperature sensors(&oneWire);

int numberOfDevices;
DeviceAddress tempDeviceAddress; 

void setup(){

  Serial.begin(9600);
  
  sensors.begin();
  
  numberOfDevices = sensors.getDeviceCount();
  
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  for(int i=0;i<numberOfDevices; i++){
    if(sensors.getAddress(tempDeviceAddress, i)){
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }

  pinMode(HEATER_PIN,OUTPUT);
  digitalWrite(HEATER_PIN,HIGH);

}

void loop(){ 
  sensors.requestTemperatures(); 
  float avg = 0;
  for(int i=0;i<numberOfDevices; i++){
    if(sensors.getAddress(tempDeviceAddress, i)){

      float tempC = sensors.getTempC(tempDeviceAddress);
      avg += tempC/numberOfDevices;
    }
  }
  Serial.println(avg);
  if (avg > TARGET_T + HIST){
    digitalWrite(HEATER_PIN,LOW);
  }else if (avg < TARGET_T - HIST){
    digitalWrite(HEATER_PIN,HIGH);
  }
  delay(500);
}

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}