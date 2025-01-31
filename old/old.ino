// THIS CODE IS OLD IT IS FOR THE FIRST VERSION OF THE WEATHER STATION

// libraries I need
//I2C
#include <Wire.h>
//Air quality
#include "PMS.h"
//Temperature pressure and humidity
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include <WiFi.h>
#include <HTTPClient.h>


#include "DHT.h"

#define DHTPIN 4    

#define DHTTYPE DHT22

#define SENDPERIOD 600000000
//#define SENDPERIOD 10000000


#define SSID "SSID"
#define PASSWORD "PASS"
#define ID 1234
#define SERVERNAME "SERVERNAME"

IPAddress local_IP(192, 168, 1, 166);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);
IPAddress secondaryDNS(8, 8, 8, 8);



Adafruit_BMP280 bmp;
bool bmeError = false;

PMS pms(Serial2);
PMS::DATA pmsData;

DHT dht(DHTPIN, DHTTYPE);


hw_timer_t * timerSend = NULL; 

unsigned long t = 0;
unsigned long t2 = 0;
int interupts = 0;

volatile bool readData = false;


bool overflow = false;


void connetcToWiFi(){
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  Serial.print("Connecting to: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("connceting to WiFi");
    delay(1000);
  }

  Serial.println("Connected to wifi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}




void IRAM_ATTR SendDataInterupt(){
  readData = true;
}


double getAparantTemp(double temp, double humidity, double wind){
  double vapourPressure = humidity*6.105*exp(17.27*temp/(237.7 + temp))/100;
  return temp + 0.33*vapourPressure-0.7*wind/3.6 -4.0;
}



void setup() {
  Wire.begin();

  Serial.begin(9600); // for testing
  Serial2.begin(9600); // for comunication wint pms 5003

  Wire.begin();

  connetcToWiFi();

  pms.passiveMode();

  if(!bmp.begin(0x76)){
    Serial.println("Failed to find BMP280 sensor");
    bmeError = true;
  }
  dht.begin();

  timerSend = timerBegin(1, 80, true);           	
  timerAttachInterrupt(timerSend, SendDataInterupt, true);
  timerAlarmWrite(timerSend, SENDPERIOD, true);  		
  timerAlarmEnable(timerSend);

}

void loop() {

  if(readData)
  {
    
    pms.wakeUp();
    delay(30000);
    pms.requestRead();
    float temp = dht.readTemperature();
    float pressure = bmp.readPressure();
    float humidity = dht.readHumidity();
    float aparantTemp = getAparantTemp(temp, humidity,0);
    if(pms.readUntil(pmsData)){
      Serial.print("wind speed: ");
      Serial.println(0);
      Serial.print("gusts speed: ");
      Serial.println(0);
      Serial.print("Wind dir: ");
      Serial.println(0);
      Serial.print("wind dir std div: ");
      Serial.println(0);
      Serial.print("Rain: ");
      Serial.println(0);
      Serial.print("PM 1.0 (ug/m3): ");
      Serial.println(pmsData.PM_AE_UG_1_0);
      Serial.print("PM 2.5 (ug/m3): ");
      Serial.println(pmsData.PM_AE_UG_2_5);
      Serial.print("PM 10.0 (ug/m3): ");
      Serial.println(pmsData.PM_AE_UG_10_0);
      Serial.print("Temperature: ");
      Serial.println(temp);
      Serial.print("Aparant Temperature: ");
      Serial.println(aparantTemp);
      Serial.print("Pressure: ");
      Serial.println(pressure / 100.0F);
      Serial.print("Humidity = ");
      Serial.println(humidity);
      Serial.println("pms read succesfull");
    }else{
      Serial.println("Some pms error has ocured");
    }

    if (WiFi.status() != WL_CONNECTED) {
      connetcToWiFi();
    }
    HTTPClient http;
    http.begin(SERVERNAME);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String request = "";
    request += "wilgotnosc=";
    request += String(humidity);
    request += "&temperatura=";
    request += String(temp);
    request += "&temp_odczu=";
    request += String(aparantTemp);
    request += "&pm_ae_1=";
    request += String(pmsData.PM_AE_UG_1_0);
    request += "&pm_ae_2_5=";
    request += String(pmsData.PM_AE_UG_2_5);
    request += "&pm_ae_10=";
    request += String(pmsData.PM_AE_UG_10_0);
    request += "&pm_sp_1=";
    request += String(pmsData.PM_SP_UG_1_0);
    request += "&pm_sp_2_5=";
    request += String(pmsData.PM_SP_UG_2_5);
    request += "&pm_sp_10=";
    request += String(pmsData.PM_SP_UG_10_0);
    request += "&id_stacji=";
    request += String(ID);
    request += "&cisnienie=";
    request += String(pressure / 100.0F);
    request += "&predkosc_wiatru=";
    request += String(0);
    request += "&predkosc_porywow=";
    request += String(0);
    request += "&kierunek_wiatru=";
    request += String(0);
    request += "&std_div_kierunku=";
    request += String(0);
    request += "&opad=";
    request += String(0);
    Serial.println(request);
    int httpResponseCode = http.POST(request);
    if (httpResponseCode > 0)
    {

      String response = http.getString();


      Serial.println(response);

      Serial.println("seems ok");
    }
    else {
      Serial.println("something went wrong");
    }
    Serial.println(httpResponseCode);
    http.end();
    
    pms.sleep();
    readData = false;
  }

}
