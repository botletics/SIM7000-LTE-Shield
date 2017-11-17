## Overview
The LTE Shield uses SIMCOM's SIM7000-series LTE modules which are low-power 4G LTE modules that support the new LTE CAT-M NB-IoT technology and also have integrated GNSS (GPS,GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. NB-IoT is achieved with the different versions, namely the SIM7000E, SIM7000C, and SIM7000E-N modules. To use the LTE Shield, simply plug the shield into an Arduino, attach a LiPo battery to power the cellular module, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

Check out the [comprehensive wiki](https://github.com/botletics/LTE-Shield/wiki)! (Note: still under development)

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Adafruit FONA Library Updates
The code for this LTE shield is an altered library built upon the [Adafruit FONA library](https://github.com/adafruit/Adafruit_FONA) with added LTE functionality for the SIM7000 module. The following list is a summary of the updates:

### Confirmed functionalities
-	Read ADC voltage (“AT+CADC?”)
-	Read battery voltage and percentage ("AT+CBC")
- Read signal RSSI ("AT+CSQ")
- SIM card ("AT+CCID")
- Get network status ("AT+CREG?")
- Set APN/bearer settings and enable/disable GPRS
- Get phone status ("AT+CPAS")
- Get system clock time ("AT+CCLK")
- Turn GPS on/off ("AT+CGPSPWR=<value>")
- GPS works even without a SIM card! The NMEA data includes UTC date/time, latitutde, longitude, altitude, etc and is quite accurate! It also obtains a GPS fix very quickly, only about 7s from cold start and even less (couple a second or two) when the device has already been on!
- Read webpage (tested with 
- Post to a webpage (tested with dweet.io)

### To-Do List
- Test phone
- Test SMS
- Test LTE/NB-IoT commands
- Add an IoT HTTP GET/POST function for cloud API's
- Create an IoT example that posts GPS lat/long data to the cloud

### Completed Tasks
-	Included FONA_LTE class to the library for SIM7000A module
-	Included FONA_LTE_A and FONA_LTE_E types in .h file but not in .cpp declaration (I don't have a SIM7000E module with me so I don't know what its manufacturer-assigned name is)
-	Added “setBaudrate(uint16_t baud)” function for LTE class using "AT+IPR=<rate>". 
- Created "FONA_LTE_setbaud.ino" sketch to set baud rate to 4800 from default 115200 because at 115200 some parts of text would show up weird in the serial monitor.
- Added "hangUp()" function but still need to test with SIM card later
- Added "powerDown()" function using "AT+CPOWD=1" to turn off the SIM7000
- Tested the dual LTE/GNSS antenna signal strength. Inside (near the window) I get an RSSI around 31 (-52 dBm) for AT&T
- Tested HTTP commands!
