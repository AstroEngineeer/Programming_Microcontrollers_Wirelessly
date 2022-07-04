//Receiver Code
#include <SoftwareSerial.h>
SoftwareSerial Serial1(2, 3); // RX, TX nodemcu

String test;
String test2;
String test3;
String test4;
boolean lock1 = false;
boolean lock2 = false;
boolean lock3 = false;
boolean lock4 = false;

void setup() {
  //delay(5000);
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(5, OUTPUT); // gpio which is attaced to Arduino reset pin.
  delay(100);
  digitalWrite(5, LOW); // sends a brief reset pulse
  delay(50); // 50ms works better
  digitalWrite(5, HIGH); // clears reset
  delay(50); // ~50ms later
  Serial1.write("\x20"); // sends CRC_EOP+CRC_EOP
  delay(70); // ~70ms later
}

//esp-link sends a brief reset pulse (1ms)
//esp-link sends CRC_EOP+CRC_EOP ~50ms later
//esp-link sends CRC_EOP+CRC_EOP every ~70-80ms
//eventually optiboot responds with STK_INSYNC+STK_OK (0x14;0x10)
//esp-link sends one CRC_EOP to sort out the even/odd issue
//either optiboot responds with STK_INSYNC+STK_OK or nothing happens for 70-80ms, in which case esp-link sends another CRC_EOP
//esp-link sends STK_GET_SYNC+CRC_EOP and optiboot responds with STK_INSYNC+STK_OK and we're in sync now ------------------------------ TO DO
//esp-link sends the next command (starts with 'u') and programming starts...

void loop() {
  if (!lock1) {
    while (Serial1.available()) {
      test += String(Serial1.read()); // check for responce with STK_INSYNC+STK_OK (0x14;0x10)
    }
    if (test == "2016") { // eventually optiboot responds with STK_INSYNC+STK_OK (0x14;0x10)
      lock1 = true;
    }
    if (!lock1) {
      Serial1.write("\x30\x20");
      delay(70);
    }
  }
  else {
    if (!lock2) {
      while (Serial1.available()) {
        test2 += String(Serial1.read());
      }
      if (test2 == "2016") {
        lock2 = true;
      }
      if (!lock2) {
        Serial1.write("\x20"); // send one CRC_EOP to sort out the even/odd issue
        delay(70);
      }
    }
    else {
      if (!lock3) {
        while (Serial1.available()) {
          test3 += String(Serial1.read());
        }
        if (test3 == "2016") {
          lock3 = true;
        }
        if (!lock3) {
          Serial1.write("\x50\x20");
          delay(70);
        }
      }
      else {
        if (!lock4) {
          while (Serial1.available()) {
            test4 += String(Serial1.read());
            if (test4 == "20") {
              lock4 = true;
              Serial.println("You f***ing legend Flash time ahhhhhhhhhhh!");
            }
          }
          if (!lock4) {
            Serial1.write("\x75\x20");
            delay(70);
          }
        }
        else {
          // print hex here the connection will have to be timed so it happens within 5 secs timeout. 
          // then reset arduino
        }
      }
    }
  }
}
