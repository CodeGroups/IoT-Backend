#include <NTPClient.h>
#include <WiFiUdp.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

// Set these to run example.
#define FIREBASE_HOST "iot-pastillero-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "4an67TrO4B7ajIQOfUWKcFkhWjyxvIskTF0U2v5E"
#define WIFI_SSID "COMTECO-N3723723"
#define WIFI_PASSWORD "DCQWV21408"
#define D1 5 //led casilla 1
#define D2 4 //led casilla 2
#define D3 0 //sensor casilla 1
#define D4 2 //sensor casilla 2
#define D5 14 //estereo

const long utcOffsetInSeconds = -14400;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


volatile bool estado = false;
FirebaseData firebaseData;
String path = "/";
long timeLong;
int sensor1,sensor2;
int hours_attention[9];



void setup() {
  Serial.begin(9600);
  //use inout for sensor
  pinMode(D3 , INPUT);
  pinMode(D4 , INPUT);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("WIFI connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D5, OUTPUT);

  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
  timeClient.setUpdateInterval(60000);
  
  //get_config_1();
}


void loop() {
  sensor1 = digitalRead(D3);  //lectura digital de pin
  sensor2 = digitalRead(D4);  //lectura digital de pin

  configAll();
  showTime();
  make_up_hours();
  check_time();
  
  if (estado)
  {
    digitalWrite(D4, HIGH);
  }
  else
  {
    digitalWrite(D4, LOW);
  }
  delay(60000);
}

void showTime() {
  
  unsigned long epochTime = timeClient.getEpochTime();
  timeLong = epochTime;
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

/* void actualizar_estado()
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
} */

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

void make_up_hours(){
  FirebaseJsonData jsonData;
  //por ahora 2 para la prueba
  for(int i=0; i<2;i++){
    firebaseData.jsonObject().get(jsonData, "dispositivo/casilla-00" + String(i+1) + "/horario/tiempo_inicio");
    hours_attention = jsonData.intValue;
  }
}

void check_time(){
  int c=0;
  if(hours_attention[0] == timeLong){
    digitalWrite(D1, HIGH);
    digitalWrite(D5, HIGH);
    while(c<60){
      if(sensor1 == HIGH){
        digitalWrite(D1, LOW);
        digitalWrite(D5, LOW);
        //guardar historial
      }
      delay(1000);
      c++;
    }
    digitalWrite(D5, LOW);
    digitalWrite(D1, LOW);
  }
  else if(hours_attention[1] == time){
    digitalWrite(D2, HIGH);
    digitalWrite(D5, HIGH);
    while(c<60){
      if(sensor1 == HIGH){
        digitalWrite(D2, LOW);
        digitalWrite(D5, LOW);
        //guardar historial
      }
      delay(1000);
      c++;
    }
    digitalWrite(D5, LOW);
    digitalWrite(D2, LOW);
  }
}
