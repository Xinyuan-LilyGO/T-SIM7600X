<h1 align = "center">🌟LilyGo SIM7600X🌟</h1>

<h2 align = "left">Product 📷:</h2>

|   Product   |                             Product  Link                             |                    Schematic                    | Status |
| :---------: | :-------------------------------------------------------------------: | :---------------------------------------------: | :----: |
| T -SIM7600X | [Product link](https://www.aliexpress.com/item/1005001705250713.html) | [Schematic](schematic/T54_SIM7600_20201012.pdf) |   ✅    |


## Notes for new Users

1. pin VIN is a USB input, if a battery is being used to power the device there will be no voltage output from VIN meaning 3.3v is the only power output.

2. When using the built-in battery pack the on/off switch will function as normal, supplying/cutting off power to the board, However, if an external battery pack is used and attached to the VBAT pin the on/off switch will be bypassed meaning the only way to shut off will be to disconnect the batteries.

3. On/off switch is for battery use only, it will not function when plugged into USB.

4. Battery voltage can not be read when plugged into USB using the onboard BAT ADC(35) 

5. Recommended solar panels are 4.4v to 6v **DO NOT EXCEED OR BOARD MAY BE DAMAGED** 

6. When an SD card in you can not upload software in Arduino IDE since the SD card uses IO2 as CS, the SD card must be removed when uploading a new sketch. 

7. There are two Type-C interfaces on the board, which are ESP32 download port and modem USB. On the back of the board, there are four switches for selection.
    1. When using ESP32 programming, select as shown below
    <img  height="320" width="240" src=image/TTL.png>

    2. When using a modem to connect a computer to the Internet, select as shown below[Click here for connection method](docs/pc/README.MD)
    <img  height="320" width="240" src=image/USB.png>



## Features

![](image/SIM7600X_1.jpg)
![](image/SIM7600X_2.jpg)
![](image/SIM7600X_3.jpg)

