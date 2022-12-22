/**
 * @file      FTPUpload.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-12-21
 *
 */
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

#ifndef TINY_GSM_MODEM_SIM7600
#define TINY_GSM_MODEM_SIM7600
#endif

#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS


#include <TinyGsmClient.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>


#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#define FILESYSTEM          SD
#define FTP_SERVER          "Your FTP server address"
#define FTP_PORT            "21"
#define FTP_USERNAME        "Your FTP server username"
#define FTP_PASSWORD        "Your FTP server password"

// Your FTP server upload path,Replace with the path you need to set
#define REMOTE_PATH         "/download"

// Please put the files in the data folder in the SD card
// Files in SD card
#define FILE_NAME1          "/lilygo230k.jpg"
#define FILE_NAME2          "/lilygo.png"



void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.path(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}


void upload(const char *remotePath, const char *filename)
{
    modem.sendAT("+CFTPSPUTFILE=\"", remotePath, filename, "\",3");
    if (modem.waitResponse(30000) == 1) {
        if (modem.waitResponse(120000, "+CFTPSPUTFILE: ")) {
            int res = modem.stream.read();
            if (res == '0') {
                Serial.println("Upload png to efs success.");
            } else {
                Serial.print("Upload png to efs failed. errcode =");
                Serial.println(char(res));
                return ;
            }
        }
    } else {
        Serial.println("Upload png to efs failed."); return ;
    }
}


uint32_t writeFileToEFS(const char *filename)
{
    size_t  fileSize = 0;
    uint8_t *pdat = NULL;

    if (FILESYSTEM.exists(filename)) {
        File f = FILESYSTEM.open(filename);
        if (f) {
            fileSize = f.size();
            pdat = new  uint8_t [fileSize] ;
            uint32_t readSize =  f.read(pdat, fileSize);
            f.close();
            Serial.printf("Need read %u Bytes , reading %u Bytes\n", fileSize, readSize );
            modem.sendAT("+CFTRANRX=\"E:", filename, "\",", String(readSize));
            if (modem.waitResponse(30000, ">") == 1) {
                /*
                Too fast write speed will cause the modem to be unable to accurately receive the
                required number of bytes. It needs to do some delay when sending.
                After testing, every byte sent can be accurately received with a delay of 200~500 us
                // modem.stream.write(pdat, readSize);
                */
                //TODO:Need to optimize here
                for (uint32_t i = 0 ; i < readSize; ++i) {
                    modem.stream.write(pdat[i]);
                    delayMicroseconds(500);
                }
                modem.stream.println();
                if (modem.waitResponse(60000) == 1) {
                    Serial.println("Upload png to efs success.");
                } else {
                    Serial.println("Upload png to efs failed.");
                    fileSize = 0;
                }
            }
        }
    } else {
        Serial.print(filename);
        Serial.println(" does not exis"); return 0;
    }
    delete pdat;
    return fileSize;
}


void setup()
{
    Serial.begin(115200); // Set console baud rate
    //Initialize SDCard
    pinMode(SD_MISO, INPUT_PULLUP);
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS)) {
        Serial.println("SDCard MOUNT FAIL"); while (1);
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        String str = "SDCard Size: " + String(cardSize) + "MB";
        Serial.println(str);
    }


    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

    /*
    The indicator light of the board can be controlled
    */
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    /*
    MODEM_FLIGHT IO:25 Modulator flight mode control,
    need to enable modulator, this pin must be set to high
    */
    pinMode(MODEM_FLIGHT, OUTPUT);
    digitalWrite(MODEM_FLIGHT, HIGH);

    /*
    MODEM_PWRKEY IO:4 The power-on signal of the modulator must be given to it,
    otherwise the modulator will not reply when the command is sent
    */

    while (!modem.testAT(5000)) {
        Serial.println("Try to start modem...");
        pinMode(MODEM_PWRKEY, OUTPUT);
        digitalWrite(MODEM_PWRKEY, HIGH);
        delay(300); //Need delay
        digitalWrite(MODEM_PWRKEY, LOW);
    }

    Serial.println("\nTesting Modem Response");
    while (!modem.testAT(10000)) {
        Serial.println(".");
    }
    Serial.println();

    Serial.println("Modem Response Started.");


    listDir(FILESYSTEM, "/", 0);


    Serial.println("Wait for network register.");
    while (!modem.isNetworkConnected()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Network is registered!");
    delay(3000);



    modem.sendAT("+CFTPSLOGOUT");
    if (modem.waitResponse(3000) != 1) {
        // Nothing needs to be done, just send this command,
        // assuming it has been enabled
    }
    // Need to add some delay appropriately
    delay(500);

    modem.sendAT("+CFTPSSTOP");
    if (modem.waitResponse(3000) != 1) {
        // Nothing needs to be done, just send this command,
        // assuming it has been enabled
    }
    // Need to add some delay appropriately
    delay(500);


    modem.sendAT("+CFTPSSTART");
    if (modem.waitResponse(3000) != 1) {
        Serial.println("Start FTP Server failed");
        return ;
    }

    modem.sendAT("+CFTPSSINGLEIP=1");
    if (modem.waitResponse(3000) != 1) {
    }


    modem.sendAT("+CFTPSLOGIN=", "\"", FTP_SERVER, "\",", FTP_PORT, ",\"", FTP_USERNAME, "\",\"", FTP_PASSWORD, "\",0");
    if (modem.waitResponse() != 1) {
        Serial.println("Login failed!"); return;
    }
    // Wait for login return value
    if (modem.waitResponse(10000, "+CFTPSLOGIN: ") == 1) {
        int res = modem.stream.read();
        if (res == '0') {
            Serial.println("login success!");
        } else {
            Serial.println("login failed!"); return ;
        }
    }

    // Write the file to be uploaded to EFS
    // /lilygo230k.jpg About 231K
    writeFileToEFS(FILE_NAME1);

    // Upload EFS files to FTP
    upload(REMOTE_PATH, FILE_NAME1);

    // /lilygo.png About 17K
    writeFileToEFS(FILE_NAME2);

    upload(REMOTE_PATH, FILE_NAME2);

}


void loop()
{
    while (1) {
        if (SerialAT.available()) {
            Serial.write(SerialAT.read());
        }
        if (Serial.available()) {
            SerialAT.write(Serial.read());
        }
        delay(1);
    }

}
















