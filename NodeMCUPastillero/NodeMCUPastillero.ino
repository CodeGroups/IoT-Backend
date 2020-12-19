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
  
  while(true)
  {
    if(configAll){
      break;
    }
  }
}


void loop() {
  sensor1 = digitalRead(D3);  //lectura digital de pin
  sensor2 = digitalRead(D4);  //lectura digital de pin

  //configAll();
  showTime();
  make_up_hours();
  check_time();
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
    //get_config_1();
    make_up_hours();
    Serial.println("------------------------------------");
    Serial.println();
    return true
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
    return false
  }
}

void make_up_hours(){
  FirebaseJsonData jsonData;
  //por ahora 2 para la prueba
  for(int i=0; i<2;i++){
    firebaseData.jsonObject().get(jsonData, "dispositivo/casilla-00" + String(i+1) + "/horario/tiempo_inicio");
    hours_attention[i] = jsonData.intValue;
  }
}

void check_time(){
  int c=0;
  FirebaseJson json1;
  FirebaseJson json2;
  if(hours_attention[0] == timeLong){
    //se construye el json
    json1.add("hora", timeLong).add("estado", true);
    json2.add("hora", timeLong).add("temp2", false);
    digitalWrite(D1, HIGH);
    digitalWrite(D5, HIGH);
    while(c<60){
      if(sensor1 == HIGH){
        digitalWrite(D1, LOW);
        digitalWrite(D5, LOW);
        //guardar historial
        if (Firebase.pushJSON(firebaseData, "dispositivo/casilla-001/historial/"+ String(timeLong)+"/", json1)) {
          Serial.println(firebaseData.dataPath() + "/"+ firebaseData.pushName());        
        } else {
          Serial.println(firebaseData.errorReason());
        }
      }
      delay(1000);
      c++;
    }
    digitalWrite(D5, LOW);
    digitalWrite(D1, LOW);
    if (Firebase.pushJSON(firebaseData, "dispositivo/casilla-001/historial/"+String(timeLong)+"/", json2)) {
          Serial.println(firebaseData.dataPath() + "/"+ firebaseData.pushName());        
        } else {
          Serial.println(firebaseData.errorReason());
        }
  }
  else if(hours_attention[1] == timeLong){
    digitalWrite(D2, HIGH);
    digitalWrite(D5, HIGH);
    // cambiar en la bd el estado a true
    while(c<60){
      if(sensor1 == HIGH){
        digitalWrite(D2, LOW);
        digitalWrite(D5, LOW);
        //guardar historial

        //actualizar horario
        if (Firebase.pushJSON(firebaseData, "/dispositivo/casilla-002/historial/"+String(timeLong)+"/", json1)) {
          Serial.println(firebaseData.dataPath() + "/"+ firebaseData.pushName());        
        } else {
          Serial.println(firebaseData.errorReason());
        }
      }
      delay(1000);
      c++;
    }
    // cambiar en la bd el estado a false
    digitalWrite(D5, LOW);
    digitalWrite(D2, LOW);
    if (Firebase.pushJSON(firebaseData, "/dispositivo/casilla-002/historial/" + String(timeLong) + "/", json2)) {
      Serial.println(firebaseData.dataPath() + "/"+ firebaseData.pushName());        
    } else {
      Serial.println(firebaseData.errorReason());
    }
  }
}
