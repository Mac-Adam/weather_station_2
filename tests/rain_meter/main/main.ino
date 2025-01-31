// Thest the rain meter

#define RAINMETERPIN 26
bool overflowed  =false;

void IRAM_ATTR PoolOverflowInterrupt() {
 overflowed = true;
}


void setup() {
  Serial.begin(115200);
  pinMode(RAINMETERPIN, INPUT_PULLUP);
  attachInterrupt(RAINMETERPIN, PoolOverflowInterrupt, CHANGE);
  Serial.println("Starting Rain Meter Test");
}

void loop() {
  if(overflowed){
    Serial.println("Overflowed");
    overflowed = false;
  }

}
