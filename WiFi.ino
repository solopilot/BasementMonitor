// WiFi.ino: implements a web server over WiFi

#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#define ELEGANTOTA_DEBUG 0
#define UPDATE_DEBUG 0

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <ElegantOTA.h>

// WiFi
const char* WiFiSSID = "cuegroup";
const char* WiFiPasswd = "beef15dead";
const char* hostname = "Basement";
bool dstEnable = true;              // daylight savings time
long long gmtOffset = 5*60*60;      // US EST

AsyncWebServer server(80);          // server on port 80
unsigned long ota_progress_millis = 0;

void onOTAStart() {  // Log when OTA has started
  Serial.println("OTA update started!");
}

void onOTAProgress(size_t current, size_t final) {  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {  // Log when OTA has finished
  if (success)
    Serial.println("OTA update finished successfully!");
  else
    Serial.println("There was an error during OTA update!");
}

void setupWiFi() {
  int delayCounter = 0;

  WiFi.hostname(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFiSSID, WiFiPasswd);
  while (WiFi.status() != WL_CONNECTED) {
    delayCounter += 1;
    // turn on blue LED to display WiFi state signature
    digitalWrite(LED, (delayCounter%2 == 0)? LOW : HIGH);
    delay(500);
  }
  digitalWrite(LED, HIGH);    // High is Off

  Serial.println();
  Serial.print("Server is at IP Address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
    MDNS.addService("_http", "_tcp", 80);
  }
  else
    Serial.println("Error setting up MDNS responder!");

  // set up NTP
  int dstOffset = (dstEnable)? 1*60*60 : 0;
  configTime(gmtOffset, dstOffset, "pool.ntp.org");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    printf("Web page request from %s\n", request->client()->remoteIP().toString().c_str());
    String s = "<!DOCTYPE HTML><html><head><title>Basement Monitor</title>"
      "<meta name=\"viewport\" content=\"width-device-width, initial-scale=1\">"
      "<meta http-equiv=\"refresh\" content=\"5\">"
      "</head><body><h2>Basement Monitor</h2>";
    s += "<p>" + getTimestamp() + "</p>";
    s += "<p>Temperature=" + String(temperature_AHT20F)+ " F</p><p>Humidity=";
    s += String(humidity) + " %</p><p>Pressure=" + String(pressureHg) + " inches Hg; ";
    s += String(pressure) + " hPa</p><p>" + waterMsg;
    s += "</p></body></html>";
    request->send(200, "text/html", s.c_str());
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA

  ElegantOTA.onStart(onOTAStart); 
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();
  Serial.println("HTTP server started");
}

// call on each loop
void loopWiFi() {
  MDNS.update();
  ElegantOTA.loop();
}
