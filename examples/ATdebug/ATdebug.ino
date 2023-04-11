/*
  FILE: ATdebug.ino
  AUTHOR: Kaibin
  PURPOSE: Test functionality
*/

#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS


// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]  = "YOUR-APN";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

#include <TinyGsmClient.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include "utilities.h"


#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif



int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;


bool reply = false;

void modem_on() {
    /*
    The indicator light of the board can be controlled
    */
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    /*
    MODEM_PWRKEY IO:4 The power-on signal of the modulator must be given to it,
    otherwise the modulator will not reply when the command is sent
    */
    pinMode(MODEM_PWRKEY, OUTPUT);
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(300); //Need delay
    digitalWrite(MODEM_PWRKEY, LOW);

    /*
    MODEM_FLIGHT IO:25 Modulator flight mode control,
    need to enable modulator, this pin must be set to high
    */
    pinMode(MODEM_FLIGHT, OUTPUT);
    digitalWrite(MODEM_FLIGHT, HIGH);

    
  int i = 40;
  Serial.print(F("\r\n# Startup #\r\n"));
  Serial.print(F("# Sending \"AT\" to Modem. Waiting for Response\r\n# "));
  while (i) {
    SerialAT.println(F("AT"));

    // Show the User: we are doing something.
    Serial.print(F("."));
    delay(500);

    // Did the Modem send something?
    if (SerialAT.available()) {
      String r = SerialAT.readString();
      Serial.print("\r\n# Response:\r\n" + r);
      if ( r.indexOf("OK") >= 0 ) {
        reply = true;
        break;;
      } else {
        Serial.print(F("\r\n# "));
      }
    }

    // Did the User try to send something? Maybe he did not receive the first messages yet. Inform the User what is happening
    if (Serial.available() && !reply) {
      Serial.read();
      Serial.print(F("\r\n# Modem is not yet online."));
      Serial.print(F("\r\n# Sending \"AT\" to Modem. Waiting for Response\r\n# "));
    }

    // On the 5th try: Inform the User what is happening
    if(i == 35) {
      Serial.print(F("\r\n# Modem did not yet answer. Probably Power loss?\r\n"));
      Serial.print(F("# Sending \"AT\" to Modem. Waiting for Response\r\n# "));
    }
    delay(500);
    i--;
  }
  Serial.println(F("#\r\n"));
}

void setup() {
  Serial.begin(115200); // Set console baud rate
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(100);

  modem_on();
  if (reply) {
    Serial.println(F("***********************************************************"));
    Serial.println(F(" You can now send AT commands"));
    Serial.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
    Serial.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
    Serial.println(F(" DISCLAIMER: Entering AT commands without knowing what they do"));
    Serial.println(F(" can have undesired consiquinces..."));
    Serial.println(F("***********************************************************\n"));

    // Uncomment to read received SMS
    //SerialAT.println("AT+CMGL=\"ALL\"");
  } else {
    Serial.println(F("***********************************************************"));
    Serial.println(F(" Failed to connect to the modem! Check the baud and try again."));
    Serial.println(F("***********************************************************\n"));
  }
}

void loop() {
  if (SerialAT.available()) {
    Serial.write(SerialAT.read());
  }
  if (Serial.available()) {
    SerialAT.write(Serial.read());
  }
  delay(1);
}
