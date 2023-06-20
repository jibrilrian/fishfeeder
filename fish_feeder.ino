#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>
#include "FirebaseESP8266.h"

// Set WiFi credentials
#define WIFI_SSID "GeekHale"
#define WIFI_PASS "[87412GeekHale89632]"
#define FIREBASE_HOST "fish-feeder-a6b91-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "cbc50031f44dcfeac52b88b133b748733d2f0878"

Servo servo;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org",25200);


FirebaseData timer,feed;

String stimer;
String Str[]={"25:00","25:00","25:00"};
int i,feednow=0;
String currMinutes;
String currHours;

void runServo() {
  servo.write(0);
  delay(1000);
  servo.write(180);
}

void sysReady() {
  Serial.println("System ready for next command!");
}

void setup() {
  // Setup serial port
  Serial.begin(115200);
  Serial.println();

  // Begin WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  servo.attach(D3);
  servo.write(180);
}

void loop() {
  
  Firebase.getInt(feed,"feednow");
  feednow = feed.to<int>();
  
  if (feednow==1) // Direct Feeding
  {
      runServo();
      feednow = 0;
      Firebase.setInt(feed, "/feednow",feednow);
      Serial.println("Fed by click success!");
      sysReady();
  }
  
  String path = "timers/timer"+String(0)+"/time";
  Firebase.getString(timer,path);
  if(timer.to<String>() != "") { // Scheduling feed
    for(i=0;i<3;i++){
      stimer = timer.to<String>();
      Str[i]= stimer.substring(0,5);
    }
    timeClient.update();
    if(timeClient.getHours() < 10)
    {
      currHours = "0"+String(timeClient.getHours());
    } else {
      currHours = String(timeClient.getHours());
    }
    if(timeClient.getMinutes() < 10)
    {
      currMinutes = "0"+String(timeClient.getMinutes());
    } else {
      currMinutes = String(timeClient.getMinutes());
    }
  }
  
  String currentTime = currHours+":"+currMinutes;

  if (Str[0]==currentTime||Str[1]==currentTime||Str[2]==currentTime)
  {
    runServo();
    Serial.println("Fed by schedule success!");
    delay(60000);
    sysReady();
  }
}
