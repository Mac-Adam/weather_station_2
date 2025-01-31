// Test for the old wind direction meter (with magnetometer)

#include <Wire.h>
#include <MechaQMC5883.h>
//this library can be downoaded from https://github.com/keepworking/Mecha_QMC5883L

#define XOFFSET 107.5
#define YOFFSET 97.5
#define XSCALE 1420.0
#define YSCALE 1675.0
#define ANGLEOFSSET 13

MechaQMC5883 qmc;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  qmc.init();
  qmc.setMode(Mode_Continuous,ODR_200Hz,RNG_8G,OSR_256);
}

double getAngle(double x, double y){
  double angle = atan2(x,y)*180/3.14;
  angle -= 180 + ANGLEOFSSET;
  while(angle<0){
    angle +=360;
  }
  return angle;
}

void loop() {
  int x,y,z,a;
  qmc.read(&x,&y,&z,&a);
  if(x > 10000 || y > 10000 || z > 10000){
    Serial.println("not valid");
  }
  else{
    double xNormalized = (x - XOFFSET)/XSCALE;
    double yNormalized = (y-YOFFSET)/YSCALE;

    //Serial.print("x:");
    Serial.print(x);
    Serial.print(",");
    //Serial.print(",y:");
    Serial.print(y);
    Serial.print(",");
    //Serial.print(",z:");
    Serial.print(z);
    Serial.print(",a:");
    Serial.print(getAngle(xNormalized, yNormalized));
    Serial.println();
  }
  delay(50);
}