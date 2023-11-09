/**
 * @file      PowefOffModem.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-11-09
 * @note      PWRKEY method power off modem example
 * */
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>
#include "utilities.h"


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


    while (!modem.testAT(5000)) {
        Serial.print(".");
    }

    Serial.println("\nModem Response Started.");


    delay(5000);

    Serial.println("Power off modem.");

    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(2600); //Need delay
    digitalWrite(MODEM_PWRKEY, LOW);

    int i = 3;
    while (i--) {
        if (!modem.testAT()) {
            Serial.print(".");
        }
        delay(1000);
    }

    if (!modem.testAT()) {
        Serial.println("Modem power off successed!");
    } else {
        Serial.println("Modem power off failed!");
    }


    Serial.println("Modem power on ...");
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(300); //Need delay
    digitalWrite(MODEM_PWRKEY, LOW);

    i = 3;
    while (i--) {
        if (!modem.testAT()) {
            Serial.print(".");
        } else {
            break;
        }
        delay(1000);
    }

    if (!modem.testAT()) {
        Serial.print("Modem power off failed!");
    } else {
        Serial.print("Modem power off successed!");
    }

}

void loop()
{

}