/*
set for departure time
link ids for buses
*/

#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// For driving the matrix display
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 10

#define BUSID 0
#define BUSTIME 1

const char* ssid = "TP-Link_51CA";
const char* password = "password";
//const char* ssid = "timesink2";
//const char* password = "sweetpotato";

// Setting up local time
#define EST -18000
const long utcOffsetInSeconds = EST;

// URL and key for mbta api server
String api_url = "https://api-v3.mbta.com/predictions?filter[stop]=9147&filter[route]=134&include=stop,trip";
String api_key = "033c7fd2b648432a84701196c7b94526";

//const char* server = "https://api-v3.mbta.com/predictions?filter[stop]=9147&filter[route]=134&include=stop,trip";  // Server URL

const char* rootCACertificate =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
  "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
  "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
  "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
  "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
  "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
  "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
  "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
  "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
  "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
  "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
  "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
  "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
  "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
  "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
  "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
  "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
  "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
  "-----END CERTIFICATE-----\n";

// Objects for getting time from time client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

AsyncWebServer server(80);

// Set up the matrix display
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(7, 7, PIXEL_PIN, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800);

//String formattedDate;
// Global time variables displayed on web server
uint8_t hrs, mns, scs;
bool daylightsavings = false;
//int16_t arrival_minutes[5];
int32_t bus_time_id[5][2];
int32_t bus_head_id[5];

String textdisplay;

// Converts hours and minutes into total minutes
uint16_t total_minutes(uint8_t hrs, uint8_t mns) {
  return (hrs * 60) + mns;
};

void setup() {
  Serial.begin(115200);

  //Connect to Wi-Fi
  //WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(matrix.Color(255, 0, 0));
}

void loop() {

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  hrs = timeClient.getHours();
  mns = timeClient.getMinutes();
  scs = timeClient.getSeconds();

  uint16_t now_minutes = total_minutes(hrs, mns);

  String formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  HTTPClient https;

  if (https.begin(client, api_url + "&api_key=" + api_key)) {

    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS]: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(14000);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          Serial.print("Error parsing JSON: ");
          Serial.println(error.c_str());
          //   String error_message = "
        } else {

          uint8_t x = 0;
          do {
            //Check for departure since arrival might be void if the bus is not going to drop off
            String departure_time = doc["data"][x]["attributes"]["departure_time"];
            String hours = departure_time.substring(11, 13);
            String minutes = departure_time.substring(14, 16);
            // Time till next bus arrives in minutes
            int16_t arrival_minutes = total_minutes(hours.toInt(), minutes.toInt()) - now_minutes;
            // Store arrival time
            bus_time_id[x][BUSTIME] = arrival_minutes;

            // ID of bus arriving in arrival_minutes
            String trip_id = doc["data"][x]["relationships"]["trip"]["data"]["id"];
            // Store ID
            bus_time_id[x][BUSID] = trip_id.toInt();


          } while (bus_time_id[x++][BUSID] != 0);  // Check to see if bus ID is zero indicating no more busses scheduled

          //Match IDs from arrival times with IDs from head signs
          for (int i = 0; i < x; i++) {
            for (int j = 0; j < x; j++) {
              String id = doc["included"][j]["id"];
              if (bus_time_id[i][BUSID] == id.toInt()) {
                String head = doc["included"][j]["attributes"]["headsign"];
                const char* a = head.c_str();  //Convert String to C-string required for printf
                Serial.printf("IDT:%d TIME:%d IDH:%d %s\n", bus_time_id[i][BUSID], bus_time_id[i][BUSTIME], bus_head_id[j], a);
              }
            }
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
    Serial.println();
    delay(3000);
  }
}
