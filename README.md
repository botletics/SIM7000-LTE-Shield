## Overview
This shield uses SIMCOM's SIM7000-series LTE module which is a low-power cellular module that supports the new LTE CAT-M1 and NB-IoT technology and also has integrated high-speed, multi-GNSS (GPS, GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. The shield can be used in different regions around the world simply by using the appropriate module version, either the SIM7000A (American), SIM7000C (Chinese), SIM7000E (European), or SIM7000G (Global) as detailed in my [Github wiki](https://github.com/botletics/SIM7000-LTE-Shield/wiki/Board-Versions). The shield also includes a high-accuracy I2C temperature sensor for IoT monitoring applications. To use the shield, simply follow the [step-by-step wiki](https://github.com/botletics/SIM7000-LTE-Shield/wiki) to attach the headers, plug the shield into an Arduino, insert a compatible SIM card, attach the dual LTE/GPS antenna, and you're ready to load the example code!

You can buy the shield on [my website](https://www.botletics.com/products/sim7000-shield) or [on Amazon.com](http://a.co/d/9ZlQJ6y)

Check out the [comprehensive wiki](https://github.com/botletics/LTE-Shield/wiki) or use [this Instructables](https://www.instructables.com/id/LTE-NB-IoT-Shield-for-Arduino/) to get started if that works better for you!

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Arduino Library Support
The library in this repo is an altered library built upon the original [Adafruit FONA library](https://github.com/adafruit/Adafruit_FONA) with added functionality for 2G (SIM800/808), 3G (SIM5320), 4G LTE ([see the SIM7500 shield](https://github.com/botletics/SIM7500-LTE-Shield)) and the SIM7000 LTE CAT-M/NB-IoT module. As such, it's probably the best Arduino library for SIMCom modules available so far and I've also included examples and library functions focusing on sending data to the cloud via HTTP/HTTPS/MQTT with more functionalities always being tested!

The following list is a summary of the things I've done so far:

### Confirmed functionalities
- FTP functions on SIM7000G firmware version 1529B01SIM7000G
- Hologram SIM card works great on both the AT&T and Verizon CAT-M1 networks in the USA!
- HTTP and HTTPS via 3G with SIM5320
- MQTT working with SIM7000 and SIM808
- HTTP functions on SIM808 and other 2G modules
- Network time works
- GPS works on SIM7000 without a SIM card! The NMEA data includes UTC date/time, latitutde, longitude, altitude, etc and is quite accurate! It also obtains a GPS fix fairly quickly, only about 20-30s from cold start and even less (a couple seconds or so) when the device has already been on! However, this may differ based on your location and how deep you're buried inside a building.
- HTTP functions via LTE CAT-M1 with SIM7000
- SMS functions (sending/reading/deleting SMS)
- Generic stuff (reading supply voltage, netowrk connection, RSSI, etc.)

### To-Do List
- Finish FTP image transfer tutorial
- Test and document MDM9206 SDK for standalone SIM7000 operation without external microcontroller
- Dedicated MQTT commands for SIM7000? (Waiting for SIMCom to release)
- Test SSL on SIM7000 (with Amazon AWS and X.509 certificate)

### Completed Tasks
- Added FTP functions
- Confirmed NTP time sync on SIM7000G firmware version 1529B01SIM7000G
- Added "postData3G" function for SIM5320 and other 3G modules
- Successfully tested MQTT connect and publish using TCP/IP!
- Added "getNetworkInfo()" function (command "1" in LTE_Demo sketch) to get connection info, connection status, cellular band, etc.
- Added if statements that can be uncommented to eliminate unnecessary AT commands from "enableGPRS()" function for LTE modules (SIM7000) in case GPRS isn't being used at all
- Added HTTP status and data length verification in postData() function
- Added driver files in documentation folder
- Added voice and audio support on revision v5
- Added baud rate setup in the setup() function in example sketches to eliminate the need for a separate "setbaud" sketch.
- Tested the [Hologram developer SIM card](https://hologram.io/devplan/)
- Updated the IoT example sketch to include support for a GPS tracker (repeated data posting)
- Created [this cool IoT example](https://github.com/botletics/NB-IoT-Shield/tree/master/Code/examples/IoT_Example) that posts GPS location, temperature, and battery data to the cloud!
- Updated the function "getGPS()" to include the LTE shield class
- I have tested almost every AT command and they have all worked!
- Added a function "postData()" for posting data to dweet.io, a free cloud API. You can choose to use HTTP GET or POST and I have added an example in the "FONA_LTE_Test" sketch (enter "2" in the menu for the option)
- Tested the [AT&T Trio SIM card](https://marketplace.att.com/products/trio-sim-trial)
- Tweaked the library so that the LTE declaration "Adafruit_FONA_LTE fona = Adafruit_FONA_LTE()" doesn't include the reset pin because the reset pin is reserved for emergencies only, according to the SIM7000 design document.
- Tested HTTP commands
- Tested the dual LTE/GNSS antenna signal strength. Inside (near the window) I get an RSSI around 31 (-52 dBm) for AT&T
- Added "powerDown()" function using "AT+CPOWD=1" to turn off the SIM7000
- Added "hangUp()" function
- Created "FONA_LTE_setbaud.ino" sketch to set baud rate to 4800 from default 115200 because at 115200 some parts of text would show up weird in the serial monitor.
-	Added “setBaudrate(uint16_t baud)” function for LTE class using "AT+IPR=<rate>". 
-	Included FONA_LTE_A and FONA_LTE_E types in .h file but not in .cpp declaration (I don't have a SIM7000E module with me so I don't know what its manufacturer-assigned name is)
-	Included FONA_LTE class to the library for SIM7000A module
