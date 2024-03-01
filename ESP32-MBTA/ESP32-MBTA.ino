#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
// For parsing JSON strings
#include <ArduinoJson.h>

const char* ssid = "TP-Link_51CA";
const char* password = "password";

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

//WiFiClientSecure client;

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
}

void loop() {
  //WiFiClientSecure client;
  WiFiClientSecure* client = new WiFiClientSecure;
  HTTPClient https;
  //client.setTrustAnchors(&cert);
   client->setCACert(rootCACertificate);
  if (https.begin(client, api_url + "&api_key=" + api_key)) {
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        DynamicJsonDocument doc(14000);
        DeserializationError error = deserializeJson(doc, payload);
        Serial.printf("payload: %s\n", payload);
        if (error) {
          Serial.print("Error parsing JSON: ");
          Serial.println(error.c_str());
          //   String error_message = "
        } else {

          String head_sign = doc["included"][1]["attributes"]["headsign"];
          String arrival_time = doc["data"][1]["attributes"]["arrival_time"];
          String hours = arrival_time.substring(11, 13);
          String minutes = arrival_time.substring(14, 16);

          // Convert arrival hours minutes to minutes
          //arrival_minutes = (total_minutes(hours.toInt(), minutes.toInt()) - now_minutes);

          //// Account for rounding errors and better to over compensate than under
        .//  arrival_minutes -= 1;

          // MBTA api sometimes predicts negative arrival values.
         // if (arrival_minutes < 0) arrival_minutes = 0;

          Serial.printf("Local time: %s\n", timeClient.getFormattedTime());
          //String textdisplay = head_sign + "; " + String(arrival_minutes-now_minutes);
          //textdisplay = head_sign + ": " + String(arrival_minutes);

          Serial.println(textdisplay);

          // Gets length of text display
          for (len = 0; textdisplay[len] != '\0'; len++)
            ;

          // Prints text display on matrix
         // for (int16_t x = matrix.width(); x > -(len * 6); x--) {
         //   matrix.fillScreen(0);
         //   matrix.setCursor(x, 0);
         //   matrix.print(textdisplay);
          //  delay(50);
           // matrix.show();
          //}
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
/*
  WiFiClientSecure* client = new WiFiClientSecure;
  if (client) {
    // set secure client with certificate
    client->setCACert(rootCACertificate);
    //create an HTTPClient instance
    HTTPClient https;
    Serial.println(api_url + "&api_key=" + api_key);

    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    //if (https.begin(*client,  api_url+"&api_key="+api_key)) {  // HTTPS
    if (https.begin(*client, api_url)) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          // print server response payload
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    }
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  Serial.println();
  Serial.println("Waiting 2min before the next round...");
  delay(12000);*/
}
