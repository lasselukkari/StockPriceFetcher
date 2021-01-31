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

void makeRequest() {
  if (!https.begin(*client, "https://financialmodelingprep.com/api/v3/quote/" STOCK_SYMBOL "?apikey=" APIKEY)) {
    Serial.println("Failed to begin API request");
    return;
  }

  if ( https.GET() != HTTP_CODE_OK) {
    Serial.println("API request failed");
    return;
  }

  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, https.getString());
  if (error) {
    Serial.println("Failed to deseiralice JSON");
    return;
  }

  // serializeJsonPretty(doc, Serial);

  float price = doc[0]["price"];
  Serial.println(price);

  https.end();
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
