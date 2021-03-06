#include <Arduino.h>
#include <ros.h>
#include <ros/time.h>
#include <geometry_msgs/Vector3.h>
#include <sensor_msgs/NavSatFix.h>
#include <std_msgs/String.h>
#include <nav_msgs/Odometry.h>
#include <string.h>
#include <Time.h>
#include <DFRobot_sim808.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>

ros::NodeHandle nh;

sensor_msgs::NavSatFix gpsMsg;
ros::Publisher gps("gps", &gpsMsg);

// Publisher object for filtered IMU

float GPS_la = 0.1f;
float GPS_lo = 0.1f;
float GPS_ws = 0.1f;
float GPS_alt = 0.1f;
float GPS_heading = 0.1f;
uint16_t GPS_year = 1;
uint8_t GPS_month = 1;
uint8_t GPS_day = 1;
uint8_t GPS_hour = 1;
uint8_t GPS_minute = 1;
uint8_t GPS_second = 1;
uint8_t GPS_centisecond = 1;
#define GPS_Sampling_Time_ms 100
unsigned long currentMillis_GPS = 0;
unsigned long previousMillis_GPS = 0;

unsigned long seq = 0;

bool state = LOW;
#define LED 8

void stateChange(){
  state = !state;
  digitalWrite(LED, state);  
}

bool isRosConnected = false;

File Logger;
int pinCS = 53; // SD card digital pin
#define Logger_Sampling_Time_ms 1000
unsigned long currentMillis_SD = 0;
unsigned long previousMillis_SD = 0;
String Logger_file_name = "Logged";
char *Logger_file_name_ptr, Logger_file_name_char;

#define GSM_MESSAGE_LENGTH 160
char GSM_message[GSM_MESSAGE_LENGTH];
int GSM_messageIndex = 0;
char GSM_MESSAGE[300];
char GSM_lat[12];
char GSM_lon[12];
char GSM_wspeed[12];
char GSM_heading[12];
#define GSM_phone "+4915758752522"
char GSM_datetime[24];
#define PIN_TX 10
#define PIN_RX 11

SoftwareSerial mySerial(PIN_TX,PIN_RX);
//The content of messages sent
#define GSM_Initial_MESSAGE  "Hello Master, This is AttBot at your service. Tell me what to do!"
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,

// uncomment "OUTPUT_READABLE_GPS_GSM" if you want to see the GPS data sent to you per Msg
//#define OUTPUT_READABLE_GPS_GSM


#define GSM_Sampling_Time_ms 250
unsigned long currentMillis_GSM = 0;
unsigned long previousMillis_GSM = 0;

void getGPS();
void readSMS();

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {

  //Connect to ROS
  nh.initNode();

  //advertise GPS data
  nh.advertise(gps);

  pinMode(LED, OUTPUT); // Declare the LED as an output
  
  // initialize serial communication
  mySerial.begin(9600);
  //Serial.begin(57600);     //open serial and set the baudrate

    // ******** Initialize sim808 module *************
  while(!sim808.init())
  {
     //Serial.print("Sim808 init error\r\n");
     delay(1000);
  }
  delay(3000);


  if(sim808.attachGPS()){
     //Serial.println("Open the GPS power success");
  }else{
     //Serial.println("Open the GPS power failure");
  }
  
  //Serial.println("Init Success, please send SMS message to me!");

  //******** test phone number and text **********
  //sim808.sendSMS(GSM_phone,GSM_Initial_MESSAGE);

  Logger_file_name += ".txt";
  Logger_file_name_ptr = &Logger_file_name_char;
  Logger_file_name.toCharArray(Logger_file_name_ptr, 50);
  //Serial.println(Logger_file_name_char);

  pinMode(pinCS, OUTPUT);

  //SD Card Initialization
  if (SD.begin())
  {
   //Serial.println("SD card is ready to use.");
  } else
  {
   //Serial.println("SD card initialization failed");
   return;
  }
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
  // check the sampling time of acquising GPS data
  currentMillis_GSM = millis();
  if (currentMillis_GSM - previousMillis_GSM > GSM_Sampling_Time_ms) {
    //*********** Detecting unread SMS ************************
    GSM_messageIndex = sim808.isSMSunread();

    //*********** At least, there is one UNREAD SMS ***********
    if (GSM_messageIndex > 0){
      readSMS();
     }
     previousMillis_GSM = currentMillis_GSM;
  }

  //check the sampling time of acquising GPS data
  currentMillis_GPS = millis();
  if (currentMillis_GPS - previousMillis_GPS > GPS_Sampling_Time_ms) {
    getGPS();
    
    //************* Turn off the GPS power ************
    sim808.detachGPS();

      if(nh.connected()){
        isRosConnected = true;
        //publish GPS data
        gpsMsg.header.stamp = nh.now();
        gpsMsg.header.frame_id = "map";
        gpsMsg.header.seq = seq++;
        gpsMsg.latitude = GPS_la;
        gpsMsg.longitude = GPS_lo;
        gpsMsg.altitude = GPS_alt;
        //gpsMsg.position_covariance_type = 0;

        gps.publish(&gpsMsg);
        nh.spinOnce();
      }else{
        isRosConnected = false;
        //nh.initNode();

        //publish GPS data
        gpsMsg.header.stamp = nh.now();
        gpsMsg.header.frame_id = "map";
        gpsMsg.header.seq = seq++;
        gpsMsg.latitude = GPS_la;
        gpsMsg.longitude = GPS_lo;
        gpsMsg.altitude = GPS_alt;
        //gpsMsg.position_covariance_type = 0;

        gps.publish(&gpsMsg);
        nh.spinOnce();
      }
    previousMillis_GPS = currentMillis_GPS;
  }

  // check the SD logging sampling time in ms
  // currentMillis_SD = millis();
  // if (currentMillis_SD - previousMillis_SD > Logger_Sampling_Time_ms) {
  //   // Write the data to the logger
  //   // Logger = SD.open(Logger_file_name_char, FILE_WRITE);
  //   Logger = SD.open("Log.txt", FILE_WRITE);
  //   if (Logger) {
  //     Logger.print(GPS_year);
  //     Logger.print(",");
  //     Logger.print(GPS_month);
  //     Logger.print(",");
  //     Logger.println(GPS_day);
  //     Logger.print(",");
  //     Logger.print(GPS_hour);
  //     Logger.print(",");
  //     Logger.print(GPS_minute);
  //     Logger.print(",");
  //     Logger.print(GPS_second);
  //     Logger.print(",");
  //     Logger.print(GPS_centisecond);
  //     Logger.print(",");
  //     Logger.print(GPS_la);
  //     Logger.print(",");
  //     Logger.print(GPS_lo);
  //     Logger.print(",");
  //     Logger.print(GPS_alt);
  //     Logger.print(",");
  //     Logger.print(GPS_ws);
  //     Logger.print(",");
  //     Logger.println(GPS_heading);
  //     Logger.close(); // close the file
  //   }else {
  //     Serial.println("error opening Logger.txt");
  //    }
  //    previousMillis_SD = currentMillis_SD;
  //  }
  
  //nh.spinOnce();
  
  delay(2);
}

void readSMS()
{
  //Serial.print("messageIndex: ");
  //Serial.println(GSM_messageIndex);
  
  //sim808.readSMS(GSM_messageIndex, GSM_message, GSM_MESSAGE_LENGTH, GSM_phone, GSM_datetime);
             
  //***********In order not to full SIM Memory, is better to delete it**********
  //sim808.deleteSMS(GSM_messageIndex);
  //Serial.print("From number: ");
  //Serial.println(GSM_phone);  
  //Serial.print("Datetime: ");
  //Serial.println(GSM_datetime);        
  //Serial.print("Recieved Message: ");
  //Serial.println(GSM_message);
}

void getGPS(){ 
  while(!sim808.attachGPS())
  {
    //Serial.println("Open the GPS power failure");
  }
  delay(80);

  //Serial.println("Open the GPS power success");
  digitalWrite(LED, HIGH);
    
  if(!sim808.getGPS())
  {
    //Serial.println("not getting anything");
    stateChange();
  }

//   Serial.print(sim808.GPSdata.year);
//   Serial.print("/");
//   Serial.print(sim808.GPSdata.month);
//   Serial.print("/");
//   Serial.print(sim808.GPSdata.day);
//   Serial.print(" ");
//   Serial.print(sim808.GPSdata.hour);
//   Serial.print(":");
//   Serial.print(sim808.GPSdata.minute);
//   Serial.print(":");
//   Serial.print(sim808.GPSdata.second);
//   Serial.print(":");
//   Serial.println(sim808.GPSdata.centisecond);
//   Serial.print("latitude :");
//   Serial.println(sim808.GPSdata.lat);
//   Serial.print("longitude :");
//   Serial.println(sim808.GPSdata.lon);
//   Serial.print("speed_kph :");
//   Serial.println(sim808.GPSdata.speed_kph);
//   Serial.print("heading :");
//   Serial.println(sim808.GPSdata.heading);
//   Serial.println();

  GPS_la = sim808.GPSdata.lat;
  GPS_lo = sim808.GPSdata.lon;
  GPS_ws = sim808.GPSdata.speed_kph;
  GPS_alt = sim808.GPSdata.altitude;
  GPS_heading = sim808.GPSdata.heading;
  GPS_year = sim808.GPSdata.year;
  GPS_month = sim808.GPSdata.month;
  GPS_day = sim808.GPSdata.day;
  GPS_hour = sim808.GPSdata.hour;
  GPS_minute = sim808.GPSdata.minute;
  GPS_second = sim808.GPSdata.second;
  GPS_centisecond = sim808.GPSdata.centisecond;

  #ifdef OUTPUT_READABLE_GPS_GSM
    dtostrf(GPS_la, 6, 2, GSM_lat); //put double value of la into char array of lat. 6 = number of digits before decimal sign. 2 = number of digits after the decimal sign.
    dtostrf(GPS_lo, 6, 2, GSM_lon); //put double value of lo into char array of lon
    dtostrf(GPS_ws, 6, 2, GSM_wspeed);  //put double value of ws into char array of wspeed
    //sprintf(GSM_MESSAGE, "Latitude : %s\nLongitude : %s\nWind Speed : %s kph\nMy Module Is Working. Atta Oveisi. Try With This Link.\nh_odomttp://www.latlong.net/Show-Latitude-Longitude.html\nh_odomttp://maps.google.com/maps?q=%s,%s\n", GSM_lat, GSM_lon, GSM_wspeed, GSM_lat, GSM_lon);
    //Serial.println("Sim808 init success");
    //Serial.println("Start to send message ...");
    //Serial.println(GSM_MESSAGE);
    //Serial.println(GSM_phone);
    sim808.sendSMS(GSM_phone,GSM_MESSAGE);
  #endif
}