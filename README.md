## Overview
The LTE Shield uses SIMCOM's SIM7000-series LTE modules which are low-power 4G LTE modules that support the new LTE CAT-M technology and also have integrated GNSS (GPS,GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. NB-IoT is achieved with the different versions, namely the SIM7000E, SIM7000C, and SIM7000E-N modules. To use the LTE Shield, simply plug the shield into an Arduino, attach a LiPo battery to power the cellular module, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

Check out the [comprehensive wiki](https://github.com/botletics/LTE-Shield/wiki)! (Note: still under development)

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Adafruit FONA Library Updates
The code for this LTE shield is an altered library built upon the [Adafruit FONA library](https://github.com/adafruit/Adafruit_FONA) with added LTE functionality for the SIM7000 module. The following list is a summary of the updates:

### Confirmed functionalities
-	Read ADC voltage (“AT+CADC?”)
-	Read battery voltage and percentage ("AT+CBC")
- Read signal RSSI ("AT+CSQ"), works even without SIM card plugged in!
- Get network status ("AT+CREG?")

### Limitations
- Can't play audio tone or drive buzzer
- 

### To-Do List
-	Change “AT+CVHU=0” at the beginning to “AT+CHUP” for LTE class
-	Test SIM card-related functionalities
- Test audio functionalities
- Test web connection and HTTP/HTTPS
- Add "ATM<value>" and "ATL<value>" command for monitor mode and monitor volume
- 

#### Completed Tasks
-	Included FONA_LTE class to the library for SIM7000A module
-	Included FONA_LTE_A and FONA_LTE_E types in .h file but not in .cpp declaration (I don't have a SIM7000E module with me so I don't know what its manufacturer-assigned name is)
-	Added “setBaudrate(uint16_t baud)” function for LTE class using "AT+IPR=<rate>". 
- Created "FONA_LTE_setbaud.ino" sketch to set baud rate to 4800 from default 115200. At 115200 some data was being lost.
- 
