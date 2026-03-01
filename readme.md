## 3D Printable weather station

This is the repository for my personal project, a weather station.
It is based on ESP32. You can view the source code in the weather station folder.
All the 3d models are available in the 3d models folder.
Folder old has the program for an older version of the design.
Folder tests has a few programs created to calibrate or test individual components.
The stations has following functionality:

- Temperature, humidity and pressure sensor
- Air quality sensor
- Custom wind speed meter
- Custom wind direction meter
- Custom rain meter with heating to measure snow

## Building And Running

Open the Weather_station project in arudino studio and program your ESP32.
Before programing, create a secrets.ino file:

```

#ifndef SECRETS

#define WIFINAME "Name of your WiFi"
#define PASSWORD "Your WiFi Password"
#define ID Unique ID
#define SERVERNAME "Url for your server"
#else

#define SECRETS
#endif
```
