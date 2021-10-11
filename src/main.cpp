#include <WiFiManager.h>
WiFiManager wm;

#define WEBSERVER_H
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <WebSerial.h>
#include <WiFi.h>
#include <analogWrite.h>
#include <ota.h>

AsyncWebServer server(80);

void sendSlackMessage() {
  String url = "";
  String format =
      R"rawliteral({"channel": "#feeder", "username": "feederBot", "text": "%s %s"}
)rawliteral";
  char reqData[100];
  String macAddr = WiFi.macAddress();
  String localIP = WiFi.localIP().toString();
  sprintf(reqData, format.c_str(), localIP.c_str(), macAddr.c_str());

  Serial.println(reqData);
  HTTPClient http;

  http.begin(url);
  int httpCode = http.POST(String(reqData));

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed with code %d\n", httpCode);
    String payload = http.getString();
    Serial.println(payload);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <iframe name="dummyframe" id="dummyframe" style="display: none;"></iframe>
  <form action="/on" target="dummyframe" style="height: 49vh">
      <input type="submit" value="ON" style="display: block;height:100%; width:100%; font-size: 5rem;
          background-color: blue;">
  </form>
  <form action="/off" target="dummyframe" style="height: 49vh">
      <input type="submit" value="OFF"
          style="display: block;height:100%; width:100%; font-size: 5rem; background-color: red;">
  </form>
</body></html>)rawliteral";

void setup() {
  Serial.begin(115200);
  wm.setConfigPortalTimeout(30);
  wm.autoConnect();
  sendSlackMessage();
  WebSerial.begin(&server);
  server.begin();
  otaBegin();
  pinMode(26, OUTPUT);
  pinMode(2, OUTPUT);
  analogWriteFrequency(50);
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(2, HIGH);
    analogWrite(26, 150, 150);
    request->send(204);
  });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(2, LOW);
    analogWrite(26, 0, 150);
    request->send(204);
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  server.begin();
}

void loop() { ArduinoOTA.handle(); }