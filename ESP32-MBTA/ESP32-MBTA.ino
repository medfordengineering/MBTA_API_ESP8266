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

#define PIXEL_PIN 6
//#define PIXEL_PIN 21

// Definitions for
#define BUSID 0
#define BUSTIME 1
#define MAXNUM_BUSES 8

// Definitions for setting DST
#define MAR 3
#define NOV 11
#define SUN 0
#define WEEK 7
#define NOTSET 3
#define MAGIC_NUMBER 8

// TEST JSON STRINGS

//String payload = "{\"data\":[{\"attributes\":{\"arrival_time\":\"2024-04-03T17:40:07-04:00\",\"arrival_uncertainty\":null,\"departure_time\":\"2024-04-03T17:40:07-04:00\",\"departure_uncertainty\":null,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":43},\"id\":\"prediction-60314919-9147-43\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314919\",\"type\":\"trip\"}},\"vehicle\":{\"data\":{\"id\":\"y0859\",\"type\":\"vehicle\"}}},\"type\":\"prediction\"}],\"included\":[{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F134-78\",\"direction_id\":1,\"headsign\":\"Wellington\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314919\",\"links\":{\"self\":\"/trips/60314919\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-3-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340357\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"address\":null,\"at_street\":\"IvyRoad\",\"description\":null,\"latitude\":42.42737,\"location_type\":0,\"longitude\":-71.12592,\"municipality\":\"Medford\",\"name\":\"WinthropStBrooksSt-MedfordHS\",\"on_street\":\"WinthropStreet\",\"platform_code\":null,\"platform_name\":null,\"vehicle_type\":3,\"wheelchair_boarding\":1},\"id\":\"9147\",\"links\":{\"self\":\"/stops/9147\"},\"relationships\":{\"facilities\":{\"links\":{\"related\":\"/facilities/?filter[stop]=9147\"}},\"parent_station\":{\"data\":null},\"zone\":{\"data\":{\"id\":\"LocalBus\",\"type\":\"zone\"}}},\"type\":\"stop\"}],\"jsonapi\":{\"version\":\"1.0\"}}";

//String payload = "{\"data\":[{\"attributes\":{\"arrival_time\":null,\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:30:00-04:00\",\"departure_uncertainty\":300,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":1},\"id\":\"prediction-60314982-9147-1\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314982\",\"type\":\"trip\"}},\"vehicle\":{\"data\":null}},\"type\":\"prediction\"},{\"attributes\":{\"arrival_time\":null,\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:25:00-04:00\",\"departure_uncertainty\":null,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":1},\"id\":\"prediction-60314952-9147-1\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314952\",\"type\":\"trip\"}},\"vehicle\":{\"data\":{\"id\":\"y2022\",\"type\":\"vehicle\"}}},\"type\":\"prediction\"},{\"attributes\":{\"arrival_time\":\"2024-03-14T14:25:19-04:00\",\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:25:19-04:00\",\"departure_uncertainty\":null,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":43},\"id\":\"prediction-60312563-9147-43\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60312563\",\"type\":\"trip\"}},\"vehicle\":{\"data\":{\"id\":\"y2006\",\"type\":\"vehicle\"}}},\"type\":\"prediction\"}],\"included\":[{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"G134-180\",\"direction_id\":1,\"headsign\":\"Wellington\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60312563\",\"links\":{\"self\":\"/trips/60312563\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-3-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340357\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F100-27\",\"direction_id\":1,\"headsign\":\"MiddlesexAve&SecondSt\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314952\",\"links\":{\"self\":\"/trips/60314952\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-4-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340351\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F137-91\",\"direction_id\":1,\"headsign\":\"MiddlesexAve&SecondSt\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314982\",\"links\":{\"self\":\"/trips/60314982\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-4-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340351\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"address\":null,\"at_street\":\"IvyRoad\",\"description\":null,\"latitude\":42.42737,\"location_type\":0,\"longitude\":-71.12592,\"municipality\":\"Medford\",\"name\":\"WinthropSt@BrooksSt-MedfordHS\",\"on_street\":\"WinthropStreet\",\"platform_code\":null,\"platform_name\":null,\"vehicle_type\":3,\"wheelchair_boarding\":1},\"id\":\"9147\",\"links\":{\"self\":\"/stops/9147\"},\"relationships\":{\"facilities\":{\"links\":{\"related\":\"/facilities/?filter[stop]=9147\"}},\"parent_station\":{\"data\":null},\"zone\":{\"data\":{\"id\":\"LocalBus\",\"type\":\"zone\"}}},\"type\":\"stop\"}],\"jsonapi\":{\"version\":\"1.0\"}}";

//String payload = "{\"data\":[{\"attributes\":{\"arrival_time\":null,\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:30:00-04:00\",\"departure_uncertainty\":300,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":1},\"id\":\"prediction-60314982-9147-1\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314982\",\"type\":\"trip\"}},\"vehicle\":{\"data\":null}},\"type\":\"prediction\"},{\"attributes\":{\"arrival_time\":null,\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:45:00-04:00\",\"departure_uncertainty\":300,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":1},\"id\":\"prediction-60314968-9147-1\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314968\",\"type\":\"trip\"}},\"vehicle\":{\"data\":null}},\"type\":\"prediction\"},{\"attributes\":{\"arrival_time\":null,\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:25:00-04:00\",\"departure_uncertainty\":null,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":1},\"id\":\"prediction-60314952-9147-1\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314952\",\"type\":\"trip\"}},\"vehicle\":{\"data\":{\"id\":\"y2022\",\"type\":\"vehicle\"}}},\"type\":\"prediction\"},{\"attributes\":{\"arrival_time\":\"2024-03-14T14:27:41-04:00\",\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T14:27:41-04:00\",\"departure_uncertainty\":null,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":43},\"id\":\"prediction-60312563-9147-43\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60312563\",\"type\":\"trip\"}},\"vehicle\":{\"data\":{\"id\":\"y2006\",\"type\":\"vehicle\"}}},\"type\":\"prediction\"},{\"attributes\":{\"arrival_time\":\"2024-03-14T15:34:43-04:00\",\"arrival_uncertainty\":null,\"departure_time\":\"2024-03-14T15:34:43-04:00\",\"departure_uncertainty\":null,\"direction_id\":1,\"revenue\":\"REVENUE\",\"schedule_relationship\":null,\"status\":null,\"stop_sequence\":43},\"id\":\"prediction-60314891-9147-43\",\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"stop\":{\"data\":{\"id\":\"9147\",\"type\":\"stop\"}},\"trip\":{\"data\":{\"id\":\"60314891\",\"type\":\"trip\"}},\"vehicle\":{\"data\":{\"id\":\"y0875\",\"type\":\"vehicle\"}}},\"type\":\"prediction\"}],\"included\":[{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"G134-180\",\"direction_id\":1,\"headsign\":\"Wellington\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60312563\",\"links\":{\"self\":\"/trips/60312563\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-3-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340357\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F134-78\",\"direction_id\":1,\"headsign\":\"Wellington\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314891\",\"links\":{\"self\":\"/trips/60314891\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-3-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340357\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F100-27\",\"direction_id\":1,\"headsign\":\"MiddlesexAve&SecondSt\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314952\",\"links\":{\"self\":\"/trips/60314952\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-4-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340351\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F131-72\",\"direction_id\":1,\"headsign\":\"MiddlesexAve&SecondSt\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314968\",\"links\":{\"self\":\"/trips/60314968\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-4-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340351\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"bikes_allowed\":1,\"block_id\":\"F137-91\",\"direction_id\":1,\"headsign\":\"MiddlesexAve&SecondSt\",\"name\":\"\",\"revenue\":\"REVENUE\",\"wheelchair_accessible\":1},\"id\":\"60314982\",\"links\":{\"self\":\"/trips/60314982\"},\"relationships\":{\"route\":{\"data\":{\"id\":\"134\",\"type\":\"route\"}},\"route_pattern\":{\"data\":{\"id\":\"134-4-1\",\"type\":\"route_pattern\"}},\"service\":{\"data\":{\"id\":\"WinterWeekday\",\"type\":\"service\"}},\"shape\":{\"data\":{\"id\":\"1340351\",\"type\":\"shape\"}}},\"type\":\"trip\"},{\"attributes\":{\"address\":null,\"at_street\":\"IvyRoad\",\"description\":null,\"latitude\":42.42737,\"location_type\":0,\"longitude\":-71.12592,\"municipality\":\"Medford\",\"name\":\"WinthropSt@BrooksSt-MedfordHS\",\"on_street\":\"WinthropStreet\",\"platform_code\":null,\"platform_name\":null,\"vehicle_type\":3,\"wheelchair_boarding\":1},\"id\":\"9147\",\"links\":{\"self\":\"/stops/9147\"},\"relationships\":{\"facilities\":{\"links\":{\"related\":\"/facilities/?filter[stop]=9147\"}},\"parent_station\":{\"data\":null},\"zone\":{\"data\":{\"id\":\"LocalBus\",\"type\":\"zone\"}}},\"type\":\"stop\"}],\"jsonapi\":{\"version\":\"1.0\"}}";

//const char* ssid = "TP-Link_51CA";
const char* ssid = "EngineeringSubNet";
const char* password = "password";
//const char* ssid = "timesink2";
//const char* password = "sweetpotato";

// Setting up local time
#define EST -18000
#define EDT -14400
//#define EDT_OFFSET 1
const long utcOffsetInSeconds = EST;
bool dst_state = false;
//uint8_t dst_state = NOTSET;

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

//AsyncWebServer server(80);

// Set up the matrix display
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(7, 7, PIXEL_PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
                                               NEO_GRB + NEO_KHZ800);

//String formattedDate;
// Global time variables displayed on web server
uint8_t hrs, mns, scs;
//bool daylightsavings = false;
//int16_t arrival_minutes[5];
int32_t bus_time_id[MAXNUM_BUSES][2];
int32_t bus_head_id[MAXNUM_BUSES];

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

  uint8_t len;

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  hrs = timeClient.getHours();
  mns = timeClient.getMinutes();
  scs = timeClient.getSeconds();

  // Day of week, month and day
  uint8_t dayofweek = timeClient.getDay();
  String formattedDate = timeClient.getFormattedDate();
  uint8_t splitDash = formattedDate.indexOf("-") + 1;
  String mnt = formattedDate.substring(splitDash, splitDash + 2);
  splitDash += 3;
  String dte = formattedDate.substring(splitDash, splitDash + 2);

  uint8_t month = mnt.toInt();
  uint8_t day = dte.toInt();

  uint8_t previousSunday = day - dayofweek;

  if (((month > MAR) && (month < NOV)) || ((month == MAR) && (previousSunday >= MAGIC_NUMBER)) || ((month == MAR) && (day > WEEK * 2)) || ((month == NOV) && (previousSunday < 1))) {
    if (dst_state == true) {
      timeClient.setTimeOffset(EDT);
      dst_state = false;
    }
  } else {
    if (dst_state == false) {
      timeClient.setTimeOffset(EST);
      dst_state = true;
    }
  }
  // if (dst_state == true) hrs += EDT_OFFSET;

  uint16_t now_minutes = total_minutes(hrs, mns);

  // String formattedDate = timeClient.getFormattedDate();
  //Serial.println(formattedDate);

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

        // COMMENT OUT IF USING TEST JSON STRINGS
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
                //Serial.printf("IDT:%d TIME:%d IDH:%d %s\n", bus_time_id[i][BUSID], bus_time_id[i][BUSTIME], bus_head_id[j], a);
                Serial.printf("IDT:%d TIME:%d IDH:%d %s\n", bus_time_id[i][BUSID], bus_time_id[i][BUSTIME], id.toInt(), a);
                textdisplay = head + ':' + String(bus_time_id[i][BUSTIME]);
                // Gets length of text display
                for (len = 0; textdisplay[len] != '\0'; len++)
                  ;

                // Prints text display on matrix
                for (int16_t x = matrix.width(); x > -(len * 6); x--) {
                  //for (int16_t x = 34; x > 0; x--) {
                  matrix.fillScreen(0);
                  matrix.setCursor(x, 0);
                  //matrix.print(textdisplay);
                  //matrix.print("Wel");
                  matrix.print(textdisplay);
                  delay(50);
                  matrix.show();
                }
                delay(1000);
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
    //delay(3000);
  }
}
