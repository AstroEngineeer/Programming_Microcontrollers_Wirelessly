#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WiFiUdp.h>
#include <string>
#include <Wire.h>
#include "hex_string.cpp"

// prototypes
boolean connectWifi();
void SendHex();
void kitchenLightsOn();

AsyncWebServer server(80);

const char *ssid = "GAVV";         // enter your ssid/ wi-fi(case sensitive) router name - 2.4 Ghz only
const char *password = "Tokyo123"; // enter ssid password (case sensitive)

// change gpio pins as you need it.
const int relayPin1 = 16;
const int relayPin2 = 4;

//#######################################
boolean wifiConnected = false;

void setup()
{
  Serial.begin(115200);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  // Initialise wifi connection
  wifiConnected = connectWifi();

  if (wifiConnected)
  {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Hi! I am ESP8266."); });

    AsyncElegantOTA.begin(&server); // Start ElegantOTA
    server.begin();
    Serial.println("HTTP server started");
  }

  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
}

void loop()
{
  if (wifiConnected)
  {
    Serial.print("Looping.....");
    delay(15000);
    SendHex();
    delay(1800000);
    // upnpBroadcastResponder.serverLoop();
    // kitchen->serverLoop();
    // office->serverLoop();
  }
}

void SendHex()
{
  Serial.print("Sending Hex Code ...");
  Wire.beginTransmission(8);                                                                  /* begin with device address 8 */
  std::string s = std::string("{\"payload\":") + hex + std::string(",\"discription\":Hex File}");
  char* data;
  data = &s[0];
  Wire.write(data); /* sends hello string */
  Wire.endTransmission();                                                                     /* stop transmitting */
}

void kitchenLightsOn()
{
  Serial.print("Switch 2 turn on ...");
  Wire.beginTransmission(8);              /* begin with device address 8 */
  Wire.write("{\"gpio\":3,\"state\":1}"); /* sends hello string */
  Wire.endTransmission();                 /* stop transmitting */
}

// connect to wifi – returns true if successful or false if not
boolean connectWifi()
{
  boolean state = true;
  int i = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting ...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i > 10)
    {
      state = false;
      break;
    }
    i++;
  }

  if (state)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  return state;
}
