#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Time.h>
#include <time.h>
#include <TimeLib.h>
//#include <SoftReset.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Time.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include "RestClient.h"
//#include <Ethernet.h>
#include <SPI.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiClient.h>

int n=0;
int timeset=0;
int timetry=0;
long tst=0;
long tnw=0;
//char ssid[] = "BGWIFI";  //  Network SSID (name)
//char pass[] = "bg123456";       // Network password
const char* host = "esp8266-webupdate";
char ssid[] = "bg_wifi1";  //  Network SSID (name)
char pass[] = "924@gambas";       // Network password

RestClient client = RestClient("52.74.122.123",80);

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {
  Serial.begin(115200); 

    // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
//  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   
  
  pinMode(5, OUTPUT);           // set pin to input
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);    //Buzzer
  digitalWrite(5, LOW);    //Light
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);

  tst=millis();

  
}

void loop(){
  //Check WiFi
  ESP.wdtEnable(15000);
  
  while (WiFi.status() != WL_CONNECTED) {
    timeset=0;
    delay(500);
    Serial.println("WiFi Down");
    timetry=timetry+1;
    if (timetry==10){
      Serial.println("WiFi is not connected, Reset");  
      ESP.reset();  
    }
  }
  
  timetry=0;
  tnw=millis();
  if ((tnw-tst)>10800000){
     Serial.println("Timeout, Reset");  
     ESP.reset();
  }
   
  String response;
  //String deviceID = "B1010";
  const char* thistring = "{\"Station_ID\":\"B1023\"}";
  Serial.print (thistring);
  response = "";
  int statusCode = client.put("/blulight/getstationstatus.php", thistring, &response);
  Serial.print("Status code from server: ");
  Serial.println(statusCode);
  Serial.print("Response body from server: ");
  Serial.println(response);
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(response);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    delay (1000);    
  }
  else{
    int light = root["Lightning_Status"];
    int sound = root["Sound_Status"];
    Serial.print("light is ");
    Serial.println(light); 
    Serial.print("Sound is ");
    Serial.println(sound);
    if (light==1){
      digitalWrite(5, HIGH);
      Serial.println("Light High");
      if (sound==1){
        if(n==0){
           int soundtime = root["Sound_Time"];      
           digitalWrite(4, HIGH);
           Serial.println("Buzzer High");
           delay(soundtime*1000);
           digitalWrite(4, LOW);
           Serial.println("Buzzer Low");
           n=1;
        }
        else{// After the 4 seconds buzzer will not work 
          digitalWrite(4, LOW);
          Serial.println("Buzzer Low");    
        }
      }
      else{
        digitalWrite(4, LOW);
           Serial.println("Buzzer Low");
      }
    }
    else {
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
      Serial.println("Light and Sound Low");
      n=0;
    }
    Serial.println(light);
    Serial.println(sound);
  }
  delay(5000);
  httpServer.handleClient();
  
}
