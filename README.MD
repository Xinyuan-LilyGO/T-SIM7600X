<h1 align = "center">LilyGo SIM7600X</h1>

<h2 align = "left">Product 📷:</h2>

|  Examples   |  Product  Link   |    Schematic    | Status |
| :---------: | :--------------: | :-------------: | :----: |
| [Version]() | [Product link]() | [New Version]() |   ✅    |


## Notes for new Users

1. pin VIN is a USB input, if a battery is being used to power the device there will be no voltage output from VIN meaning 3.3v is the only power output.

2. When using the built-in battery pack the on/off switch will function as normal, supplying/cutting off power to the board, However, if an external battery pack is used and attached to the VBAT pin the on/off switch will be bypassed meaning the only way to shut off will be to disconnect the batteries.

3. On/off switch is for battery use only, it will not function when plugged into USB.

4. Battery voltage can not be read when plugged into USB using the onboard BAT ADC(35) 

5. Recommended solar panels are 4.4v to 6v **DO NOT EXCEED OR BOARD MAY BE DAMAGED** 

6. When an SD card in you can not upload software in Arduino IDE since the SD card uses IO2 as CS, the SD card must be removed when uploading a new sketch. 

7. There are two Type-C interfaces on the board, which are ESP32 download port and modem USB. On the back of the board, there are four switches for selection.
    1. When using ESP32 programming, select as shown below
        ![](schematic/TTL.png)

        2.When using a modem to connect a computer to the Internet, select as shown below[Click here for connection method](docs/README.MD)
            ![](schematic/USB.png)


