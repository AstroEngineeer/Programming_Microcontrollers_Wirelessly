#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WiFiUdp.h>
#include <string>
#include <Wire.h>
#include "hex_string.cpp"

#define TIMEOUT 1000UL
#define RESETPIN 12

AsyncWebServer server(80);

const char *ssid = "internet";         // enter your ssid/ wi-fi(case sensitive) router name - 2.4 Ghz only
const char *password = "password"; // enter ssid password (case sensitive)

boolean wifiConnected = false;

uint16_t _addr;
uint32_t _hex_size;
uint32_t _start_time;
bool _is_timeout = false;

void flushRxBuff()
{
    while (Serial.available())
        Serial.read();
}

bool waitOptibootRes_1s()
{
    _start_time = millis();
    while ((Serial.available() < 2) && !(_is_timeout = ((millis() - _start_time) > TIMEOUT)))
        ;
    if (_is_timeout || (Serial.read() != 0x14) || (Serial.read() != 0x10))
    {
        // WARNING_STK_FAILED;
        return false;
    }

    return true;
}

bool getSync()
{
    Serial.write(0x30);
    Serial.write(0x20);
    return waitOptibootRes_1s();
}

bool sendHex_STK500(const uint8_t *hex, uint8_t len)
{
    // STK500 Protocol
    Serial.write(0x55);
    Serial.write(_addr & 0xFF);
    Serial.write(_addr >> 8);
    Serial.write(0x20);
    _hex_size -= len;

    if (waitOptibootRes_1s())
    {
        Serial.write(0x64);
        Serial.write(0x00);
        Serial.write(len);
        Serial.write(0x46);

        for (uint8_t i = 0; i < len; i++)
        {
            Serial.write(*(hex + i));
        }

        Serial.write(0x20);
    }
    else
    {
        // FLASH_FAILED "END"
        return false;
    }

    if (waitOptibootRes_1s())
    {
        if (_hex_size)
        {
            _addr += (len >> 1);
            // OK
        }
        else
        {
            Serial.write(0x51);
            Serial.write(0x20);
            // Done flashing!
            waitOptibootRes_1s();
            // Exiting bootloader... "END"
            return false;
        }
    }
    else
    {
        // FLASH_FAILED "END"
        return false;
    }

    return true;
}

void startFlashing(byte *payload, uint32_t length)
{
    _hex_size = 0;

    for (uint8_t i = 1; i < length; i++)
    {
        uint8_t payload_i = (uint8_t)payload[i];

        if ((payload_i >= '0') && (payload_i <= '9'))
        {
            _hex_size *= 10;
            _hex_size += payload_i - '0';
        }
        else
        {
            // [FAILED] Invalid hex length payload received. "END"
            return;
        }
    }

    _addr = 0;
    digitalWrite(RESETPIN, LOW);
    flushRxBuff();
    delay(200);
    digitalWrite(RESETPIN, HIGH);
    delay(300);
    getSync();
}

void startUpload(byte *payload, uint32_t length)
{
    startFlashing((uint8_t *)payload, (uint8_t)length);
    Serial.write(0x50);
    Serial.write(0x20);
    if (waitOptibootRes_1s())
    {
        sendHex_STK500((uint8_t *)payload, (uint8_t)length);
    }
}

boolean connectWifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    Serial.print("Trying to connecting...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void printHex()
{
    int len = strlen(hex);
    for (int i = 0; i < len; i++)
    {
        Serial.printf(&hex[0]);
    }
}

void setup()
{
    Serial.begin(115200);
    wifiConnected = connectWifi();
    if (wifiConnected)
    {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(200, "text/plain", "Hi! I am ESP8266."); });

        AsyncElegantOTA.begin(&server); // Start ElegantOTA
        server.begin();
        Serial.println("HTTP server started");
    }
}

void loop()
{
    if (wifiConnected)
    {
        Serial.print("Looping.....");
        delay(10000);
        printHex();
        delay(10000);
    }
}