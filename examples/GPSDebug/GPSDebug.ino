/**
 * @file      GPSDebug.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-13
 * @note      The diagram shows the result through external analysis analysis NMAE example,
 *            can be used for GPS failure judgment, can be used to see GPS correct or not operation normal.
 *            If the GPS output is NMEA phrase, it is possible to confirm that GPS is normal. https://github.com/Xinyuan-LilyGO/T-SIM7600X/issues/42#issuecomment-1507181275
 *            It's been a long time since I've been unlocated.
 * */
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS


// Use TinyGPS NMEA math analysis library
// #define USING_TINYGPS_LIBRARY           https://github.com/mikalhart/TinyGPSPlus.git

// set GSM PIN, if any
#define GSM_PIN ""

#include <TinyGsmClient.h>
#include "utilities.h"

#ifdef USING_TINYGPS_LIBRARY
// Use TinyGPS NMEA math analysis library
#include <TinyGPS++.h>
TinyGPSPlus gps;
void displayInfo();
#endif


#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

void setup()
{
    Serial.begin(115200); // Set console baud rate
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

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

    Serial.println("Start modem...");


    for (int i = 0; i < 3; ++i) {
        while (!modem.testAT(5000)) {
            Serial.println("Try to start modem...");
            pinMode(MODEM_PWRKEY, OUTPUT);
            digitalWrite(MODEM_PWRKEY, HIGH);
            delay(300); //Need delay
            digitalWrite(MODEM_PWRKEY, LOW);
        }
    }

    Serial.println("Modem Response Started.");

    // Stop GPS Server
    modem.sendAT("+CGPS=0");
    modem.waitResponse(30000);

    // Configure GNSS support mode
    modem.sendAT("+CGNSSMODE=15,1");
    modem.waitResponse(30000);

    // Configure NMEA sentence type
    modem.sendAT("+CGPSNMEA=200191");
    modem.waitResponse(30000);

    // Set NMEA output rate to 1HZ
    modem.sendAT("+CGPSNMEARATE=1");
    modem.waitResponse(30000);

    // Enable GPS
    modem.sendAT("+CGPS=1");
    modem.waitResponse(30000);

    // Download Report GPS NMEA-0183 sentence , NMEA TO AT PORT
    modem.sendAT("+CGPSINFOCFG=1,31");
    modem.waitResponse(30000);


    //Disable NMEA OUTPUT
    // modem.sendAT("+CGPSINFOCFG=0,31");
    // modem.waitResponse(30000);
}

void loop()
{

#ifdef USING_TINYGPS_LIBRARY
    while (SerialAT.available()) {
        if (gps.encode(SerialAT.read())) {
            displayInfo();
        }
    }
#else
    if (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }
    if (Serial.available()) {
        SerialAT.write(Serial.read());
    }
#endif
    delay(1);
}


#ifdef USING_TINYGPS_LIBRARY
void displayInfo()
{
    Serial.print(F("Location: "));
    if (gps.location.isValid()) {
        Serial.print(gps.location.lat(), 6);
        Serial.print(F(","));
        Serial.print(gps.location.lng(), 6);
    } else {
        Serial.print(F("INVALID"));
    }

    Serial.print(F("  Date/Time: "));
    if (gps.date.isValid()) {
        Serial.print(gps.date.month());
        Serial.print(F("/"));
        Serial.print(gps.date.day());
        Serial.print(F("/"));
        Serial.print(gps.date.year());
    } else {
        Serial.print(F("INVALID"));
    }

    Serial.print(F(" "));
    if (gps.time.isValid()) {
        if (gps.time.hour() < 10) Serial.print(F("0"));
        Serial.print(gps.time.hour());
        Serial.print(F(":"));
        if (gps.time.minute() < 10) Serial.print(F("0"));
        Serial.print(gps.time.minute());
        Serial.print(F(":"));
        if (gps.time.second() < 10) Serial.print(F("0"));
        Serial.print(gps.time.second());
        Serial.print(F("."));
        if (gps.time.centisecond() < 10) Serial.print(F("0"));
        Serial.print(gps.time.centisecond());
    } else {
        Serial.print(F("INVALID"));
    }

    Serial.println();
}
#endif