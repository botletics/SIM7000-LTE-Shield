## Overview
The LTE Shield uses SIMCOM's SIM7000-series LTE modules which are low-power 4G LTE modules that support the new LTE CAT-M technology and also have integrated GNSS (GPS,GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. NB-IoT is achieved with the different versions, namely the SIM7000E, SIM7000C, and SIM7000E-N modules. To use the LTE Shield, simply plug the shield into an Arduino, attach a LiPo battery to power the cellular module, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

Check out the [comprehensive wiki](https://github.com/botletics/LTE-Shield/wiki)! (Note: still under development)

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Adafruit FONA Library Updates
The code for this LTE shield is an altered library built upon the [Adafruit FONA library](https://github.com/adafruit/Adafruit_FONA) with added LTE functionality for the SIM7000 module. The following list is a summary of the updates:

### Tested functionalities
-	Read ADC voltage (“AT+CADC?”)
-	

### To-Do List
-	Change “AT+CVHU=0” at the beginning to “AT+CHUP” for LTE class
-	

#### Completed Tasks
-	Included FONA_LTE class
-	Included FONA_LTE_A and FONA_LTE_E types
-	Added “setBaudrate(uint16_t baud)” function
-	Updated FONA3G_setbaud function to include FONA_LTE definition
