#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <SSD1306Wire.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define APIKEY ""
#define STOCK_SYMBOL "GME"
#define EXCHANGE "NYSE"
#define INTERVAL 300000
#define SCREEN_WIDTH 64
#define DISPLAY_GEOMETRY GEOMETRY_64_48
#define DIGIT_COUNT 4
#define FONT_SIZE 10
#define FONT ArialMT_Plain_10
#define VALUE_COUNT 4

struct value {
  const char * title;
  const char * key;
};

value values[VALUE_COUNT] = {
  {title: "Price", key: "price"},
  {title: "Chg", key: "changesPercentage"},
  {title: "High", key: "dayHigh"},
  {title: "Low",  key: "dayLow"}
};

HTTPClient https;
std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
SSD1306Wire display(0x3c, SDA, SCL, DISPLAY_GEOMETRY);

void drawRow(int row, const char* title, double value) {
  char buffer [16] {};
  gcvt(value, DIGIT_COUNT, buffer);
  int offset = row * FONT_SIZE;
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, offset, title);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(SCREEN_WIDTH, offset, buffer);
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

  StaticJsonDocument<1000> data;
  DeserializationError error = deserializeJson(data, response);
  if (error) {
    Serial.println("Failed to deseiralice JSON:");
    Serial.println(response);
    return false;
  }

  Serial.println("Response: ");
  serializeJsonPretty(data, Serial);
  Serial.println();

  for (JsonVariant item : data.as<JsonArray>()) {
    String exchange = item["exchange"];
    if (exchange == EXCHANGE) {
      display.clear();
      for (int i = 0; i < VALUE_COUNT; i++) {
        drawRow(i, values[i].title, item[values[i].key]);
      }
      display.display();
      return true;
    }
  }

  return false;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  client->setInsecure();
  display.init();
  display.setFont(FONT);
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
