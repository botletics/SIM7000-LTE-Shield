## Overview
The LTE Shield uses SIMCOM's SIM7000-series LTE modules which are low-power 4G LTE modules that support the new LTE CAT-M NB-IoT technology and also have integrated GNSS (GPS,GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. NB-IoT is achieved with the different versions, namely the SIM7000E, SIM7000C, and SIM7000E-N modules. To use the LTE Shield, simply plug the shield into an Arduino, attach a LiPo battery to power the cellular module, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

Check out the [comprehensive wiki](https://github.com/botletics/LTE-Shield/wiki)! (Note: still under development)
You can also use [this Instructables](https://www.instructables.com/id/LTE-NB-IoT-Shield-for-Arduino/) if that works better for you!

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Adafruit FONA Library Updates
The code for this LTE shield is an altered library built upon the [Adafruit FONA library](https://github.com/adafruit/Adafruit_FONA) with added LTE functionality for the SIM7000 module. The following list is a summary of the updates:

### Confirmed functionalities
- GPS works even without a SIM card! The NMEA data includes UTC date/time, latitutde, longitude, altitude, etc and is quite accurate! It also obtains a GPS fix fairly quickly, only about 20s from cold start and even less (a couple seconds or so) when the device has already been on!
- Read a webpage via LTE CAT-M1
- Post to a web API via LTE CAT-M1

### To-Do List
- Test phone functionality (maybe on a later product version with voice support)
- Create an IoT example that posts temperature and GPS lat/long data to the cloud

### Completed Tasks
-	Included FONA_LTE class to the library for SIM7000A module
-	Included FONA_LTE_A and FONA_LTE_E types in .h file but not in .cpp declaration (I don't have a SIM7000E module with me so I don't know what its manufacturer-assigned name is)
-	Added “setBaudrate(uint16_t baud)” function for LTE class using "AT+IPR=<rate>". 
- Created "FONA_LTE_setbaud.ino" sketch to set baud rate to 4800 from default 115200 because at 115200 some parts of text would show up weird in the serial monitor.
- Added "hangUp()" function but still need to test with SIM card later
- Added "powerDown()" function using "AT+CPOWD=1" to turn off the SIM7000
- Tested the dual LTE/GNSS antenna signal strength. Inside (near the window) I get an RSSI around 31 (-52 dBm) for AT&T
- Tested HTTP commands with GPRS (2G)
- Tweaked the library so that the LTE declaration "Adafruit_FONA_LTE fona = Adafruit_FONA_LTE()" doesn't include the reset pin because the reset pin is reserved for emergencies only, according to the SIM7000 design document.
- Added a function "postData()" for posting data to dweet.io, a free cloud API. You can choose to use HTTP GET or POST and I have added an example in the "FONA_LTE_Test" sketch (enter "2" in the menu for the option)
- I have tested almost every AT command and they have all worked!
