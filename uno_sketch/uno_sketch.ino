#include <math.h>
#include <Wire.h>
#include "DHT.h"

#define BUZZER 3  // D3
#define LIGHT_SENSOR A0
#define DHTPIN 4  // D4
#define DHTTYPE DHT11
#define LED 5

DHT dht(DHTPIN, DHTTYPE);
float sensorValue;
float Rsensor;

struct DHT_MEASURES{
  float h; // humidity
  float t; // temperature in Celsius
  float f; // temperature in Fahrenheit
};
DHT_MEASURES dht_m;

/*
 * Read data from dht sensor and write it to dht_m struct.
 */
DHT_MEASURES dht_update(DHT_MEASURES *dht_m, DHT *dht) {
  dht_m->h = dht->readHumidity();
  dht_m->t = dht->readTemperature();
  dht_m->f = dht->readTemperature(true);
}

void setup(){
  Serial.begin(9600);

  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
 
  pinMode(BUZZER, OUTPUT);

  dht.begin();
  Serial.println("System initialized\n");
}

void requestEvent() {
  sendHumidity();
  sendTemperature();
  sendBrightness();
}

void receiveEvent(int howMany) {
  String dataString = "";

  while (0 <Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    dataString = dataString + c;
  }

  /* Assume received `LEDX` where X is a float number
     If message is longer, ignore everything past that
     float is contained in 6 bytes
  */
  int value = dataString.substring(3, 3 + 6).toFloat();
  if(value == 1.0) {
    digitalWrite(LED, HIGH);
  } else if (value == 0.0) {
    digitalWrite(LED, LOW);
  } else {
    Serial.print("Incorrect LED state value: ");
    Serial.print(value);
    Serial.println(" leaving unchanged");
  }
  Serial.println();
}

void sendHumidity() {
  char buff[7];
  dtostrf(dht_m.h,6,2, buff);
  Wire.write("H");
  Wire.write(buff);
  Serial.print("Sending \"H");
  Serial.print(buff);
  Serial.println("\"");
}

void sendTemperature() {
  char buff[7];
  dtostrf(dht_m.t,6,2, buff);
  Wire.write("T");
  Wire.write(buff);
  Serial.print("Sending \"T");
  Serial.print(buff);
  Serial.println("\"");
}

void sendBrightness() {
  char buff[7];
  dtostrf(float(sensorValue),6,2, buff);
  Wire.write("B");
  Wire.write(buff);
  Serial.print("Sending \"B");
  Serial.print(buff);
  Serial.println("\"");
}


void loop()
{
  String dataString = "";
  delay(2000);

  /* Buzzer works great but it's annoying so comment it out for now */
//  digitalWrite(BUZZER, HIGH);
//  delay(200);
//  digitalWrite(BUZZER, LOW);

  sensorValue = analogRead(LIGHT_SENSOR);
  Rsensor = (float)(1023-sensorValue)*10/sensorValue;


  Serial.print("Light brightness is ");
  Serial.print(sensorValue);
  Serial.println(" in range 0 to 540");
  Serial.print("The sensor resistance is ");
  Serial.print(Rsensor,DEC);
  Serial.println(" Ohm");

  dht_update(&dht_m, &dht);
  // Check if any reads failed and exit early (to try again).
  if (isnan(dht_m.h) || isnan(dht_m.t) || isnan(dht_m.f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  Serial.print(F("Humidity: "));
  Serial.print(dht_m.h);
  Serial.print(F("%  Temperature: "));
  Serial.print(dht_m.t);
  Serial.print(F("°C "));
  Serial.println(dht_m.f);

  Serial.println("");
}
