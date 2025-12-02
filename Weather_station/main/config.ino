#ifndef CONFIG
// Depending on your weather station set values here correctly

// set to 1 if you are using encoder based wind direction meter
// set to 0 if you are using magnetometer based wind direction meter
#define ENCODERWIND 1

#define HEATER 1

// debug mode - faster readings, outputs to Serial
#define DEBUG 1

//Pins 27 new, 26 old
#define RAINMETERPIN 27
#define WINDMETERPIN 25

#if HEATER
  #define RAIN_TEMP_PIN 14
  #define HEATER_PIN 12

  #define TARGET_T 2
  #define HIST 0.2
#endif 



#if DEBUG 
  #define WINDDIRMEASUREMENTPERIODUS 100000 //us
  #define SENDPERIOD 20000000 // us
  #define WAITFORSTABLEDATA 5000 //ms
#else
  #define WINDDIRMEASUREMENTPERIODUS 10000000 //us
  #define SENDPERIOD 600000000  // us
  #define WAITFORSTABLEDATA 30000 //ms
#endif

#define DEBOUNCEPERIOD 10000
#define DEBOUNCEPERIODRAIN 100000
#define MICROSPERSECOND 1000000.0
#define MILISPERSECOND 1000.0
#define SECONDSPERHOUR 3600.0
#define PI 3.1415

IPAddress local_IP(192, 168, 1, 166);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);
IPAddress secondaryDNS(8, 8, 8, 8);

#define CONFIG
#endif