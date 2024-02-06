
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define EST -18000
#define EDT_OFFSET    1

// Root certificate for howsmyssl.com
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

// Replace with your network credentials
const char* ssid = "timesink2";
const char* password = "sweetpotato";

const long utcOffsetInSeconds = EST;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Create a list of certificates with the server certificate
X509List cert(IRG_Root_X1);

String api_url = "https://api-v3.mbta.com/predictions?filter[stop]=9147&filter[route]=134&include=stop,trip";
String api_key = "033c7fd2b648432a84701196c7b94526";

String displaybuffer[200];

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
}

//2024-02-05T17:46:48-05:00
void loop() {
    timeClient.update();
    //uint8_t hrs = timeClient.getHours();
    //uint8_t mns = timeClient.getMinutes();
    //uint8_t scs = timeClient.getSeconds();
   // uint16_t now_minutes = total_minutes(hrs, mns);
    uint16_t now_minutes = total_minutes(timeClient.getHours(), timeClient.getMinutes());
     //Serial.println(timeClient.getFormattedTime());

    //Serial.printf("%02d:%02d:%02d\n", hrs, mns, scs);
  
  if ((WiFi.status() == WL_CONNECTED)) {
      WiFiClientSecure client;
      HTTPClient https;
      client.setTrustAnchors(&cert);
      if(https.begin(client, api_url+"&api_key="+api_key)){
          int httpCode = https.GET();
          if (httpCode >0) {
              if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                  String payload = https.getString();
                  //Serial.println("Response: " + payload);
                  DynamicJsonDocument doc(14000);
                  DeserializationError error = deserializeJson(doc, payload);
                  if (error) {
                    Serial.print("Error parsing JSON: ");
                    Serial.println(error.c_str());
                  } else {
                   /// DynamicJsonDocument buses(2000);
                    //buses["arrival"] = doc["data"][0]["attributes"]["arrival_time"];
                    //buses["id"] = doc["data"][0]["relationships"]["trip"]["data"]["id"];
                    //buses["head"] =  doc["included"][0]["attributes"]["headsign"];
                    String head_sign = doc["included"][0]["attributes"]["headsign"];
                    String arrival_time = doc["data"][0]["attributes"]["arrival_time"];
                    String hours = arrival_time.substring(11,13);
                    String minutes = arrival_time.substring(14,16);
                    uint16_t arrival_minutes = total_minutes(hours.toInt(), minutes.toInt());
                    Serial.printf("Arrival Minutes: %d\n", arrival_minutes);
                    Serial.printf("Present Minutes: %d\n", now_minutes);
                    Serial.printf("Time to arrival: %d\n", arrival_minutes - now_minutes);
                    Serial.printf("Where: %s\n",head_sign);
                    //displaybuffer = doc["included"][1]["attributes"]["headsign"] + ":" + doc["data"][0]["attributes"]["arrival_time"];
                   //String output;
                   // serializeJson(buses, output);
                   // Serial.println(output);
                   // Serial.println(arrival_time);
                    //Serial.printf("time: %s\n", hours);
                  
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

  delay(6000);

}

 /*   if (https.begin(client, url+"&api_key="+api_key)) {

    https.begin(client, url+"&api_key="+api_key);
      int httpCode = https.GET();
 payload = https.getString();
          Serial.println(payload);
          
      // httpCode will be negative on error
     // if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
      //  Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
       // if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          
      //  }
      //}else {
      //  Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
     // }
      
      JsonObject& root = jsonBuffer.parseObject(payload);
      
      //JSONVar myObject = JSON.parse(payload);
      // JSON.typeof(jsonVar) can be used to get the type of the var
     // if (JSON.typeof(myObject) == "undefined") {
      //  Serial.println("Parsing input failed!");
      //  return;
     // }
     // Serial.print("JSON object = ");
     // Serial.println(myObject);
      for (int x = 0; x < 5; x++) {
        Serial.print(x);
        bus_time["arrival"] = myObject["data"][x]["attributes"]["arrival_time"];
        bus_time["id"] = myObject["data"][x]["relationships"]["trip"]["data"]["id"];
        String bustime = myObject["data"][x]["attributes"]["arrival_time"];
        String busid = myObject["data"][x]["relationships"]["trip"]["data"]["id"];
        
      //Serial.println(myObject["data"][0]["attributes"]["arrival_time"]);
      //Serial.print("Departure Time2: ");
      //Serial.println(myObject["data"][1]["attributes"]["arrival_time"]);
     // }
      
      
      https.end();
   // } else {
   //   Serial.printf("[HTTPS] Unable to connect\n");
   // }
  }*/
 // }
 // Serial.println();
 // Serial.println("Waiting 2min before the next round...");
 // delay(10000);
  
//}
