/**************************************************************

   TinyGSM Getting Started guide:
     https://tiny.cc/tinygsm-readme

   NOTE:
   Some of the functions may be unavailable for your modem.
   Just comment them out.
   https://simcom.ee/documents/SIM7600C/SIM7500_SIM7600%20Series_AT%20Command%20Manual_V1.01.pdf
 **************************************************************/

#define TINY_GSM_MODEM_SIM7600

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

#define SMS_TARGET  "+86xxxxxxx" //Change the number you want to send sms message

// Your GPRS credentials, if any
const char apn[] = "YourAPN";
// const char apn[] = "ibasis.iot";
const char gprsUser[] = "";
const char gprsPass[] = "";


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

void light_sleep(uint32_t sec )
{
    esp_sleep_enable_timer_wakeup(sec * 1000000ULL);
    esp_light_sleep_start();
}


void setup()
{
    // Set console baud rate
    SerialMon.begin(115200);

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

    Serial.println("Start modem...");
    delay(3000);

    while (!modem.testAT()) {
        delay(10);
    }

    bool ret = modem.setNetworkMode(2);
    DBG("setNetworkMode:", ret);

    // Check network registration status and network signal status
    int16_t sq ;
    Serial.print("Wait for the modem to register with the network.");
    SIM7600RegStatus status = REG_NO_RESULT;
    while (status == REG_NO_RESULT || status == REG_SEARCHING || status == REG_UNREGISTERED) {
        status = modem.getRegistrationStatus();
        switch (status) {
        case REG_UNREGISTERED:
        case REG_SEARCHING:
            sq = modem.getSignalQuality();
            Serial.printf("[%lu] Signal Quality:%d\n", millis() / 1000, sq);
            delay(1000);
            break;
        case REG_DENIED:
            Serial.println("Network registration was rejected, please check if the APN is correct");
            return ;
        case REG_OK_HOME:
            Serial.println("Online registration successful");
            break;
        case REG_OK_ROAMING:
            Serial.println("Network registration successful, currently in roaming mode");
            break;
        default:
            Serial.printf("Registration Status:%d\n", status);
            delay(1000);
            break;
        }
    }
    Serial.println();
    Serial.printf("Registration Status:%d\n", status);
    delay(1000);

    //https://github.com/vshymanskyy/TinyGSM/pull/405
    uint8_t mode = modem.getGNSSMode();
    DBG("GNSS Mode:", mode);

    /**
        CGNSSMODE: <gnss_mode>,<dpo_mode>
        This command is used to configure GPS, GLONASS, BEIDOU and QZSS support mode.
        gnss_mode:
            0 : GLONASS
            1 : BEIDOU
            2 : GALILEO
            3 : QZSS
        dpo_mode :
            0 disable
            1 enable
    */
    modem.setGNSSMode(1, 1);
    light_sleep(1);

    DBG("Enabling GPS/GNSS/GLONASS");
    modem.enableGPS();
    light_sleep(2);

    // Text mode
    modem.sendAT("+CMGF=1");
    modem.waitResponse();
}


bool start_positioning = false;
void loop()
{
    int8_t  ret = modem.waitResponse(60000, "+CMTI: \"SM\",");
    if (ret == 1) {
        String mes_index = modem.stream.readStringUntil('\n');
        modem.waitResponse();//Wait oK!

        Serial.printf("Message come.. , index : %d\n", mes_index.toInt());
        Serial.println("-----------------------");
        modem.sendAT("+CMGR=", mes_index);
        ret = modem.waitResponse(30000UL, "+CMGR: ");
        if (ret == 1) {
            modem.stream.readStringUntil('\n'); //Skip \n
            String data = modem.stream.readStringUntil('\n'); //Get content
            Serial.print("MSG:");
            Serial.println(data);
            modem.waitResponse();   //Wait oK!

            if (data.startsWith("Location")) {
                start_positioning = true;
            }
        }
    }

    if (!start_positioning) {
        return ; // Next loop
    }

    float lat2      = 0;
    float lon2      = 0;
    float speed2    = 0;
    float alt2      = 0;
    int   vsat2     = 0;
    int   usat2     = 0;
    float accuracy2 = 0;
    int   year2     = 0;
    int   month2    = 0;
    int   day2      = 0;
    int   hour2     = 0;
    int   min2      = 0;
    int   sec2      = 0;
    DBG("Requesting current GPS/GNSS/GLONASS location");
    for (;;) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        if (modem.getGPS(&lat2, &lon2, &speed2, &alt2, &vsat2, &usat2, &accuracy2,
                         &year2, &month2, &day2, &hour2, &min2, &sec2)) {
            DBG("Latitude:", String(lat2, 8), "\tLongitude:", String(lon2, 8));
            DBG("Speed:", speed2, "\tAltitude:", alt2);
            DBG("Visible Satellites:", vsat2, "\tUsed Satellites:", usat2);
            DBG("Accuracy:", accuracy2);
            DBG("Year:", year2, "\tMonth:", month2, "\tDay:", day2);
            DBG("Hour:", hour2, "\tMinute:", min2, "\tSecond:", sec2);
            break;
        } else {
            light_sleep(2);
        }
    }

    String msg_str = "Longitude:" + String(lon2, 6) + " ";
    msg_str += "Latitude:" + String(lat2, 6) + "\n";
    msg_str += "UTC Date:" + String(year2) + "/";
    msg_str +=  String(month2) + "/";
    msg_str +=  String(day2) + " \n";
    msg_str += "UTC Time:" + String(hour2) + ":";
    msg_str += String(min2) + ":";
    msg_str += String(sec2);
    msg_str += "\n";

    Serial.print("MESSAGE:"); Serial.println(msg_str);

    bool res = modem.sendSMS(SMS_TARGET, msg_str);
    Serial.print("Send sms message ");
    Serial.println(res ? "OK" : "fail");

    DBG("Retrieving GPS/GNSS/GLONASS location again as a string");
    String gps_raw = modem.getGPSraw();
    DBG("GPS/GNSS Based Location String:", gps_raw);
    DBG("Disabling GPS");
    modem.disableGPS();

    SerialMon.printf("End of tests. Enable deep sleep , Will wake up in %d seconds", TIME_TO_SLEEP);

    // Wait for modem to power off
    light_sleep(5);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    delay(200);
    esp_deep_sleep_start();

    while (1);
}
