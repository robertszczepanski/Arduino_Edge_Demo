#include <math.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Wire.h>

#define FIREBASE_HOST "YOUR_FIREBASE_HOST_URL" //Without http:// or https:// schemes
#define FIREBASE_AUTH "YOUR_FIREBASE_AUTHORIZATION_TOKEN"
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

#define DATA_LENGTH 7
#define SENSORS 3
#define MSG_LENGTH (DATA_LENGTH * SENSORS)

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseData ledData;

FirebaseJson json;

int sensorValue;

struct DHT_MEASURES{
  float h; // humidity
  float t; // temperature in Celsius
  float f; // temperature in Fahrenheit
};
DHT_MEASURES dht_m;

void setup(){
  Serial.begin(9600);
  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */

  /* Configure WiFi network */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Configure Firebase database */
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  Serial.println("System initialized\n");
}

void sensorUpdate() {
  String dataString = "";

  Wire.requestFrom(8, DATA_LENGTH * 3);
  while (Wire.available()) {
    char c = Wire.read();
    dataString = dataString + c;
  }

  dht_m.h = getValueFromMsg('H', dataString);
  dht_m.t = getValueFromMsg('T', dataString);
  sensorValue = getValueFromMsg('B', dataString);
  
  Serial.print("Received string: \"");
  Serial.print(dataString);
  Serial.println("\"");
}

float getValueFromMsg(char type, String msg) {
  if(msg.length() != MSG_LENGTH) {
    Serial.print("Unsupported message length, required: ");
    Serial.print(MSG_LENGTH);
    Serial.print(", actual: ");
    Serial.println(msg.length());
    return -1;
  }
  if(type == 'H') {
    Serial.print("Parsed humidity: ");
    Serial.println(msg.substring(1, DATA_LENGTH));
    return msg.substring(1, DATA_LENGTH).toFloat();
  } else if (type == 'T') {
    Serial.print("Parsed temperature: ");
    Serial.println(msg.substring(DATA_LENGTH + 1, DATA_LENGTH * 2));
    return msg.substring(DATA_LENGTH + 1, DATA_LENGTH * 2).toFloat();
  } else if (type == 'B') {
    Serial.print("Parsed brightness: ");
    Serial.println(msg.substring(DATA_LENGTH * 2 + 1, DATA_LENGTH * 3));
    return msg.substring(DATA_LENGTH * 2 + 1, DATA_LENGTH * 3).toFloat();
  } else {
    Serial.print("Unsupported type for given message! Type must be one of following: H, T, B. Type requested: ");
    Serial.println(type);
    return -1;
  }
}

void firebaseUpdate(){
  if (Firebase.setFloat(firebaseData, "/ASB Arduino Project/temperature", dht_m.t))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
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

  if (Firebase.setFloat(firebaseData, "/ASB Arduino Project/humidity", dht_m.h))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
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

  if (Firebase.setFloat(firebaseData, "/ASB Arduino Project/brightness", sensorValue))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
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

void loop()
{
  sensorUpdate();
  firebaseUpdate();
  delay(2000);
}
