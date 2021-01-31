#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <SSD1306Wire.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define APIKEY ""
#define STOCK_SYMBOL "GME"
#define INTERVAL 300000

HTTPClient https;
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
StaticJsonDocument<1000> data;
SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_64_48);
char buffer [33] {};

void displayData(float price, float change, float dayLow, float dayHigh) {
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  gcvt(price, 4, buffer);
  display.drawString(0, 0, "Price");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 0, buffer);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  gcvt(change, 4, buffer);
  display.drawString(0, 10, "Chg");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 10, buffer);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  gcvt(dayHigh, 4, buffer);
  display.drawString(0, 20, "High");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 20, buffer);

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  gcvt(dayLow, 4, buffer);
  display.drawString(0, 30, "Low");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(64, 30, buffer);

  display.display();
}

bool update() {
  if (!https.begin(*client, "https://financialmodelingprep.com/api/v3/quote/" STOCK_SYMBOL "?apikey=" APIKEY)) {
    Serial.println("Failed to begin API request");
    return false;
  }

  int status = https.GET();
  if (status != HTTP_CODE_OK) {
    Serial.print("API request failed: ");
    Serial.println(status);
    return false;
  }

  String response = https.getString();
  https.end();

  DeserializationError error = deserializeJson(data, response);
  if (error) {
    Serial.println("Failed to deseiralice JSON:");
    Serial.println(response);
    return false;
  }

  Serial.println("Response: ");
  serializeJsonPretty(data, Serial);
  Serial.println();

  float price = data[0]["price"];
  float change = data[0]["changesPercentage"];
  float dayLow = data[0]["dayLow"];
  float dayHigh = data[0]["dayHigh"];

  displayData(price, change, dayLow, dayHigh);
  return true;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client->setInsecure();
  display.init();
  display.setFont(ArialMT_Plain_10);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!update()) {
      display.clear();
      display.drawString(0, 0, "Error!");
      display.display();
    }
    delay(INTERVAL);
  } else {
    display.clear();
    display.drawString(0, 0, "No WiFi!");
    display.display();
    delay(1000);
  }
}
