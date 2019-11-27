#include <WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient espClient;
PubSubClient client(espClient);
IPAddress server(10, 42, 0, 1);

void setup() {
  Serial.begin(115200);

  WiFiManager wifiManager;
  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("WifiManager", "password");
  client.setServer(server, 1883);
  client.setCallback(callback);
  // Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
  Wire.begin(5, 4);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.display();
  delay(500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

String removeChar(String str) {
  for (int i = 0; i < str.length(); ++i) {
    if (str[i] >= 65 && str[i] <= 122)
      str[i] = ' ';
  }
  str.trim();
  for (int i = 0; i < str.length(); ++i) {
    if (str[i] >= 48 && str[i] <= 57)
      break;
    if (str[i] == 44) {
      str[i] = ' ';
      str.trim();
      break;
    }
  }
  if (str.length() > 8)
    str = str.substring(0, 8);
  return str;
}

void show(char* topic, char* payload) {
  String msg(payload);
  msg = removeChar(msg);
  if (strcmp(topic, "/c0/temp") ==  0) {
    display.setCursor(0, 0);
    display.fillRect(0, 0, 128, 16, SSD1306_BLACK);
    msg = "t=" + msg + "C";
    display.print(msg);
  } else if (strcmp(topic, "/c0/eng") ==  0) {
    display.setCursor(0, 16);
    display.fillRect(0, 16, 128, 16, SSD1306_BLACK);
    msg = "m=" + msg;
    display.print(msg);
  } else if (strcmp(topic, "/c0/servo") ==  0) {
    display.setCursor(0, 32);
    display.fillRect(0, 32, 128, 16, SSD1306_BLACK);
    msg = "v=" + msg;
    display.print(msg);
  } else if (strcmp(topic, "/c0/ir") ==  0) {
    display.setCursor(0, 48);
    display.print("Colidiu!");
    display.display();
    delay(1000);
    display.fillRect(0, 48, 128, 16, SSD1306_BLACK);
  } else {
    Serial.print("Topico nao identificado");
  }
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[strlen((char*)payload)] = '\0';
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  Serial.print((char*)payload);
  Serial.println();
  Serial.print(strlen((char*)payload));
  Serial.println();
  show(topic, (char*)payload);
  for (int i = 0; i < strlen((char*)payload); ++i) {
    payload[i] = '\0';
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("/c0/temp");
      client.subscribe("/c0/ir");
      client.subscribe("/c0/eng");
      client.subscribe("/c0/servo");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 0.5 seconds");
      // Wait 0.5 seconds before retrying
      delay(500);
    }
  }
}
