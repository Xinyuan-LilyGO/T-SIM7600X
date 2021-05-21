/**************************************************************
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 * NOTE:
 * Some of the functions may be unavailable for your modem.
 * Just comment them out.
 * https://simcom.ee/documents/SIM7600C/SIM7500_SIM7600%20Series_AT%20Command%20Manual_V1.01.pdf
 **************************************************************/

#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include <TinyGsmClient.h>
#include "utilities.h"

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);
    delay(10);

    // Set GSM module baud rate
    SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);

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

    //Initialize SDCard
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS)) {
        Serial.println("SDCard MOUNT FAIL");
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        String str = "SDCard Size: " + String(cardSize) + "MB";
        Serial.println(str);
    }

    // Uncomment below will perform loopback test
//    while (1) {
//        while (SerialMon.available()) {
//            SerialAT.write(SerialMon.read());
//        }
//        while (SerialAT.available()) {
//            SerialMon.write(SerialAT.read());
//        }
//    }
}

void Init(void);
void light_sleep(uint32_t sec )
{
    esp_sleep_enable_timer_wakeup(sec * 1000000ULL);
    esp_light_sleep_start();
}


void loop()
{
    bool res ;

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    DBG("Initializing modem...");
    if (!modem.restart()) {
        DBG("Failed to restart modem, delaying 10s and retrying");
        // restart autobaud in case GSM just rebooted
        return;
    }
    Init();

    DBG("Access to SSL/TLS server (not verify server and client)");
    modem.sendAT("+CCHOPEN=0,\"www.baidu.com\", 443,2");//AT+CCHOPEN=<session_id>,  ”<host>”,<port>[<client_type>,[<bind_port>]]
    //<host> ----The server address, maximum length is 256 bytes.
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHOPEN=0,\"www.baidu.com\", 443,2 Fail");
    }

    delay(2000);
    modem.sendAT("+CCHSEND=0,121");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHSEND=0,121 Fail");
    }
    /*Send data to server
     * Example
      GET / HTTP/1.1
      Host: www.baidu.com
      User-Agent: MAUI htp User Agent
      Proxy-Connection: keep-alive
      Content-Length: 0
      */

    SerialAT.write("GET / HTTP/1.1\r\n\
Host: www.baidu.com\r\n\
User-Agent: MAUI htp User Agent\r\n\
Proxy-Connection: keep-alive\r\n\
Content-Length: 0\r\n\r\n");

    if (modem.waitResponse(10000L) != 1) {
        DBG("report the received data from server fail");

    }
    if (modem.waitResponse(10000L) != 1) {
        DBG("report the received data from server fail");
    }
    if (modem.waitResponse(10000L) == 1) {
        //DBG("OK");
    }


    DBG("Access to SSL/TLS server (not verify server and client)");
    modem.sendAT("+CCHOPEN=0,\"www.sogou.com\", 443,2");//AT+CCHOPEN=<session_id>,  ”<host>”,<port>[<client_type>,[<bind_port>]]
    //<host> ----The server address, maximum length is 256 bytes.
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHOPEN=0,\"www.sogou.com\", 443,2 Fail");
    }

    delay(2000);
    modem.sendAT("+CCHSEND=0,121");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHSEND=0,121 Fail");
    }
    SerialAT.write("GET / HTTP/1.1\r\n\
Host: www.sogou.com\r\n\
User-Agent: MAUI htp User Agent\r\n\
Proxy-Connection: keep-alive\r\n\
Content-Length: 0\r\n\r\n");

    DBG("/**********Test OK**********/");
    DBG("End SSL Service");
    modem.sendAT("+CCHCLOSE=0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHCLOSE=0 Fail");
    }
    modem.sendAT("+CCHSTOP");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHSTOP Fail");
    }

    modem.poweroff();
    DBG("Poweroff.");

    SerialMon.printf("End of tests. Enable deep sleep , Will wake up in %d seconds", TIME_TO_SLEEP);

    //Wait moden power off
    light_sleep(5);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    delay(200);
    esp_deep_sleep_start();
    while (1) {
        delay(1000);
    };
}


void Init(void)
{

    modem.sendAT("+CPIN?");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CPIN? Fail");
    }

    modem.sendAT("+CSQ");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CSQ Fail");
    }

    modem.sendAT("+CREG?");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CREG? Fail");
    }
    modem.sendAT("+CGATT?");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CGATT? Fail");
    }
    modem.sendAT("+CPSI?");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+AT+CPSI? Fail");
    }

    modem.sendAT("+CGDCONT=1,\"IP\",\"cmnet\"");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CGDCONT=1,\"IP\",\"cmnet\" Fail");
    }

    modem.sendAT("+NETOPEN");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+NETOPEN Fail");
    }


    modem.sendAT("+CSSLCFG=\"sslversion\",0,4");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CSSLCFG=\"sslversion\",0,4 Fail");
    }
    modem.sendAT("+CSSLCFG=\"authmode\",0,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CSSLCFG=\"authmode\",0,0 Fail");
    }
    modem.sendAT("+CCHSET=1");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHSET=1 Fail");
    }
    modem.sendAT("+CCHSTART");
    if (modem.waitResponse(10000L) != 1) {
        DBG("+CCHSTART Fail");
    }
    modem.sendAT("+CCHSSLCFG=0,0");
    if (modem.waitResponse(10000L) != 1) {
        DBG("CCHSSLCFG=0,0 Fail");
    }

}
