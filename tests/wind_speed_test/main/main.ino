// Test for the wind speed meter


#define GUSTCOUNT 10

struct WindMeterData{
  double averageRMP;
  double gustRMP;
  double latestRPM;
};

struct WindSpeedMeter{
  int pin;
  int ticksPerRevolution;
  unsigned long usLastRead;
  unsigned long debounceingPeriod;
  unsigned long usLastRevolutionFinish;
  unsigned long ticks;
  unsigned long revolutions;
  double gusts[GUSTCOUNT];
  double runningAverage;
  double latest;
};

int mesNum =0;
unsigned long micros1;

int i =0;
WindSpeedMeter windMeter = {25,4,micros(),10000,micros()};

double averageArray(double* arr, size_t len, bool countZeors = false){
  double res = 0;
  int zeros = 0;
  for (size_t i =0; i<len; i++){
    if(arr[i] ==0.0){
      zeros++;
    }
    res += arr[i];
  }
  if(countZeors){
    return res/len;
  }
  if(zeros == len){
    return 0;
  }
  return res/(len -zeros);
  
}

void clearArray(double* arr, size_t len){
  for ( size_t i = 0; i<len;i++){
    arr[i] = 0;
  }
}

void printArray(double* arr, size_t len){
   for ( size_t i = 0; i<len;i++){
    Serial.printf("%lf \n",arr[i]);
  }
}

WindMeterData readWindSpeed(WindSpeedMeter* meter, bool reset = true){
  WindMeterData data = {meter->runningAverage, averageArray(meter->gusts,GUSTCOUNT), meter->latest};
  if(reset){
    meter->runningAverage = 0;
    meter->revolutions = 0;
    clearArray(meter->gusts, GUSTCOUNT);
  }
  return data;
}

void IRAM_ATTR WindSpeedInterupt() {
  unsigned long timeSinceLastRead = micros() - windMeter.usLastRead;
  if( timeSinceLastRead <= windMeter.debounceingPeriod){
    return;
  }
  windMeter.ticks ++;
  if(windMeter.ticks % windMeter.ticksPerRevolution == 0){
    
    double sPeriod = (micros() - windMeter.usLastRevolutionFinish)/1000000.0;
    windMeter.usLastRevolutionFinish = micros();

    double rpm = 60/sPeriod;
    windMeter.runningAverage = ((windMeter.revolutions*windMeter.runningAverage) + rpm) / (windMeter.revolutions+1);
    windMeter.latest = rpm;
    int placeInTheQ = 0;
    while (placeInTheQ < GUSTCOUNT && rpm < windMeter.gusts[placeInTheQ]  ){
      placeInTheQ++;
  
    }
    double swaper = rpm;
    for ( size_t i = placeInTheQ;i < GUSTCOUNT;i ++){
        double temp = windMeter.gusts[i];
        windMeter.gusts[i] = swaper;
        swaper = temp;
         
    }

    windMeter.revolutions ++;
  }
  windMeter.usLastRead = micros();
}

void setup() {
  Serial.begin(115200);
  pinMode(windMeter.pin, INPUT_PULLUP);
  attachInterrupt(windMeter.pin, WindSpeedInterupt, CHANGE);
  micros1 = micros();
}

void loop() {

 if (micros() - micros1 >= 1000000)
 {
   micros1 = micros();
   WindMeterData data = readWindSpeed(&windMeter, false);
   Serial.printf("%i,%lf,%lf,%lf\n",mesNum,data.averageRMP, data.gustRMP, data.latestRPM);
   mesNum++;
 }
}
