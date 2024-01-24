/* 
  Sketch generated by the Arduino IoT Cloud Thing "Untitled"
  https://create.arduino.cc/cloud/things/79cc410a-1cc0-4110-9fb6-12f98feafbf3 

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  float tempCelsius;
  bool bulbControl;
  bool fanControl;
  bool showTemperature;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include "thingProperties.h"
#include "DHT.h"

#define DHTPIN 2     // DHT humidity/temperature sensors are connected to pin2

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor.

#define TEMPERATURE_THRESHOLD_1 28 // max temperature for turning on the fan
#define TEMPERATURE_THRESHOLD_2 30 // max temperature for turning on the fan
#define fan_pin 11 // Define fan on pin 11
//int fanSpeed; // initialize fan speed
#define led_pin 13 // Define led on pin 13

int calibrationTime = 45; //the time we give the sensor to calibrate
long unsigned int lowIn; //the time when the sensor outputs a low impulse
long unsigned int pause = 5000; //the amount of milliseconds the sensor has to be low before we assume all motion has stopped
boolean lockLow = true;
boolean takeLowTime;
#define motionSensorPin 3 //the digital pin connected to the Motion sensor's output
int counter1 = 1, counter2 = 0;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(5000);
  Serial.println("============================================================");
  Serial.println("New test!");

  pinMode(motionSensorPin, INPUT);
  digitalWrite(motionSensorPin, LOW);

  //give the sensor some time to calibrate
  Serial.print("Calibrating motion sensor ");
  for(int i = 0; i < calibrationTime; i++){
    Serial.print(".");
    delay(1000);
    }
  Serial.println(" done");
  Serial.println("MOTION SENSOR ACTIVE");
  delay(50);

  pinMode(fan_pin, OUTPUT);
  digitalWrite(fan_pin, HIGH);
  pinMode(led_pin, OUTPUT);

  dht.begin();
  delay(2000);
  float t = dht.readTemperature(); // Read temperature as Celsius (the default)
  tempCelsius = t; // store the temperature in a global variable
  float h = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  // Your code here 
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Read temperature as Celsius (the default)
  tempCelsius = t; // store the temperature in a global variable
  float f = dht.readTemperature(true); // Read temperature as Fahrenheit (isFahrenheit = true)
  float hif = dht.computeHeatIndex(f, h); // Compute heat index in Fahrenheit (the default)
  float hic = dht.computeHeatIndex(t, h, false); // Compute heat index in Celsius (isFahreheit = false)


  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if(t < TEMPERATURE_THRESHOLD_1 && fanControl == false)
    digitalWrite(fan_pin, HIGH);

  if(t >= TEMPERATURE_THRESHOLD_1 && t < TEMPERATURE_THRESHOLD_2) {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");

    Serial.println("FAN turning ON!");
    digitalWrite(fan_pin, LOW);
    Serial.println("LED turned OFF!");
    digitalWrite(led_pin, LOW);

    delay(10000);

  }
  else if(t >= TEMPERATURE_THRESHOLD_2) {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");

    Serial.println("FAN turning ON!");
    digitalWrite(fan_pin, LOW);
    Serial.println("LED turning ON!");
    digitalWrite(led_pin, HIGH);

    delay(10000);
  }
  else{
    if(digitalRead(motionSensorPin) == HIGH) {
      counter2++;
      if(counter2 == 1) {
        Serial.println("LED turning ON!");
      }
      digitalWrite(led_pin, HIGH);   //the led visualizes the sensors output pin state
      if(lockLow) {  
        //makes sure we wait for a transition to LOW before any further output is made:
        lockLow = false;
        Serial.print("motion detected at ");
        Serial.print(millis()/1000);
        Serial.println(" sec"); 
        delay(50);
        }         
        takeLowTime = true;
        counter1 = 0;
    }

    if(digitalRead(motionSensorPin) == LOW && bulbControl == false) {       
      digitalWrite(led_pin, LOW);  //the led visualizes the sensors output pin state
      counter1++;
      if(counter1 == 1) {
        Serial.println("LED turned OFF!");
      }
      if(takeLowTime) {
        lowIn = millis();          //save the time of the transition from high to LOW
        takeLowTime = false;       //make sure this is only done at the start of a LOW phase
      }
      //if the sensor is low for more than the given pause, 
      //we assume that no more motion is going to happen
      if(!lockLow && millis() - lowIn > pause) {  
        //makes sure this block of code is only executed again after 
        //a new motion sequence has been detected
        lockLow = true;                        
        Serial.print("motion ended at ");      //output
        Serial.print((millis() - pause)/1000);
        Serial.println(" sec");
        delay(50);
      }
      counter2 = 0;
    }
  }
}


/*
  Since BulbControl is READ_WRITE variable, onBulbControlChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onBulbControlChange()  {
  // Add your code here to act upon BulbControl change
  if(bulbControl){
    digitalWrite(led_pin, HIGH);
    Serial.println("LED turning ON!");
  }
  else{
    digitalWrite(led_pin, LOW);
    Serial.println("LED turned OFF!");
  }
    
}


/*
  Since FanControl is READ_WRITE variable, onFanControlChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onFanControlChange()  {
  // Add your code here to act upon FanControl change
  if(fanControl){
    Serial.println("FAN turning ON!");
    digitalWrite(fan_pin, LOW);
  }
  else{
    Serial.println("FAN turned OFF!");
    digitalWrite(fan_pin, HIGH);
  }
}
void onShowTemperatureChange()  {
  // Add your code here to act upon ShowTemperature change
  if(showTemperature){
    // Wait a few seconds between measurements.
    delay(2000);
    float t = dht.readTemperature(); // Read temperature as Celsius (the default)
    tempCelsius = t; // store the temperature in a global variable
    float f = dht.readTemperature(true); // Read temperature as Fahrenheit (isFahrenheit = true)
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.println(" *F");
  }
}












/*
  Since TempCelsius is READ_WRITE variable, onTempCelsiusChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onTempCelsiusChange()  {
  // Add your code here to act upon TempCelsius change
}