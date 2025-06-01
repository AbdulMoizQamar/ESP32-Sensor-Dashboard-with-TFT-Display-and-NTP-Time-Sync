#include <WiFi.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#define TFT_MISO 37
#define TFT_MOSI 35
#define TFT_SCLK 36
#define TFT_CS   39  // TFT chip select
#define TFT_DC   2  // TFT data/command
#define TFT_RST  4  // TFT reset (-1 if not used)

const char* ssid     = "WIFI-moiz";
const char* password = "utflabs786@@";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 18000;
const int   daylightOffset_sec = 0;

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 1000;  

int buttonPin = 12;          
int menuIndex = 0;  

//Function Decalration
void showMenu();

TFT_eSPI tft = TFT_eSPI();  // Initialize TFT

// GPIO where the DS18B20 is connected to
const int oneWireBus = 7;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void displaySensorDetails(void)
{
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2"); 
  Serial.println("------------------------------------");
  Serial.println("");
  delay(1000);
}

void displayDataRate(void)
{
  Serial.print  ("Data Rate:    "); 
 
  switch(accel.getDataRate())
  {
    case ADXL345_DATARATE_3200_HZ:
      Serial.print  ("3200 "); 
      break;
    case ADXL345_DATARATE_1600_HZ:
      Serial.print  ("1600 "); 
      break;
    case ADXL345_DATARATE_800_HZ:
      Serial.print  ("800 "); 
      break;
    case ADXL345_DATARATE_400_HZ:
      Serial.print  ("400 "); 
      break;
    case ADXL345_DATARATE_200_HZ:
      Serial.print  ("200 "); 
      break;
    case ADXL345_DATARATE_100_HZ:
      Serial.print  ("100 "); 
      break;
    case ADXL345_DATARATE_50_HZ:
      Serial.print  ("50 "); 
      break;
    case ADXL345_DATARATE_25_HZ:
      Serial.print  ("25 "); 
      break;
    case ADXL345_DATARATE_12_5_HZ:
      Serial.print  ("12.5 "); 
      break;
    case ADXL345_DATARATE_6_25HZ:
      Serial.print  ("6.25 "); 
      break;
    case ADXL345_DATARATE_3_13_HZ:
      Serial.print  ("3.13 "); 
      break;
    case ADXL345_DATARATE_1_56_HZ:
      Serial.print  ("1.56 "); 
      break;
    case ADXL345_DATARATE_0_78_HZ:
      Serial.print  ("0.78 "); 
      break;
    case ADXL345_DATARATE_0_39_HZ:
      Serial.print  ("0.39 "); 
      break;
    case ADXL345_DATARATE_0_20_HZ:
      Serial.print  ("0.20 "); 
      break;
    case ADXL345_DATARATE_0_10_HZ:
      Serial.print  ("0.10 "); 
      break;
    default:
      Serial.print  ("???? "); 
      break;
  } 
  Serial.println(" Hz"); 
}

void displayRange(void)
{
  Serial.print ("Range:         +/- ");
 
  switch(accel.getRange())
  {
    case ADXL345_RANGE_16_G:
      Serial.print  ("16 "); 
      break;
    case ADXL345_RANGE_8_G:
      Serial.print  ("8 "); 
      break;
    case ADXL345_RANGE_4_G:
      Serial.print  ("4 "); 
      break;
    case ADXL345_RANGE_2_G:
      Serial.print  ("2 "); 
      break;
    default:
      Serial.print  ("?? "); 
      break;
  } 
  Serial.println(" g"); 
}

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);  
  startMillis = millis();  //initial start time

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);


  // Start the DS18B20 sensor
  sensors.begin();

  //#ifndef ESP8266
//  while (!Serial); // for Leonardo/Micro/Zero
//#endif
  Serial.begin(115200);
  Serial.println("Accelerometer Test"); Serial.println("");
 
  showMenu();

  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // accel.setRange(ADXL345_RANGE_8_G);
  // accel.setRange(ADXL345_RANGE_4_G);
  // accel.setRange(ADXL345_RANGE_2_G);
 
  /* Display some basic information on this sensor */
  displaySensorDetails();
 
  /* Display additional settings (outside the scope of sensor_t) */
  displayDataRate();
  displayRange();
  Serial.println("");

  tft.begin();
  tft.setRotation(1); // Adjust rotation (1 = landscape)
 // int downloadpercent = 50;
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(4);      // Set text size
  tft.setTextColor(TFT_BLUE,TFT_BLACK);  // White text on blue background
  //tft.setCursor(80, 70);   // Set cursor position (X=0, Y=10)

}

void loop() { 
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
 // tft.setCursor(0, 5);
  Serial.print(temperatureC);
  Serial.println("ºC");
  //tft.println("Temperature");
 // tft.printf("%.3f");
 // delay(1000);
    /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);
 
  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");
 // delay(1000);
//  tft.setCursor(0, 160);
//  tft.println("Acceleration");
 // tft.print("X: "); tft.print(event.acceleration.x); tft.print("  ");
 // tft.print("Y: "); tft.print(event.acceleration.y); tft.print("  ");
 // tft.print("Z: "); tft.print(event.acceleration.z); tft.print("  ");

  printLocalTime();

   currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
        if (digitalRead(buttonPin) == LOW) {  
    delay(300);                         
    menuIndex = menuIndex + 1;
    if (menuIndex > 2) {
      menuIndex = 0;
    }
   // showMenu();
    while (digitalRead(buttonPin) == LOW);
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
    //delay(100);
  }
  }
  showMenu();
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
 // tft.setCursor(0, 100);
 // tft.println("Current Time");
 // tft.println(&timeinfo, "%I:%M:%S");
 //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
 // Serial.print("Day of week: ");
 // Serial.println(&timeinfo, "%A");
  //Serial.print("Month: ");
  //Serial.println(&timeinfo, "%B");
  //Serial.print("Day of Month: ");
 // Serial.println(&timeinfo, "%d");
 // Serial.print("Year: ");
 // Serial.println(&timeinfo, "%Y");
 // Serial.print("Hour: ");
 // Serial.println(&timeinfo, "%H");
  Serial.print("Hour : ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

 // Serial.println("Time variables");
 // char timeHour[3];
 // strftime(timeHour,3, "%H", &timeinfo);
 // Serial.println(timeHour);
 // char timeWeekDay[10];
 // strftime(timeWeekDay,10, "%A", &timeinfo);
//  Serial.println(timeWeekDay);
 // Serial.println(); 
  
}

void showMenu() {
  if (menuIndex == 0) {
  Serial.println("Entering in menuIndex 0");
  sensors.requestTemperatures(); 
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 60);
  tft.println("Temperature");
  tft.printf("%.3f"); 
  delay(100);
  }
    else if (menuIndex == 1) {
  sensors_event_t event; 
  accel.getEvent(&event); 
   /* Display the results (acceleration is measured in m/s^2) */
  Serial.println("Entering in menuIndex 1");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 60);
  tft.println("Acceleration");
  tft.print("X: "); tft.print(event.acceleration.x); tft.print("  ");
  tft.print("Y: "); tft.print(event.acceleration.y); tft.print("  ");
  tft.print("Z: "); tft.print(event.acceleration.z); tft.print("  ");
  delay(100);
  }
   else if (menuIndex == 2) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);  
  Serial.println("Entering in menuIndex 2");
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 60);
  tft.println("Current Time");
  tft.println(&timeinfo, "%I:%M:%S");
  delay(100);
  }
}