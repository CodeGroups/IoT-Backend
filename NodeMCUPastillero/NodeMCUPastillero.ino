#include <NTPClient.h>
#include <WiFiUdp.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

// Set these to run example.
#define FIREBASE_HOST "iot-pastillero-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "4an67TrO4B7ajIQOfUWKcFkhWjyxvIskTF0U2v5E"
#define WIFI_SSID "COMTECO-N3723723"
#define WIFI_PASSWORD "DCQWV21408"
#define Led D4


const long utcOffsetInSeconds = -14400;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


volatile bool estado = false;
FirebaseData firebaseData;
String path = "/";




void setup() {
  Serial.begin(9600);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  pinMode(Led, OUTPUT);

  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
  timeClient.setUpdateInterval(60000);
  
  //get_config_1();
}


void loop() {
  configAll();
  showTime();
  
  if (estado)
  {
    digitalWrite(Led, HIGH);
  }
  else
  {
    digitalWrite(Led, LOW);
  }
  delay(60000);
}

void showTime() {
  
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print(epochTime);
  Serial.print(" - ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.print(timeClient.getSeconds());
  Serial.print(" - ");
  Serial.println(timeClient.getFormattedTime());
}

void actualizar_estado()
{
  Firebase.getJSON(firebaseData, path + "medicine");

  if (firebaseData.jsonString().length() > 0)
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.print("VALUE: ");
    String value = firebaseData.jsonString();
    //estado = firebaseData.boolData();
    Serial.println(value);
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  delay(20000);
}

int current_time;

void configAll() {
  Firebase.getJSON(firebaseData, path);

  if (firebaseData.jsonString().length() > 0)
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.print("VALUE: ");
    String value = firebaseData.jsonString();
    //estado = firebaseData.boolData();
    Serial.println(value);
    Serial.println("------------------------------------");
    Serial.println();
    get_config_1();
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}


//slot 1
String id_1 = "med-001";
bool  historial_status_1;
String historial_time_1;
int   quantity_1;
String   first_day_1;
String   init_time_1;
int   interval_1;

void get_config_1() {

  /*
  Firebase.getJSON(historial,   path + "historial/" + id);
  Firebase.getJSON(medicine,    path + "medicine/"  + id);
  Firebase.getJSON(schedule,    path + "schedule/"  + id);

  */
  
  FirebaseJsonData jsonData;
  
  firebaseData.jsonObject().get(jsonData, "historial/" + id_1 + "/status");
  historial_status_1 = jsonData.boolValue;
  
  firebaseData.jsonObject().get(jsonData, "historial/" + id_1 + "/time");
  historial_time_1 = jsonData.stringValue;

  firebaseData.jsonObject().get(jsonData, "medicine/" + id_1 + "/quantity");
  quantity_1 = jsonData.intValue;

  firebaseData.jsonObject().get(jsonData, "schedule/" + id_1 + "/first_date");
  first_day_1 = jsonData.stringValue;

  firebaseData.jsonObject().get(jsonData, "schedule/" + id_1 + "/init_time");
  init_time_1 = jsonData.stringValue;

  firebaseData.jsonObject().get(jsonData, "schedule/" + id_1 + "/interval");
  interval_1 = jsonData.intValue;

  Serial.print("historial_status_1:   ");
  Serial.println(historial_status_1);
  Serial.print("historial_time_1:     ");
  Serial.println(historial_time_1);
  Serial.print("quantity_1:           ");
  Serial.println(quantity_1);
  Serial.print("first_day_1:          ");
  Serial.println(first_day_1);
  Serial.print("init_time_1:          ");
  Serial.println(init_time_1);
  Serial.print("interval_1:           ");
  Serial.println(interval_1);
  
  
  /*
  quantity_1;
  start_day_1;
  start_time_1;
  next_alarm_1;
  */
  
}

void updateStatusDown_1() {
  
}
