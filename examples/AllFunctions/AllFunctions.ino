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
#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

/*
   Tests enabled
*/
#define TINY_GSM_TEST_GPRS          true
#define TINY_GSM_TEST_TCP           true
// #define TINY_GSM_TEST_CALL     true
// #define TINY_GSM_TEST_SMS      true
// #define TINY_GSM_TEST_USSD     true
// #define TINY_GSM_TEST_TEMPERATURE   true
// #define TINY_GSM_TEST_TIME          true
#define TINY_GSM_TEST_GPS           true
// powerdown modem after tests
#define TINY_GSM_POWERDOWN          true
// #define TEST_RING_RI_PIN            true

// set GSM PIN, if any
#define GSM_PIN             ""

// Set phone numbers, if you want to test SMS and Calls
// #define SMS_TARGET  "+380xxxxxxxxx"
// #define CALL_TARGET "+380xxxxxxxxx"

// Your GPRS credentials, if any
const char apn[] = "YourAPN";
// const char apn[] = "ibasis.iot";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Server details to test TCP/SSL
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";

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
  //     while (1) {
  //         while (SerialMon.available()) {
  //             SerialAT.write(SerialMon.read());
  //         }
  //         while (SerialAT.available()) {
  //             SerialMon.write(SerialAT.read());
  //         }
  //     }
}

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
  if (!modem.init()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    return;
  }
  //
  //    // Restart takes quite some time
  //    // To skip it, call init() instead of restart()
  //    DBG("Initializing modem...");
  //    if (!modem.restart()) {
  //        DBG("Failed to restart modem, delaying 10s and retrying");
  //        // restart autobaud in case GSM just rebooted
  //        return;
  //    }

#if TINY_GSM_TEST_GPRS
  /*  Preferred mode selection : AT+CNMP
        2 – Automatic
        13 – GSM Only
        14 – WCDMA Only
        38 – LTE Only
        59 – TDS-CDMA Only
        9 – CDMA Only
        10 – EVDO Only
        19 – GSM+WCDMA Only
        22 – CDMA+EVDO Only
        48 – Any but LTE
        60 – GSM+TDSCDMA Only
        63 – GSM+WCDMA+TDSCDMA Only
        67 – CDMA+EVDO+GSM+WCDMA+TDSCDMA Only
        39 – GSM+WCDMA+LTE Only
        51 – GSM+LTE Only
        54 – WCDMA+LTE Only
  */
  String ret;
  //    do {
  //        ret = modem.setNetworkMode(2);
  //        delay(500);
  //    } while (ret != "OK");
  ret = modem.setNetworkMode(2);
  DBG("setNetworkMode:", ret);


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

  String name = modem.getModemName();
  DBG("Modem Name:", name);

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);

  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }

  DBG("Waiting for network...");
  if (!modem.waitForNetwork(600000L)) {
    light_sleep(10);
    return;
  }

  if (modem.isNetworkConnected()) {
    DBG("Network connected");
  }
#endif


#if TINY_GSM_TEST_GPRS
  DBG("Connecting to", apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    light_sleep(10);
    return;
  }

  res = modem.isGprsConnected();
  DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  String imsi = modem.getIMSI();
  DBG("IMSI:", imsi);

  String cop = modem.getOperator();
  DBG("Operator:", cop);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);

  int csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);
#endif

#if TINY_GSM_TEST_USSD && defined TINY_GSM_MODEM_HAS_SMS
  String ussd_balance = modem.sendUSSD("*111#");
  DBG("Balance (USSD):", ussd_balance);

  String ussd_phone_num = modem.sendUSSD("*161#");
  DBG("Phone number (USSD):", ussd_phone_num);
#endif

#if TINY_GSM_TEST_TCP && defined TINY_GSM_MODEM_HAS_TCP
  TinyGsmClient client(modem, 0);
  const int port = 80;
  DBG("Connecting to ", server);
  if (!client.connect(server, port)) {
    DBG("... failed");
  } else {
    // Make a HTTP GET request:
    client.print(String("GET ") + resource + " HTTP/1.0\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.print("Connection: close\r\n\r\n");

    // Wait for data to arrive
    uint32_t start = millis();
    while (client.connected() && !client.available() &&
           millis() - start < 30000L) {
      delay(100);
    };

    // Read data
    start = millis();
    while (client.connected() && millis() - start < 5000L) {
      while (client.available()) {
        SerialMon.write(client.read());
        start = millis();
      }
    }
    client.stop();
  }
#endif

#if TINY_GSM_TEST_CALL && defined(CALL_TARGET)

  DBG("Calling:", CALL_TARGET);
  SerialAT.println("ATD"CALL_TARGET";");
  modem.waitResponse();
  light_sleep(20);
#endif

#if TINY_GSM_TEST_GPS && defined TINY_GSM_MODEM_HAS_GPS
  DBG("Enabling GPS/GNSS/GLONASS");
  modem.enableGPS();
  light_sleep(2);

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
  DBG("Retrieving GPS/GNSS/GLONASS location again as a string");
  String gps_raw = modem.getGPSraw();
  DBG("GPS/GNSS Based Location String:", gps_raw);
  DBG("Disabling GPS");
  modem.disableGPS();
#endif

#if TINY_GSM_TEST_TIME && defined TINY_GSM_MODEM_HAS_TIME
  int year3 = 0;
  int month3 = 0;
  int day3 = 0;
  int hour3 = 0;
  int min3 = 0;
  int sec3 = 0;
  float timezone = 0;
  for (int8_t i = 5; i; i--) {
    DBG("Requesting current network time");
    if (modem.getNetworkTime(&year3, &month3, &day3, &hour3, &min3, &sec3,
                             &timezone)) {
      DBG("Year:", year3, "\tMonth:", month3, "\tDay:", day3);
      DBG("Hour:", hour3, "\tMinute:", min3, "\tSecond:", sec3);
      DBG("Timezone:", timezone);
      break;
    } else {
      DBG("Couldn't get network time, retrying in 15s.");
      light_sleep(15);
    }
  }
  DBG("Retrieving time again as a string");
  String time = modem.getGSMDateTime(DATE_FULL);
  DBG("Current Network Time:", time);
#endif

#if TINY_GSM_TEST_GPRS
  modem.gprsDisconnect();
  light_sleep(5);
  if (!modem.isGprsConnected()) {
    DBG("GPRS disconnected");
  } else {
    DBG("GPRS disconnect: Failed.");
  }
#endif

#if TINY_GSM_TEST_TEMPERATURE && defined TINY_GSM_MODEM_HAS_TEMPERATURE
  float temp = modem.getTemperature();
  DBG("Chip temperature:", temp);
#endif

#ifdef TEST_RING_RI_PIN
#ifdef MODEM_RI
  //Set RI Pin input
  pinMode(MODEM_RI, INPUT);

  Serial.println("Wait for call in");
  //When is no calling ,RI pin is high level
  while (digitalRead(MODEM_RI)) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("call in ");

  //Wait for 5 seconds to connect the call
  delay(5000);

  //Accept call
  SerialAT.println("ATA");

  // Hang up after 20 seconds of talk time
  delay(20000);

  SerialAT.println("ATH");

#endif  //MODEM_RI
#endif  //TEST_RING_RI_PIN


#ifdef MODEM_DTR1

  modem.sleepEnable();

  delay(100);

  // test modem response , res == 0 , modem is sleep
  res = modem.testAT();
  Serial.print(" Test AT result -> ");
  Serial.println(res);

  delay(1000);

  Serial.println("Use DTR Pin Wakeup");
  pinMode(MODEM_DTR, OUTPUT);
  //Set DTR Pin low , wakeup modem .
  digitalWrite(MODEM_DTR, LOW);

  // test modem response , res == 1 , modem is wakeup
  res = modem.testAT();
  Serial.print(" Test AT result -> ");
  Serial.println(res);

#endif


#if TINY_GSM_POWERDOWN
  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.poweroff();
  DBG("Poweroff.");
#endif

  SerialMon.printf("End of tests. Enable deep sleep , Will wake up in %d seconds", TIME_TO_SLEEP);

  // Wait for modem to power off
  light_sleep(5);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(200);
  esp_deep_sleep_start();

  while (1);
}
