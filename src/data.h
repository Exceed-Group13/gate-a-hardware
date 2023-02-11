#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// declare variable
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
///////////////////////////////////////////////////////////////
const String baseUrl =
    "https://ecourse.cpe.ku.ac.th/exceed13";
String result, house_name;
JsonArray result_array, pin, new_pin;
bool temp_is_open,is_open;
int delay_val;

void GET_home()
{
    DynamicJsonDocument doc(4096);
    HTTPClient http;
    const String url = baseUrl + "/home";
    http.begin(url);

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200)
    {
        String payload = http.getString();
        Serial.println(payload);
        deserializeJson(doc, payload);

        result_array = doc["result"].as<JsonArray>();

        temp_is_open = result_array[0]["state"].as<bool>();
        house_name = result_array[0]["house_name"].as<String>();
        delay_val = result_array[0]["delay"].as<int>();
        pin = result_array[0]["pin"].as<JsonArray>();
    }
    else
    {
        Serial.print("Error ");
        Serial.println(httpResponseCode);
    }
    Serial.println("----------------------------------");
}

void PUT_home()
{
    const String url = baseUrl + "/home";
    String json;
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(2048);
    doc["state"] = is_open;
    doc["house_name"] = house_name;
    serializeJson(doc, json);
    Serial.println(json);

    int httpResponseCode = http.PUT(json);
    if (httpResponseCode == 200)
    {
        Serial.print("Done");
        Serial.println();
    }
    else
    {
        Serial.print("Error ");
        Serial.println(httpResponseCode);
    }

    Serial.println("----------------------------------");
}
