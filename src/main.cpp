#include <Arduino.h>
#include "data.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char *ssid = "POCOPHONE";
const char *password = "a1b2c3d4";
void Connect_Wifi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("OK! IP=");
  Serial.println(WiFi.localIP());
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Connect_Wifi();
  Serial.println("Get home");
  GET_home();
  Serial.println("Put home");
  PUT_home();
  // Serial.println("Get resetpin");
  // GET_resetpin();
}

void loop()
{
  // put your main code here, to run repeatedly:
}