
// For making an HTTPS request
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// For creating a webserver
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// For parsing JSON strings
#include <ArduinoJson.h>

// Getting access to a time client
#include <NTPClient.h>
#include <WiFiUdp.h>

// For driving the matrix display
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 15

// Setting up local time
#define EST -18000
const long utcOffsetInSeconds = EST;

// Root certificate for api-v3.mbta.com
const char IRG_Root_X1 [] PROGMEM = R"CERT(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)CERT";

// SSID and password for local router
const char* ssid = "TP-Link_51CA";
const char* password = "password";

// Objects for getting time from time client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Create a list of certificates with the server certificate
X509List cert(IRG_Root_X1);


// URL and key for mbta api server
String api_url = "https://api-v3.mbta.com/predictions?filter[stop]=9147&filter[route]=134&include=stop,trip";
String api_key = "033c7fd2b648432a84701196c7b94526";

// Server object
AsyncWebServer server(80);

// Set up the matrix display
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(7, 7, PIXEL_PIN,
  NEO_MATRIX_BOTTOM    + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);


// Global time variables displayed on web server
uint8_t hrs, mns, scs;
bool daylightsavings = false;
int16_t arrival_minutes;

// Required for web server
const char* DST_FORM = "dst";
bool newRequest = false;
String textdisplay;

// Processing variables on web server
String processor(const String& var) {
  
  const char* dstState[] = {"EST", "EDT"};

  // Display EST or EDT
  if (var == "SET_DST") {
    return dstState[daylightsavings];
  }

  // Display current time
  if (var == "TIME") {
    char buf[12];
    sprintf(buf, "%02d:%02d:%02d", hrs, mns, scs);
    return buf;
  }

  // Display destination and arrival time
  if (var == "DESTINATION") {
    return textdisplay;
  }
  return String();
}

// Converts hours and minutes into total minutes
uint16_t total_minutes(uint8_t hrs, uint8_t mns) {
  return (hrs * 60) + mns;
};

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  timeClient.begin();

  // Set up local file system access
  if (!SPIFFS.begin()) {
    Serial.println("Failed to initialize SPIFFS");
    return;
  }
  
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
   // request->send(SPIFFS, "/index.html", "text/html");
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/styles.css", "text/css");
  });

 // Process input from the web form
  server.on("/", HTTP_POST, [] (AsyncWebServerRequest * request) {

   int params = request->params();
   for (int i = 0; i < params; i++) {
      AsyncWebParameter* p = request->getParam(i);
      if (p->isPost()) {
        if (p->name() == DST_FORM) {
          String dst = p->value().c_str();
          daylightsavings = dst.toInt();
          //Serial.println(daylightsavings);
        }
        newRequest = true;
      }
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.begin();
  Serial.println("HTTP server started");

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(matrix.Color(255,0,0));
}

void loop() {

  uint8_t len;
  
  // Get time
  timeClient.update();
  hrs = timeClient.getHours();
  mns = timeClient.getMinutes();
  scs = timeClient.getSeconds();

  if (daylightsavings == true) {
    hrs +=1;
  }

  // Convert now hours minutes to minutes
  //uint16_t now_minutes = total_minutes(timeClient.getHours(), timeClient.getMinutes());
   uint16_t now_minutes = total_minutes(hrs, mns);

  if ((WiFi.status() == WL_CONNECTED)) {
      WiFiClientSecure client;
      HTTPClient https;
      client.setTrustAnchors(&cert);
      if(https.begin(client, api_url+"&api_key="+api_key)){
          int httpCode = https.GET();
          if (httpCode >0) {
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                  String payload = https.getString();
                  DynamicJsonDocument doc(14000);
                  DeserializationError error = deserializeJson(doc, payload);
                  if (error) {
                    Serial.print("Error parsing JSON: ");
                    Serial.println(error.c_str());
                  } else {
                
                    String head_sign = doc["included"][0]["attributes"]["headsign"];
                    String arrival_time = doc["data"][0]["attributes"]["arrival_time"];
                    String hours = arrival_time.substring(11,13);
                    String minutes = arrival_time.substring(14,16);

                    // Convert arrival hours minutes to minutes
                    arrival_minutes = (total_minutes(hours.toInt(), minutes.toInt()) - now_minutes);

                    if (arrival_minutes < 0) arrival_minutes = 0;
                    
                    Serial.printf("Local time: %s\n", timeClient.getFormattedTime());
                    //String textdisplay = head_sign + "; " + String(arrival_minutes-now_minutes);
                    textdisplay = head_sign + ": " + String(arrival_minutes);
                
                    Serial.println(textdisplay);

                    // Gets length of text display
                    for( len = 0; textdisplay[len] != '\0'; len++);

                    // Prints text display on matrix
                    for (int16_t x = matrix.width(); x > -(len * 6); x--) {
                        matrix.fillScreen(0);
                        matrix.setCursor(x, 0);
                        matrix.print(textdisplay);
                        delay(50);
                        matrix.show();
                    }
                  }
              } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
              }
            https.end();
          } else {
        Serial.println("[HTTP] Unable to connect");
      }
    }
  }
  //delay(1000);
}
