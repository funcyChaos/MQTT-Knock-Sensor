#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include "secrets.h"

// MQTT settings
const unsigned long   PUBLISH_DELAY   = 1000;
const char*           DEVICE_ID       = "things_esp32_knocks";
const char*           EVENT_TOPIC     = "homeassistant/event/knock-alert";
WiFiClient espClient;
PubSubClient client(espClient);

// Knocks
const int             KNOCKPIN        = 4;
const unsigned long   MIN_GAP         = 150;
float                 baseline        = 0;
float                 envelope        = 0;
unsigned long         lastKnockTime   = 0;
bool                  isKnocking      = false;
int                   knockCount      = 0;

Adafruit_NeoPixel pixels(1, 10, NEO_GRB + NEO_KHZ800);
enum ColorChoice{
  GREEN,
  RED,
  BLUE
};
uint32_t colors[] = {
  pixels.Color(20,0,0),
  pixels.Color(0,20,0),
  pixels.Color(0,0,20)
};

void loop(){
  // Update OTA
  ArduinoOTA.handle();
  // MQTT stay connected
  if(!client.connected()){
    reconnect();
  }
  client.loop();

  int sensorReading = analogRead(KNOCKPIN);
  unsigned long now = millis();
  // static bool published = false;
  // This is where we're getting our primary baseline from.
  baseline = baseline * 0.999 + sensorReading * 0.001;
  // This is a high pass filter and absolute in one, which makes sure we're measuring positive numbers based from 0.
  float signal = abs(sensorReading - baseline);
  //  This is also called an integrator or low pass filter and it kind of keeps tabs on what we've been getting.
  envelope = envelope * 0.7 + signal * 0.3;

  if(envelope > 15 && !isKnocking && (now - lastKnockTime > MIN_GAP)){
    isKnocking = true;
    knockCount++;
    lastKnockTime = now;
    set_led(RED);

    // debug
    // Serial.print("knockCount: ");
    // Serial.println(knockCount);
    // Serial.print("envelope: ");
    // Serial.println(envelope);
  }
  if(isKnocking && envelope < 8){
    isKnocking = false;
  }
  if(now - lastKnockTime > MIN_GAP){
    set_led(GREEN);
  }

  if(knockCount > 0 && (now - lastKnockTime > PUBLISH_DELAY)){
    String payload =  "{"
                      "\"event\": \"knock-alert\","
                      "\"knocks\": " + String(knockCount) + ","
                      "\"device\": \"" + String(DEVICE_ID) + "\""
                      "}";
    client.publish(EVENT_TOPIC, payload.c_str(), false);
    knockCount = 0;
  }
}

void setup(){
  setup_wifi();

  client.setServer(MQTT_SERVER, 1883);

  pixels.begin();
  pixels.setBrightness(50);
  set_led(GREEN);

  for(int i=0; i<100; i++){
    baseline = baseline * 0.9 + analogRead(KNOCKPIN) * 0.1;
  }

    // Start OTA
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    // Serial.println("Start updating " + type);
  });
  // ArduinoOTA.onEnd([]() {
  //   Serial.println("\nEnd");
  // });
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  // });
  // ArduinoOTA.onError([](ota_error_t error) {
  //   Serial.printf("Error[%u]: ", error);
  //   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  //   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  //   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  //   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  //   else if (error == OTA_END_ERROR) Serial.println("End Failed");
  // });

  ArduinoOTA.begin();
}

void set_led(ColorChoice color){
  pixels.setPixelColor(0, colors[color]);
  pixels.show();
}

void setup_wifi(){
  // Serial.print("Connecting to WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    // Serial.print(".");
  }
  // Serial.println(" connected!");
}

void reconnect(){
  static unsigned long lastAttempt = 0;
  unsigned long now = millis();
  if(client.connected()) return;
  if(now - lastAttempt < 2000) return;

  lastAttempt = now;
  client.connect(DEVICE_ID, MQTT_USER, MQTT_PASS);
}