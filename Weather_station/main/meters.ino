#ifndef METERS

//Air quality
#include "PMS.h"
//Temperature pressure and humidity
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <Wire.h>

#if HEATER
  #include <OneWire.h>
  #include <DallasTemperature.h>
#endif

//the compass for the wind direction 
#if ENCODERWIND
  #include "AS5600.h"
#else
  #include <MechaQMC5883.h>
#endif

#define GUSTCOUNT 10
#define NONZEROWIDNCOUNT 400
#define ANGLECOUNT 400

#if ENCODERWIND
  #define DIRANGLEOFSSET 220 // angle value at 0

#else 
  //compass calibration
  #define DIRXOFFSET -107.5
  #define DIRYOFFSET -97.5
  #define DIRXSCALE 1420.0
  #define DIRYSCALE 1675.0
  #define DIRANGLEOFSSET -167
#endif


//Wind speed calibration
#define SPEEDOFFSET 50.5
#define SPEEDRATIO 10.0
#define MINWINDSPEED 5.2
#define TICKSPERREVOLUTION 4

//Rain meter Calibration 
#define MMPEROVERFLOW 0.3125


struct WindSpeedMeter{
  int pin;
  int ticksPerRevolution;
  unsigned long lastReadUS;
  unsigned long lastRevolutionFinishUS;
  unsigned long ticks;
  unsigned long revolutions;
  unsigned long measureingSinceMS;
  double gusts[GUSTCOUNT];
  double nonZeroWind[NONZEROWIDNCOUNT];
  size_t nonZeroReads;
};

struct WindDirMeter{
  double angles[ANGLECOUNT];
  size_t currentReads;
};

struct RainMeter{
  int pin;
  double mmPerOverfolw;
  unsigned int overflows;
  unsigned long lastReadUS;
  unsigned long measureingSinceMS;
};

struct WindMeterData{
  double averageSpeed;
  double averageGust;
};

struct WindDirData{
  double angle;
  double standardDiv;
};

WindSpeedMeter windSpeedMeter = {WINDMETERPIN, TICKSPERREVOLUTION};
WindDirMeter windDirMeter = {};
RainMeter rainMeter = {RAINMETERPIN, MMPEROVERFLOW};

Adafruit_BME280 bme;
bool bmeError = false;
#if ENCODERWIND
  AS5600 as5600; 
#else
  MechaQMC5883 qmc;
#endif
PMS pms(Serial2);
PMS::DATA pmsData;

#if HEATER
OneWire oneWire(RAIN_TEMP_PIN);
DallasTemperature sensors(&oneWire);

int numberOfDevices;
DeviceAddress tempDeviceAddress; 
#endif

bool overflow = false;
volatile bool readData = false;
volatile bool readWindDir = false;



void clearArray(double* arr, size_t len){
  for ( size_t i = 0; i<len;i++){
    arr[i] = 0;
  }
}

double averageArray(double* arr, size_t len, bool countZeors = false){
  double sum = 0;
  int zeros = 0;
  for (size_t i =0; i<len; i++){
    if(arr[i] ==0.0){
      zeros++;
    }
    sum += arr[i];
  }

  if(countZeors){

    return sum/len;
  }
  if(zeros == len){
    return 0;
  }

  return sum/(len -zeros);
  
}

double RpmToKmps (double rpm){
  double speed = (rpm + SPEEDOFFSET)/SPEEDRATIO;
  if ( speed >= MINWINDSPEED){
    return speed;
  }
  return 0;
}

WindMeterData readWindSpeed(WindSpeedMeter* meter, int reset = 2){
  double gustSpeed = RpmToKmps(averageArray(meter->gusts, GUSTCOUNT));
  double period = (millis() - meter->measureingSinceMS)/MILISPERSECOND;
  double currWindSpeed = RpmToKmps(60*meter->revolutions/period);
  if (currWindSpeed > 0){
    meter->nonZeroWind[meter->nonZeroReads] = currWindSpeed;
    meter->nonZeroReads++;
  }
  double windSpeed = averageArray(meter->nonZeroWind, NONZEROWIDNCOUNT);
  if(reset){
    meter->revolutions = 0;
    meter->measureingSinceMS = millis();
    if (reset > 1){
      clearArray(meter->gusts, GUSTCOUNT);
      clearArray(meter->nonZeroWind, NONZEROWIDNCOUNT);
    }
  }
  return {windSpeed, gustSpeed};
}


double readRain(RainMeter* meter, bool reset = true, bool normalized = false){

  double mmOfliquid = meter->overflows*meter->mmPerOverfolw;
  if(reset){
    meter->overflows = 0;
    meter->measureingSinceMS = millis();
  }
  if(normalized){
    double measureingForH = (micros() - meter->measureingSinceMS)/MICROSPERSECOND/SECONDSPERHOUR;
    return mmOfliquid/measureingForH;
  }
  return mmOfliquid;
}



WindDirData readDirection(WindDirMeter* meter, bool reset = true){
  double sinSum = 0.0;
  double cosSum = 0.0;
  
  for(size_t i =0;i<meter->currentReads;i++){
    sinSum += sin(meter->angles[i]*PI/180);
    cosSum += cos(meter->angles[i]*PI/180);
  }
  sinSum /= meter->currentReads;
  cosSum /= meter->currentReads;
  double angle = atan2(sinSum,cosSum)*180/PI;
  double R = sinSum*sinSum + cosSum*cosSum;
  double stdDiv = sqrt(-log(R));
  stdDiv *= 180/PI;
  if(reset){
    clearArray(meter->angles, ANGLECOUNT);
    meter->currentReads = 0;
  }

  if(angle<0){
    angle +=360;
  }
  return{angle, stdDiv};


}


void IRAM_ATTR PoolOverflowInterrupt() {
  if(micros() - rainMeter.lastReadUS <= DEBOUNCEPERIODRAIN){
    //this interrupt is called by reed switch bouce
    rainMeter.lastReadUS = micros();
    return;
  }
  rainMeter.lastReadUS = micros();
  rainMeter.overflows++;
  overflow = true;
}



void IRAM_ATTR WindSpeedInterupt() {
  if(micros() - windSpeedMeter.lastReadUS <= DEBOUNCEPERIOD){
    //this interrupt is called by reed switch bouce
    windSpeedMeter.lastReadUS = micros();
    return;
  }
   windSpeedMeter.ticks ++;
  if(windSpeedMeter.ticks % windSpeedMeter.ticksPerRevolution == 0){
    double periodS = (micros() - windSpeedMeter.lastRevolutionFinishUS)/MICROSPERSECOND;
    windSpeedMeter.lastRevolutionFinishUS = micros();

    double rpm = 60/periodS;
    size_t placeInTheQ = 0;
    while(placeInTheQ < GUSTCOUNT && rpm < windSpeedMeter.gusts[placeInTheQ]){
      placeInTheQ++;
    }
    double swaper = rpm;
    for( size_t i = placeInTheQ; i < GUSTCOUNT; i++){
      double temp = windSpeedMeter.gusts[i];
      windSpeedMeter.gusts[i] = swaper;
      swaper = temp;
    }
    windSpeedMeter.revolutions ++;
    windSpeedMeter.lastReadUS = micros();
  }
}

void IRAM_ATTR WindDirectionInterupt(){
  readWindDir = true;
}


double getAparantTemp(double temp, double humidity, double wind){
  double vapourPressure = humidity*6.105*exp(17.27*temp/(237.7 + temp))/100;
  return temp + 0.33*vapourPressure-0.7*wind/3.6 -4.0;
}



double getAngle(double x, double y){
  double angle = atan2(x,y)*180/PI;
  angle -= 180 + DIRANGLEOFSSET;
  while(angle<0){
    angle +=360;
  }
  return angle;
}

double offsetAngle(double x){
  double angle = x - DIRANGLEOFSSET;
   while(angle<0){
    angle +=360;
  }
  return angle;
}

#if HEATER
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}
#endif

#define METERS
#endif