#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define APIKEY ""
#define STOCK_SYMBOL "GME"
#define INTERVAL 300000


HTTPClient https;
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
StaticJsonDocument<1000> data;

void makeRequest() {
  if (!https.begin(*client, "https://financialmodelingprep.com/api/v3/quote/" STOCK_SYMBOL "?apikey=" APIKEY)) {
    Serial.println("Failed to begin API request");
    return;
  }

  int status = https.GET();
  if (status != HTTP_CODE_OK) {
    Serial.print("API request failed: ");
    Serial.println(status);
    return;
  }

  String response = https.getString();
  https.end();

  DeserializationError error = deserializeJson(data, response);
  if (error) {
    Serial.println("Failed to deseiralice JSON:");
    Serial.println(response);
    return;
  }

  Serial.println("Response: ");
  serializeJsonPretty(data, Serial);
  Serial.println();

  float price = data[0]["price"];
  float open = data[0]["open"];
  float dayLow = data[0]["dayLow"];
  float dayHigh = data[0]["dayHigh"];

  Serial.print("Price: ");
  Serial.println(price);
  Serial.print("Open: ");
  Serial.println(open);
  Serial.print("Low: ");
  Serial.println(dayLow);
  Serial.print("High: ");
  Serial.println(dayHigh);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client->setInsecure();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    makeRequest();
    delay(INTERVAL);
  }
}
