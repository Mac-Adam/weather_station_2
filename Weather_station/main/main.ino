#include "secrets.ino"
#include "config.ino"
#include "meters.ino"

#include <WiFi.h>
#include <HTTPClient.h>



hw_timer_t * timerDir = NULL; 
hw_timer_t * timerSend = NULL; 

unsigned long t = 0;
unsigned long t2 = 0;
int interupts = 0;



void connetcToWiFi(){
  
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
#if DEBUG
    Serial.println("STA Failed to configure");
#endif
  }
#if DEBUG
  Serial.print("Connecting to: ");
  Serial.println(WIFINAME);
#endif
  WiFi.begin(WIFINAME, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
#if DEBUG
    Serial.println("connceting to WiFi");
#endif
    delay(1000);
  }
#if DEBUG
  Serial.println("Connected to wifi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}


void IRAM_ATTR SendDataInterupt(){
  readData = true;
}


void setup() {
  Wire.begin();
#if DEBUG
  Serial.begin(115200); // for testing
#endif
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // for comunication wint pms 5003

  Wire.begin();

  connetcToWiFi();

  pms.passiveMode();

  if(!bme.begin(0x76)){
#if DEBUG
    Serial.println("Failed to find BME280 sensor");
#endif
    bmeError = true;
  }
#if HEATER
  sensors.begin();
  numberOfDevices = sensors.getDeviceCount();
#if DEBUG
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
#endif
  pinMode(HEATER_PIN,OUTPUT);
  digitalWrite(HEATER_PIN,HIGH);
#endif


  pinMode(rainMeter.pin, INPUT_PULLUP);
  attachInterrupt(rainMeter.pin, PoolOverflowInterrupt, CHANGE);

#if !ENCODERWIND
  qmc.init();
  qmc.setMode(Mode_Continuous,ODR_200Hz,RNG_8G,OSR_256);
#endif

  pinMode(windSpeedMeter.pin, INPUT_PULLUP);
  attachInterrupt(windSpeedMeter.pin, WindSpeedInterupt, CHANGE);

  // periodic timer that will evoke the WindDirectionInterupt function every WINDDIRMEASUREMENTPERIODUS us
  timerDir = timerBegin(1000000);           	
  timerAttachInterrupt(timerDir, WindDirectionInterupt);
  timerAlarm(timerDir, WINDDIRMEASUREMENTPERIODUS, true,0);  		

  timerSend = timerBegin(1000000);           	
  timerAttachInterrupt(timerSend, SendDataInterupt);
  timerAlarm(timerSend, SENDPERIOD, true,0);  		

}

void loop() {

  if(readWindDir){
#if DEBUG
    Serial.println("Readning Wind");
#endif
#if HEATER
    sensors.requestTemperatures(); 
    float avg = 0;
    for(int i=0;i<numberOfDevices; i++){
      if(sensors.getAddress(tempDeviceAddress, i)){

        float tempC = sensors.getTempC(tempDeviceAddress);
        avg += tempC/numberOfDevices;
      }
    }
#if DEBUG
    Serial.print("Avreage reain meter temp: ");
    Serial.println(avg);
#endif
    if (avg > TARGET_T + HIST){
      digitalWrite(HEATER_PIN,LOW);
#if DEBUG
      Serial.println("Heating off");
#endif
    }else if (avg < TARGET_T - HIST){
      digitalWrite(HEATER_PIN,HIGH);
#if DEBUG
      Serial.println("Heating on");
#endif
    }
#endif
    readWindDir = false;
#if ENCODERWIND
    windDirMeter.angles[windDirMeter.currentReads] = offsetAngle(as5600.rawAngle() * AS5600_RAW_TO_DEGREES);
    windDirMeter.currentReads ++;
#else
    int x,y,z,a;
    qmc.read(&x,&y,&z,&a);
    double xNormalized = (x + DIRXOFFSET)/DIRXSCALE;
    double yNormalized = (y + DIRYOFFSET)/DIRYSCALE;
      
    windDirMeter.angles[windDirMeter.currentReads] = getAngle(xNormalized, yNormalized);
    windDirMeter.currentReads ++;

#endif
    readWindSpeed(&windSpeedMeter,1);

  }
  if(readData)
  {
    
#if DEBUG
    Serial.println("Preparing to read data");
#endif

    WindMeterData windSpeed = readWindSpeed(&windSpeedMeter);
    WindDirData windDir = readDirection(&windDirMeter);
    double rain = readRain(&rainMeter);
    pms.wakeUp();
    delay(WAITFORSTABLEDATA);
#if DEBUG
    Serial.println("Reading data");
#endif

    //detachInterrupt(digitalPinToInterrupt(rainMeter.pin));
    //detachInterrupt(digitalPinToInterrupt(windSpeedMeter.pin));
    pms.requestRead();

    float temp = bme.readTemperature();
    float pressure = bme.readPressure();
    float humidity = bme.readHumidity();
    float aparantTemp = getAparantTemp(temp, humidity, windSpeed.averageSpeed);
    if(pms.readUntil(pmsData)){
#if DEBUG
      Serial.print("wind speed: ");
      Serial.println(windSpeed.averageSpeed);
      Serial.print("gusts speed: ");
      Serial.println(windSpeed.averageGust);
      Serial.print("Wind dir: ");
      Serial.println(windDir.angle);
      Serial.print("wind dir std div: ");
      Serial.println(windDir.standardDiv);
      Serial.print("Rain: ");
      Serial.println(rain);
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
#endif
    }else{
#if DEBUG
      Serial.println("Some pms error has ocured");
#endif
    }

    //attachInterrupt(rainMeter.pin, PoolOverflowInterrupt, CHANGE);
    //attachInterrupt(windSpeedMeter.pin, WindSpeedInterupt, CHANGE);

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
    request += String(windSpeed.averageSpeed);
    request += "&predkosc_porywow=";
    request += String(windSpeed.averageGust);
    request += "&kierunek_wiatru=";
    request += String(windDir.angle);
    request += "&std_div_kierunku=";
    request += String(windDir.standardDiv);
    request += "&opad=";
    request += String(rain);

    int httpResponseCode = http.POST(request);
    if (httpResponseCode > 0)
    {

      String response = http.getString();

#if DEBUG
      Serial.println(response);

      Serial.println("seems ok");
#endif
    }
    else {
#if DEBUG
      Serial.println("something went wrong");
#endif
    }
#if DEBUG
    Serial.println(httpResponseCode);
#endif
    http.end();
    


    pms.sleep();
    readData = false;

    

  }

}
