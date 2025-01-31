// I used this code to test the PMS sensor
// ON ESP32 there is a bug, that you need to initialize Serial2 with some pins eaven thought it should be be initalized by default

#include "PMS.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define ONLYPMS 0

#define SLEEP 5000
#define WAIT 10000

#define SEALEVELPRESSURE_HPA (1013.25)
#if !ONLYPMS
Adafruit_BME280 bme;
#endif
PMS pms(Serial2);
PMS::DATA data;

void setup()
{

  Serial.begin(115200);   
  Serial2.begin(9600, SERIAL_8N1, 16, 17); 
  pms.passiveMode();  
#if !ONLYPMS  
  bool status;
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
#endif
  Serial.println("-- Default Test --");


  Serial.println();
}

void loop()
{
  Serial.println("Waking up, waiting for stable readings...");
  pms.wakeUp();
  delay(WAIT);

  Serial.println("Send read request...");
  pms.requestRead();

  Serial.println("Reading data...");

  Serial.println();
  if (pms.readUntil(data))
  {
    Serial.print("PM 1.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM 10.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_10_0);
    
#if !ONLYPMS
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    
    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
#endif

    Serial.println();
  }
  else
  {
    Serial.println("No data.");
  }

  Serial.println("Going to sleep");
  pms.sleep();
  delay(SLEEP);
}