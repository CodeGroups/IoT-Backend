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
String path = "/dispositivo/";
long timeLong;
int sensor1,sensor2;
int hours_attention[9];

FirebaseData fbDataCasilla[2];

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
  
  
}


void loop() {
  sensor1 = digitalRead(D3);  //lectura digital de pin
  sensor2 = digitalRead(D4);  //lectura digital de pin

  updateFirebaseData();
  
  showTime();

  make_up_hours(0);
  make_up_hours(1);
  
  check_time();
  delay(60000);
}

void updateFirebaseData() {
  while(true)
  {
    if(getDataFromFirebase("/dispositivo/casilla-001", 0)){
      break;
    }
    delay(10000);
  }
  while(true)
  {
    if(getDataFromFirebase("/dispositivo/casilla-002/", 1)){
      break;
    }
    delay(10000);
  }
}

void showTime() {
  timeClient.update();
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

bool getDataFromFirebase(String mPath, int casilla) {
  Serial.print("Trying: ");
  Serial.println(mPath);
  Firebase.getJSON(fbDataCasilla[casilla], mPath);

  if (fbDataCasilla[casilla].jsonString().length() > 0)
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbDataCasilla[casilla].dataPath());
    Serial.println("TYPE: " + fbDataCasilla[casilla].dataType());
    Serial.println("ETag: " + fbDataCasilla[casilla].ETag());
    Serial.print("VALUE: ");
    String value = fbDataCasilla[casilla].jsonString();
    Serial.println(value);
    Serial.println("------------------------------------");
    Serial.println();
    
    Serial.println("------------------------------------");
    Serial.println();
    return true;
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbDataCasilla[casilla].errorReason());
    Serial.println("------------------------------------");
    Serial.println();
    return false;
  }
}

void make_up_hours(int casilla){
  FirebaseJsonData jsonData;
  //por ahora 2 para la prueba
  fbDataCasilla[casilla].jsonObject().get(jsonData, "/horario/tiempo_inicio");
  hours_attention[casilla] = jsonData.intValue;
  Serial.print("hours_attention: ");
  Serial.println(hours_attention[casilla]);
}

void check_time(){
  int c=0;
  FirebaseJsonData jsonData;
  // debemos darle un rango
  int tiempo_minimo = timeLong - 30;
  int tiempo_limite = timeLong + 30; 
  if(hours_attention[0] >= tiempo_minimo && hours_attention[0] <= tiempo_limite ){
    //recupera intervalo
    fbDataCasilla[0].jsonObject().get(jsonData, "/horario/intervalo");
    int intervalo1 = jsonData.intValue;
    digitalWrite(D1, HIGH);
    digitalWrite(D5, HIGH);
    String mPath = fbDataCasilla[0].dataPath();
    //cambiamos el estado a acttivo
    if (Firebase.setBool(fbDataCasilla[0], fbDataCasilla[0].dataPath()+"/estado", true)) {
      getDataFromFirebase(mPath, 0);
      Serial.println("Pastillero activo");
    } else {
      Serial.println(fbDataCasilla[0].errorReason());
    }
    
    while(c<60){
      sensor1 = digitalRead(D3);  //lectura digital de pin
      Serial.print("Sensor 1: ");
      Serial.println(sensor1);
      if(sensor1 == LOW){
        digitalWrite(D1, LOW);
        digitalWrite(D5, LOW);
        //guardar historial
        if (Firebase.setInt(fbDataCasilla[0], mPath + "/horario/tiempo_inicio", (int(timeLong) + intervalo1))) {
          getDataFromFirebase(mPath, 0);
          //cambiamos el estado a apagado
          if (Firebase.setBool(fbDataCasilla[0], mPath + "/estado", false)) {
            getDataFromFirebase(mPath, 0);
            Serial.println("Pastillero activo");
          } else {
            Serial.println(fbDataCasilla[0].errorReason());
          }
        } else {
          Serial.println(firebaseData.errorReason());
        }
        break;
      }
      delay(1000);
      c++;
    }
    digitalWrite(D5, LOW);
    digitalWrite(D1, LOW);
    //cambiamos el estado a desactivado
    if (Firebase.setBool(fbDataCasilla[0], mPath + "/estado", false)) {
      getDataFromFirebase(mPath, 0);
      Serial.println("Pastillero activo");
    } else {
      Serial.println(fbDataCasilla[0].errorReason());
    }
  }
  else if(hours_attention[1] >= tiempo_minimo && hours_attention[1] <= tiempo_limite){
    fbDataCasilla[1].jsonObject().get(jsonData, "/horario/intervalo");
    int intervalo2 = jsonData.intValue;
    digitalWrite(D2, HIGH);
    digitalWrite(D5, HIGH);
    String mPath = fbDataCasilla[1].dataPath();
    //cambiamos el estado a acttivo
    if (Firebase.setBool(fbDataCasilla[1], fbDataCasilla[1].dataPath()+"/estado", true)) {
      getDataFromFirebase(mPath, 1);
      Serial.println("Pastillero activo");
    } else {
      Serial.println(fbDataCasilla[1].errorReason());
    }
    // cambiar en la bd el estado a true
    while(c<60){
      sensor2 = digitalRead(D4);  //lectura digital de pin
      Serial.print("Sensor 2: ");
      Serial.println(sensor2);
      if(sensor2 == LOW){
        digitalWrite(D2, LOW);
        digitalWrite(D5, LOW);
        //actualizar horario
        if (Firebase.setInt(fbDataCasilla[1], mPath + "/horario/tiempo_inicio", (int(timeLong) + intervalo2))) {
          getDataFromFirebase(mPath, 1);
          if (Firebase.setBool(fbDataCasilla[1], fbDataCasilla[1].dataPath()+"/estado", false)) {
            getDataFromFirebase(mPath, 1);
            Serial.println("Pastillero activo");
          } else {
            Serial.println(fbDataCasilla[1].errorReason());
          }
        } else {
          Serial.println(firebaseData.errorReason());
        }
        break;
      }
      delay(1000);
      c++;
    }
    // cambiar en la bd el estado a false
    digitalWrite(D5, LOW);
    digitalWrite(D2, LOW);
    if (Firebase.setBool(fbDataCasilla[1], mPath + "/estado", false)) {
      getDataFromFirebase(mPath, 1);
      Serial.println("Pastillero activo");
    } else {
      Serial.println(fbDataCasilla[1].errorReason());
    }
  }
}

int get_hours_interval( int interval){

}
