## Overview
The LTE Shield uses SIMCOM's SIM7000-series LTE module which is a low-power 4G LTE module that supports the new LTE CAT-M1 and NB-IoT technology and also has integrated high-speed, multi-GNSS (GPS, GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. The shield can be used in different regions around the world simply by using the appropriate module version, either the SIM7000A (American), SIM7000C (Chinese), or SIM7000E (European). The shield also includes a high-accuracy I2C temperature sensor for IoT applications. To use the LTE Shield, simply plug the shield into an Arduino, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

You can buy the shield on [my website](https://www.botletics.com/products/sim7000-shield).

Check out the [comprehensive wiki](https://github.com/botletics/LTE-Shield/wiki)! (Note: still under development)
You can also use [this Instructables](https://www.instructables.com/id/LTE-NB-IoT-Shield-for-Arduino/) if that works better for you!

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Adafruit FONA Library Updates
The code for this LTE shield is an altered library built upon the [Adafruit FONA library](https://github.com/adafruit/Adafruit_FONA) with added LTE functionality for the SIM7000 module. The following list is a summary of the updates:

### Confirmed functionalities
- GPS works without a SIM card! The NMEA data includes UTC date/time, latitutde, longitude, altitude, etc and is quite accurate! It also obtains a GPS fix fairly quickly, only about 20-30s from cold start and even less (a couple seconds or so) when the device has already been on! However, this may differ based on your location and how deep you're buried inside a building.
- Read a webpage via LTE CAT-M1
- Post to a web API via LTE CAT-M1

### To-Do List
- Test phone functionality (maybe on a later product version with voice support)
- Test with Verizon SIM card
- Measure current consumption

### Completed Tasks
- 
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
