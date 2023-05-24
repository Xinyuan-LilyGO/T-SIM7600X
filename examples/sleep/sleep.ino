/**
 * @file      sleep.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-05-24
 *
 */
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb
#define SerialAT                    Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

#include <TinyGsmClient.h>

#define uS_TO_S_FACTOR      1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP       30          /* Time ESP32 will go to sleep (in seconds) */

#define UART_BAUD           115200

#define MODEM_TX            27
#define MODEM_RX            26
#define MODEM_PWRKEY        4
#define MODEM_DTR           32
#define MODEM_RI            33
#define MODEM_FLIGHT        25
#define MODEM_STATUS        34

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#define LED_PIN             12


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
    delay(100);
    /*
    The indicator light of the board can be controlled
    */
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {
        /*
        MODEM_PWRKEY IO:4 The power-on signal of the modulator must be given to it,
        otherwise the modulator will not reply when the command is sent
        */
        pinMode(MODEM_PWRKEY, OUTPUT);
        digitalWrite(MODEM_PWRKEY, LOW);
        delay(100);
        digitalWrite(MODEM_PWRKEY, HIGH);
        //Ton >= 100 <= 500
        delay(100);
        digitalWrite(MODEM_PWRKEY, LOW);

        /*
        MODEM_FLIGHT IO:25 Modulator flight mode control,
        need to enable modulator, this pin must be set to high
        */
        pinMode(MODEM_FLIGHT, OUTPUT);
        digitalWrite(MODEM_FLIGHT, HIGH);

    } else {
        Serial.println("Wakeup modem !");

        // Need to cancel GPIO hold if wake from sleep
        gpio_hold_dis((gpio_num_t )MODEM_DTR);

        // Pull down DTR to wake up MODEM
        pinMode(MODEM_DTR, OUTPUT);
        digitalWrite(MODEM_DTR, LOW);
        delay(2000);
        modem.sleepEnable(false);
    }


    Serial.println("Check modem online .");
    while (!modem.testAT()) {
        Serial.print("."); delay(500);
    }
    Serial.println("Modem is online !");

    delay(5000);


    Serial.println("Enter modem sleep mode!");

    // Pull up DTR to put the modem into sleep
    pinMode(MODEM_DTR, OUTPUT);
    digitalWrite(MODEM_DTR, HIGH);
    // Set DTR to keep at high level, if not set, DTR will be invalid after ESP32 goes to sleep.
    gpio_hold_en((gpio_num_t )MODEM_DTR);
    gpio_deep_sleep_hold_en();


    if (modem.sleepEnable(true) != true) {
        Serial.println("modem sleep failed!");
    } else {
        Serial.println("Modem enter sleep modem!");
    }

    delay(5000);

    Serial.println("Check modem response .");
    while (modem.testAT()) {
        Serial.print("."); delay(500);
    }
    Serial.println("Modem is not respone ,modem has sleep !");

    delay(5000);

    Serial.println("Enter esp32 goto deepsleep!");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    delay(200);
    esp_deep_sleep_start();
    Serial.println("This will never be printed");


}

void loop()
{
}

