/*  This is an example sketch to test the core functionalities of SIMCom-based cellular modules.
    This code supports the SIM7000-series modules (LTE/NB-IoT shields) for low-power IoT devices!

    Note: this code is specifically meant for AVR microcontrollers (Arduino Uno, Mega, Leonardo, etc)
    However, if you are using an ESP8266 please make the minor modifications mentioned below in the
    comments for the pin definitions and software serial initialization.

    For ESP32 please use the ESP32_LTE_Demo instead: https://github.com/botletics/SIM7000-LTE-Shield/blob/master/Code/examples/ESP32_LTE_Demo/ESP32_LTE_Demo.ino

    Author: Timothy Woo (www.botletics.com)
    Github: https://github.com/botletics/SIM7000-LTE-Shield
    Last Updated: 8/1/2019
    License: GNU GPL v3.0
*/

/******* ORIGINAL ADAFRUIT FONA LIBRARY TEXT *******/
/***************************************************
  This is an example for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963
  ----> http://www.adafruit.com/products/2468
  ----> http://www.adafruit.com/products/2542

  These cellular modules use TTL Serial to communicate, 2 pins are
  required to interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_FONA.h"

// Define *one* of the following lines:
//#define SIMCOM_2G // SIM800/808/900/908, etc.
//#define SIMCOM_3G // SIM5320A/E
#define SIMCOM_7000 // SIM7000A/C/E/G
//#define SIMCOM_7500 // SIM7500A/E

// For TinySine SIM5320 shield
//#define FONA_PWRKEY 8
//#define FONA_RST 9
//#define FONA_TX 2 // Microcontroller RX (note: won't work on Mega)
//#define FONA_RX 3 // Microcontroller TX

// ESP8266 + SIM7000 shield
//#define FONA_PWRKEY 14 // D5 on NodeMCU
//#define FONA_RST 12 // D6 on NodeMCU
//#define FONA_TX 4 // D2 on NodeMCU, microcontroller RX
//#define FONA_RX 5 // D1 on NodeMCU, microcontroller TX

// For SIM7000 shield
#define FONA_PWRKEY 6
#define FONA_RST 7
//#define FONA_DTR 8 // Connect with solder jumper
//#define FONA_RI 9 // Need to enable via AT commands
#define FONA_TX 10 // Microcontroller RX
#define FONA_RX 11 // Microcontroller TX
//#define T_ALERT 12 // Connect with solder jumper

// For SIM7500 shield
//#define FONA_PWRKEY 6
//#define FONA_RST 7
////#define FONA_DTR 9 // Connect with solder jumper
////#define FONA_RI 8 // Need to enable via AT commands
//#define FONA_TX 11 // Microcontroller RX
//#define FONA_RX 10 // Microcontroller TX
////#define T_ALERT 5 // Connect with solder jumper

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);

// Use the following line for ESP8266 instead of the line above (comment out the one above)
//SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX, false, 256); // TX, RX, inverted logic, buffer size

SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//HardwareSerial *fonaSerial = &Serial1;

// Use this for 2G modules
#ifdef SIMCOM_2G
  Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
  
// Use this one for 3G modules
#elif defined(SIMCOM_3G)
  Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);
  
// Use this one for LTE CAT-M/NB-IoT modules (like SIM7000)
// Notice how we don't include the reset pin because it's reserved for emergencies on the LTE module!
#elif defined(SIMCOM_7000) || defined(SIMCOM_7500)
  Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();
#endif

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!

void setup() {
  //  while (!Serial);

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  pinMode(FONA_PWRKEY, OUTPUT);

  // Turn on the module by pulsing PWRKEY low for a little bit
  // This amount of time depends on the specific module that's used
  powerOn(); // See function definition at the very end of the sketch

  Serial.begin(9600);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing....(May take several seconds)"));

  // SIM7000 takes about 3s to turn on but SIM7500 takes about 15s
  // Press reset button if the module is still turning on and the board doesn't find it.
  // When the module is on it should communicate right after pressing reset
  
  fonaSS.begin(115200); // Default SIM7000 shield baud rate

  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600"); // Set baud rate
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600);
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // Don't proceed if it couldn't find the device
  }
  

  // The commented block of code below is an alternative that will find the module at 115200
  // Then switch it to 4800 without having to wait for the module to turn on and manually
  // press the reset button in order to establish communication. However, once the baud is set
  // this method will be much slower.
  /*
  fonaSerial->begin(115200); // Default LTE shield baud rate
  fona.begin(*fonaSerial); // Don't use if statement because an OK reply could be sent incorrectly at 115200 baud

  Serial.println(F("Configuring to 9600 baud"));
  fona.setBaudrate(9600); // Set to 4800 baud
  fonaSerial->begin(9600);
  if (!fona.begin(*fonaSerial)) {
  Serial.println(F("Couldn't find modem"));
  while(1); // Don't proceed if it couldn't find the device
  }
  */

  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case SIM800L:
      Serial.println(F("SIM800L")); break;
    case SIM800H:
      Serial.println(F("SIM800H")); break;
    case SIM808_V1:
      Serial.println(F("SIM808 (v1)")); break;
    case SIM808_V2:
      Serial.println(F("SIM808 (v2)")); break;
    case SIM5320A:
      Serial.println(F("SIM5320A (American)")); break;
    case SIM5320E:
      Serial.println(F("SIM5320E (European)")); break;
    case SIM7000A:
      Serial.println(F("SIM7000A (American)")); break;
    case SIM7000C:
      Serial.println(F("SIM7000C (Chinese)")); break;
    case SIM7000E:
      Serial.println(F("SIM7000E (European)")); break;
    case SIM7000G:
      Serial.println(F("SIM7000G (Global)")); break;
    case SIM7500A:
      Serial.println(F("SIM7500A (American)")); break;
    case SIM7500E:
      Serial.println(F("SIM7500E (European)")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

  // Set modem to full functionality
  fona.setFunctionality(1); // AT+CFUN=1

  // Configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setNetworkSettings(F("your APN"), F("your username"), F("your password"));
  //fona.setNetworkSettings(F("m2m.com.attz")); // For AT&T IoT SIM card
  //fona.setNetworkSettings(F("telstra.internet")); // For Telstra (Australia) SIM card - CAT-M1 (Band 28)
  fona.setNetworkSettings(F("hologram")); // For Hologram SIM card

  // Optionally configure HTTP gets to follow redirects over SSL.
  // Default is not to follow SSL redirects, however if you uncomment
  // the following line then redirects over SSL will be followed.
  //fona.setHTTPSRedirect(true);

  /*
  // Other examples of some things you can set:
  fona.setPreferredMode(38); // Use LTE only, not 2G
  fona.setPreferredLTEMode(1); // Use LTE CAT-M only, not NB-IoT
  fona.setOperatingBand("CAT-M", 12); // AT&T uses band 12
//  fona.setOperatingBand("CAT-M", 13); // Verizon uses band 13
  fona.enableRTC(true);
  
  fona.enableSleepMode(true);
  fona.set_eDRX(1, 4, "0010");
  fona.enablePSM(true);

  // Set the network status LED blinking pattern while connected to a network (see AT+SLEDS command)
  fona.setNetLED(true, 2, 64, 3000); // on/off, mode, timer_on, timer_off
  fona.setNetLED(false); // Disable network status LED
  */

  printMenu();
}

void printMenu(void) {
  Serial.println(F("-------------------------------------"));
  // General
  Serial.println(F("[?] Print this menu"));
  Serial.println(F("[a] Read the ADC; 2.8V max for SIM800/808, 0V-VBAT for SIM7000 shield"));
  Serial.println(F("[b] Read supply voltage")); // Will also give battery % charged for most modules
  Serial.println(F("[C] Read the SIM CCID"));
  Serial.println(F("[U] Unlock SIM with PIN code"));
  Serial.println(F("[i] Read signal strength (RSSI)"));
  Serial.println(F("[n] Get network status"));
  Serial.println(F("[1] Get network connection info")); // See what connection type and band you're on!

#ifndef SIMCOM_7000
  // Audio
  Serial.println(F("[v] Set audio Volume"));
  Serial.println(F("[V] Get volume"));
  Serial.println(F("[H] Set headphone audio (SIM800/808)"));
  Serial.println(F("[e] Set external audio (SIM800/808)"));
  Serial.println(F("[T] Play audio Tone"));
  Serial.println(F("[P] PWM/buzzer out (SIM800/808)"));

  // Calling
  Serial.println(F("[c] Make phone Call"));
  Serial.println(F("[A] Get call status"));
  Serial.println(F("[h] Hang up phone"));
  Serial.println(F("[p] Pick up phone"));
#endif

#ifdef SIMCOM_2G
  // FM (SIM800 only!)
  Serial.println(F("[f] Tune FM radio (SIM800)"));
  Serial.println(F("[F] Turn off FM (SIM800)"));
  Serial.println(F("[m] Set FM volume (SIM800)"));
  Serial.println(F("[M] Get FM volume (SIM800)"));
  Serial.println(F("[q] Get FM station signal level (SIM800)"));
#endif

  // SMS
  Serial.println(F("[N] Number of SMS's"));
  Serial.println(F("[r] Read SMS #"));
  Serial.println(F("[R] Read all SMS"));
  Serial.println(F("[d] Delete SMS #"));
  Serial.println(F("[s] Send SMS"));
  Serial.println(F("[u] Send USSD"));
  
  // Time
  Serial.println(F("[y] Enable local time stamp (SIM800/808/7000)"));
  Serial.println(F("[Y] Enable NTP time sync (SIM800/808/7000)")); // Need to use "G" command first!
  Serial.println(F("[t] Get network time")); // Works just by being connected to network

  // Data Connection
  Serial.println(F("[G] Enable cellular data"));
  Serial.println(F("[g] Disable cellular data"));
  Serial.println(F("[l] Query GSMLOC (2G)"));
#if !defined(SIMCOM_3G) && !defined(SIMCOM_7500) 
  Serial.println(F("[w] Read webpage"));
  Serial.println(F("[W] Post to website"));
#endif
  // The following option below posts dummy data to dweet.io for demonstration purposes. See the
  // IoT_example sketch for an actual application of this function!
  Serial.println(F("[2] Post to dweet.io via 2G / LTE CAT-M / NB-IoT")); // This can be SIM800/808/900/7000
  Serial.println(F("[3] Post to dweet.io via 3G / 4G LTE")); // SIM5320/7500

  // GPS
  if ((type == SIM5320A) || (type == SIM5320E) || (type == SIM808_V1) || (type == SIM808_V2) || 
      (type == SIM7000A) || (type == SIM7000C) || (type == SIM7000E) || (type == SIM7000G) ||
      (type == SIM7500A) || (type == SIM7500E)) {
    Serial.println(F("[O] Turn GPS on (SIM808/5320/7000)"));
    Serial.println(F("[o] Turn GPS off (SIM808/5320/7000)"));
    Serial.println(F("[L] Query GPS location (SIM808/5320/7000)"));
    if (type == SIM808_V1) {
      Serial.println(F("[x] GPS fix status (FONA808 v1 only)"));
    }
    Serial.println(F("[E] Raw NMEA out (SIM808)"));
  }
  
  Serial.println(F("[S] Create serial passthru tunnel"));
  Serial.println(F("-------------------------------------"));
  Serial.println(F(""));
}

void loop() {
  Serial.print(F("FONA> "));
  while (! Serial.available() ) {
    if (fona.available()) {
      Serial.write(fona.read());
    }
  }

  char command = Serial.read();
  Serial.println(command);


  switch (command) {
    case '?': {
        printMenu();
        break;
      }

    case 'a': {
        // read the ADC
        uint16_t adc;
        if (! fona.getADCVoltage(&adc)) {
          Serial.println(F("Failed to read ADC"));
        } else {
          Serial.print(F("ADC = ")); Serial.print(adc); Serial.println(F(" mV"));
        }
        break;
      }

    case 'b': {
        // read the battery voltage and percentage
        uint16_t vbat;
        if (! fona.getBattVoltage(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
        }

        if ( (type != SIM7500A) && (type != SIM7500E) ) {
          if (! fona.getBattPercent(&vbat)) {
            Serial.println(F("Failed to read Batt"));
          } else {
            Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
          }
        }

        break;
      }

    case 'U': {
        // Unlock the SIM with a PIN code
        char PIN[5];
        flushSerial();
        Serial.println(F("Enter 4-digit PIN"));
        readline(PIN, 3);
        Serial.println(PIN);
        Serial.print(F("Unlocking SIM card: "));
        if (! fona.unlockSIM(PIN)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }

    case 'C': {
        // read the CCID
        fona.getSIMCCID(replybuffer);  // make sure replybuffer is at least 21 bytes!
        Serial.print(F("SIM CCID = ")); Serial.println(replybuffer);
        break;
      }

    case 'i': {
        // read the RSSI
        uint8_t n = fona.getRSSI();
        int8_t r;

        Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
        if (n == 0) r = -115;
        if (n == 1) r = -111;
        if (n == 31) r = -52;
        if ((n >= 2) && (n <= 30)) {
          r = map(n, 2, 30, -110, -54);
        }
        Serial.print(r); Serial.println(F(" dBm"));

        break;
      }

    case 'n': {
        // read the network/cellular status
        uint8_t n = fona.getNetworkStatus();
        Serial.print(F("Network status "));
        Serial.print(n);
        Serial.print(F(": "));
        if (n == 0) Serial.println(F("Not registered"));
        if (n == 1) Serial.println(F("Registered (home)"));
        if (n == 2) Serial.println(F("Not registered (searching)"));
        if (n == 3) Serial.println(F("Denied"));
        if (n == 4) Serial.println(F("Unknown"));
        if (n == 5) Serial.println(F("Registered roaming"));
        break;
      }
    case '1': {
        // Get connection type, cellular band, carrier name, etc.
        fona.getNetworkInfo();        
        break;
      }

#ifndef SIMCOM_7000
    /*** Audio ***/
    case 'v': {
        // set volume
        flushSerial();
        if ( (type == SIM5320A) || (type == SIM5320E) ) {
          Serial.print(F("Set Vol [0-8] "));
        } else if ( (type == SIM7500A) || (type == SIM7500E) ) {
          Serial.print(F("Set Vol [0-5] "));
        } else {
          Serial.print(F("Set Vol % [0-100] "));
        }
        uint8_t vol = readnumber();
        Serial.println();
        if (! fona.setVolume(vol)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }

    case 'V': {
        uint8_t v = fona.getVolume();
        Serial.print(v);
        if ( (type == SIM5320A) || (type == SIM5320E) ) {
          Serial.println(" / 8");
        } else if ( (type == SIM7500A) || (type == SIM7500E) ) { // Don't write anything for SIM7500
          Serial.println();
        } else {
          Serial.println("%");
        }
        break;
      }

    case 'H': {
        // Set Headphone output
        if (! fona.setAudio(FONA_HEADSETAUDIO)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        fona.setMicVolume(FONA_HEADSETAUDIO, 15);
        break;
      }
    case 'e': {
        // Set External output
        if (! fona.setAudio(FONA_EXTAUDIO)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }

        fona.setMicVolume(FONA_EXTAUDIO, 10);
        break;
      }

    case 'T': {
        // play tone
        flushSerial();
        Serial.print(F("Play tone #"));
        uint8_t kittone = readnumber();
        Serial.println();
        // play for 1 second (1000 ms)
        if (! fona.playToolkitTone(kittone, 1000)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }

    /*** PWM ***/

    case 'P': {
        // PWM Buzzer output @ 2KHz max
        flushSerial();
        Serial.print(F("PWM Freq, 0 = Off, (1-2000): "));
        uint16_t freq = readnumber();
        Serial.println();
        if (! fona.setPWM(freq)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }

    /*** Calling ***/
    case 'c': {
        // call a phone!
        char number[30];
        flushSerial();
        Serial.print(F("Call #"));
        readline(number, 30);
        Serial.println();
        Serial.print(F("Calling ")); Serial.println(number);
        if (!fona.callPhone(number)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }

        break;
      }
    case 'A': {
        // get call status
        int8_t callstat = fona.getCallStatus();
        switch (callstat) {
          case 0: Serial.println(F("Ready")); break;
          case 1: Serial.println(F("Could not get status")); break;
          case 3: Serial.println(F("Ringing (incoming)")); break;
          case 4: Serial.println(F("Ringing/in progress (outgoing)")); break;
          default: Serial.println(F("Unknown")); break;
        }
        break;
      }
      
    case 'h': {
        // hang up!
        if (! fona.hangUp()) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }

    case 'p': {
        // pick up!
        if (! fona.pickUp()) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }
#endif

#ifdef SIMCOM_2G
    /*** FM Radio ***/

    case 'f': {
        // get freq
        flushSerial();
        Serial.print(F("FM Freq (eg 1011 == 101.1 MHz): "));
        uint16_t station = readnumber();
        Serial.println();
        // FM radio ON using headset
        if (fona.FMradio(true, FONA_HEADSETAUDIO)) {
          Serial.println(F("Opened"));
        }
        if (! fona.tuneFMradio(station)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Tuned"));
        }
        break;
      }
    case 'F': {
        // FM radio off
        if (! fona.FMradio(false)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }
    case 'm': {
        // Set FM volume.
        flushSerial();
        Serial.print(F("Set FM Vol [0-6]:"));
        uint8_t vol = readnumber();
        Serial.println();
        if (!fona.setFMVolume(vol)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
        }
        break;
      }
    case 'M': {
        // Get FM volume.
        uint8_t fmvol = fona.getFMVolume();
        if (fmvol < 0) {
          Serial.println(F("Failed"));
        } else {
          Serial.print(F("FM volume: "));
          Serial.println(fmvol, DEC);
        }
        break;
      }
    case 'q': {
        // Get FM station signal level (in decibels).
        flushSerial();
        Serial.print(F("FM Freq (eg 1011 == 101.1 MHz): "));
        uint16_t station = readnumber();
        Serial.println();
        int8_t level = fona.getFMSignalLevel(station);
        if (level < 0) {
          Serial.println(F("Failed! Make sure FM radio is on (tuned to station)."));
        } else {
          Serial.print(F("Signal level (dB): "));
          Serial.println(level, DEC);
        }
        break;
      }
#endif

    /*** SMS ***/

    case 'N': {
        // read the number of SMS's!
        int8_t smsnum = fona.getNumSMS();
        if (smsnum < 0) {
          Serial.println(F("Could not read # SMS"));
        } else {
          Serial.print(smsnum);
          Serial.println(F(" SMS's on SIM card!"));
        }
        break;
      }
    case 'r': {
        // read an SMS
        flushSerial();
        Serial.print(F("Read #"));
        uint8_t smsn = readnumber();
        Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);

        // Retrieve SMS sender address/phone number.
        if (! fona.getSMSSender(smsn, replybuffer, 250)) {
          Serial.println("Failed!");
          break;
        }
        Serial.print(F("FROM: ")); Serial.println(replybuffer);

        // Retrieve SMS value.
        uint16_t smslen;
        if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
          Serial.println("Failed!");
          break;
        }
        Serial.print(F("***** SMS #")); Serial.print(smsn);
        Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
        Serial.println(replybuffer);
        Serial.println(F("*****"));

        break;
      }
    case 'R': {
        // read all SMS
        int8_t smsnum = fona.getNumSMS();
        uint16_t smslen;
        int8_t smsn;

        if ( (type == SIM5320A) || (type == SIM5320E) ) {
          smsn = 0; // zero indexed
          smsnum--;
        } else {
          smsn = 1;  // 1 indexed
        }

        for ( ; smsn <= smsnum; smsn++) {
          Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
          if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
            Serial.println(F("Failed!"));
            break;
          }
          // if the length is zero, its a special case where the index number is higher
          // so increase the max we'll look at!
          if (smslen == 0) {
            Serial.println(F("[empty slot]"));
            smsnum++;
            continue;
          }

          Serial.print(F("***** SMS #")); Serial.print(smsn);
          Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
          Serial.println(replybuffer);
          Serial.println(F("*****"));
        }
        break;
      }

    case 'd': {
        // delete an SMS
        flushSerial();
        Serial.print(F("Delete #"));
        uint8_t smsn = readnumber();

        Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
        if (fona.deleteSMS(smsn)) {
          Serial.println(F("OK!"));
        } else {
          Serial.println(F("Couldn't delete"));
        }
        break;
      }

    case 's': {
        // send an SMS!
        char sendto[21], message[141];
        flushSerial();
        Serial.print(F("Send to #"));
        readline(sendto, 20);
        Serial.println(sendto);
        Serial.print(F("Type out one-line message (140 char): "));
        readline(message, 140);
        Serial.println(message);
        if (!fona.sendSMS(sendto, message)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }

        break;
      }

    case 'u': {
      // send a USSD!
      char message[141];
      flushSerial();
      Serial.print(F("Type out one-line message (140 char): "));
      readline(message, 140);
      Serial.println(message);

      uint16_t ussdlen;
      if (!fona.sendUSSD(message, replybuffer, 250, &ussdlen)) { // pass in buffer and max len!
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Sent!"));
        Serial.print(F("***** USSD Reply"));
        Serial.print(" ("); Serial.print(ussdlen); Serial.println(F(") bytes *****"));
        Serial.println(replybuffer);
        Serial.println(F("*****"));
      }
    }

    /*** Time ***/

    case 'y': {
        // enable network time sync
        if (!fona.enableRTC(true))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 'Y': {
        // enable NTP time sync
        if (!fona.enableNTPTimeSync(true, F("pool.ntp.org")))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 't': {
        // read the time
        char buffer[23];

        fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
        Serial.print(F("Time = ")); Serial.println(buffer);
        break;
      }


    /*********************************** GPS */

    case 'o': {
        // turn GPS off
        if (!fona.enableGPS(false))
          Serial.println(F("Failed to turn off"));
        break;
      }
    case 'O': {
        // turn GPS on
        if (!fona.enableGPS(true))
          Serial.println(F("Failed to turn on"));
        break;
      }
    case 'x': {
        int8_t stat;
        // check GPS fix
        stat = fona.GPSstatus();
        if (stat < 0)
          Serial.println(F("Failed to query"));
        if (stat == 0) Serial.println(F("GPS off"));
        if (stat == 1) Serial.println(F("No fix"));
        if (stat == 2) Serial.println(F("2D fix"));
        if (stat == 3) Serial.println(F("3D fix"));
        break;
      }

    case 'L': {
        /*
        // Uncomment this block if all you want to see is the AT command response
        // check for GPS location
        char gpsdata[120];
        fona.getGPS(0, gpsdata, 120);
        if (type == SIM808_V1)
          Serial.println(F("Reply in format: mode,longitude,latitude,altitude,utctime(yyyymmddHHMMSS),ttff,satellites,speed,course"));
        else if ( (type == SIM5320A) || (type == SIM5320E) || (type == SIM7500A) || (type == SIM7500E) )
          Serial.println(F("Reply in format: [<lat>],[<N/S>],[<lon>],[<E/W>],[<date>],[<UTC time>(yyyymmddHHMMSS)],[<alt>],[<speed>],[<course>]"));
        else
          Serial.println(F("Reply in format: mode,fixstatus,utctime(yyyymmddHHMMSS),latitude,longitude,altitude,speed,course,fixmode,reserved1,HDOP,PDOP,VDOP,reserved2,view_satellites,used_satellites,reserved3,C/N0max,HPA,VPA"));
        
        Serial.println(gpsdata);

        break;
        */

        float latitude, longitude, speed_kph, heading, altitude, second;
        uint16_t year;
        uint8_t month, day, hour, minute;

        // Use the top line if you want to parse UTC time data as well, the line below it if you don't care
//        if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude, &year, &month, &day, &hour, &minute, &second)) {
        if (fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) { // Use this line instead if you don't want UTC time
          Serial.println(F("---------------------"));
          Serial.print(F("Latitude: ")); Serial.println(latitude, 6);
          Serial.print(F("Longitude: ")); Serial.println(longitude, 6);
          Serial.print(F("Speed: ")); Serial.println(speed_kph);
          Serial.print(F("Heading: ")); Serial.println(heading);
          Serial.print(F("Altitude: ")); Serial.println(altitude);
          // Comment out the stuff below if you don't care about UTC time
          /*
          Serial.print(F("Year: ")); Serial.println(year);
          Serial.print(F("Month: ")); Serial.println(month);
          Serial.print(F("Day: ")); Serial.println(day);
          Serial.print(F("Hour: ")); Serial.println(hour);
          Serial.print(F("Minute: ")); Serial.println(minute);
          Serial.print(F("Second: ")); Serial.println(second);
          Serial.println(F("---------------------"));
          */
        }

        break;
      }

    case 'E': {
        flushSerial();
        if (type == SIM808_V1) {
          Serial.print(F("GPS NMEA output sentences (0 = off, 34 = RMC+GGA, 255 = all)"));
        } else {
          Serial.print(F("On (1) or Off (0)? "));
        }
        uint8_t nmeaout = readnumber();

        // turn on NMEA output
        fona.enableGPSNMEA(nmeaout);

        break;
      }

    /*********************************** GPRS */

    case 'g': {
        // turn GPRS off
        if (!fona.enableGPRS(false))
          Serial.println(F("Failed to turn off"));
        break;
      }
    case 'G': {
        // turn GPRS off first for SIM7500
        #ifdef SIMCOM_7500
          fona.enableGPRS(false);
        #endif
        
        // turn GPRS on
        if (!fona.enableGPRS(true))
          Serial.println(F("Failed to turn on"));
        break;
      }
    case 'l': {
        // check for GSMLOC (requires GPRS)
        uint16_t returncode;

        if (!fona.getGSMLoc(&returncode, replybuffer, 250))
          Serial.println(F("Failed!"));
        if (returncode == 0) {
          Serial.println(replybuffer);
        } else {
          Serial.print(F("Fail code #")); Serial.println(returncode);
        }

        break;
      }

#if !defined(SIMCOM_3G) && !defined(SIMCOM_7500)
    // The code below was written by Adafruit and only works on some modules
    case 'w': {
        // read website URL
        uint16_t statuscode;
        int16_t length;
        char url[80];

        flushSerial();
        Serial.println(F("URL to read (e.g. dweet.io/get/latest/dweet/for/sim7500test123):"));
        Serial.print(F("http://")); readline(url, 79);
        Serial.println(url);

        Serial.println(F("****"));
        if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
          Serial.println("Failed!");
          break;
        }
        while (length > 0) {
          while (fona.available()) {
            char c = fona.read();

            // Serial.write is too slow, we'll write directly to Serial register!
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
            loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
            UDR0 = c;
#else
            Serial.write(c);
#endif
            length--;
            if (! length) break;
          }
        }
        Serial.println(F("\n****"));
        fona.HTTP_GET_end();
        break;
      }

    case 'W': {
        // Post data to website
        uint16_t statuscode;
        int16_t length;
        char url[80];
        char data[80];

        flushSerial();
        Serial.println(F("NOTE: in beta! Use simple websites to post!"));
        Serial.println(F("URL to post (e.g. httpbin.org/post):"));
        Serial.print(F("http://")); readline(url, 79);
        Serial.println(url);
        Serial.println(F("Data to post (e.g. \"foo\" or \"{\"simple\":\"json\"}\"):"));
        readline(data, 79);
        Serial.println(data);

        Serial.println(F("****"));
        if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)) {
          Serial.println("Failed!");
          break;
        }
        while (length > 0) {
          while (fona.available()) {
            char c = fona.read();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
            loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
            UDR0 = c;
#else
            Serial.write(c);
#endif

            length--;
            if (! length) break;
          }
        }
        Serial.println(F("\n****"));
        fona.HTTP_POST_end();
        break;
      }
#endif

#if defined(SIMCOM_2G) || defined(SIMCOM_7000)
    case '2': {
        // Post data to website via 2G or LTE CAT-M/NB-IoT
        float temperature = analogRead(A0)*1.23; // Change this to suit your needs
        
        // Voltage in mV, just for testing. Use the read battery function instead.
        // Please note that for the LTE shield the voltage read will always be around 3.6V
        // because the SIM7000 is powered by a 3.6V regulator. If you want to monitor the
        // power source to the Arduino you will have to use something else.
        uint16_t battLevel = 3600;

        // Create char buffers for the floating point numbers for sprintf
        // Make sure these buffers are long enough for your request URL
        char URL[150];
        char body[100];
        char tempBuff[16];
        char battLevelBuff[16];
      
        // Format the floating point numbers as needed
        dtostrf(temperature, 1, 2, tempBuff); // float_val, min_width, digits_after_decimal, char_buffer
        dtostrf(battLevel, 1, 0, battLevelBuff);

        // Construct the appropriate URL's and body, depending on request type
        // Use IMEI as device ID for this example
        
        // GET request
        sprintf(URL, "dweet.io/dweet/for/%s?temp=%s&batt=%s", imei, tempBuff, battLevelBuff); // No need to specify http:// or https://
//        sprintf(URL, "http://dweet.io/dweet/for/%s?temp=%s&batt=%s", imei, tempBuff, battLevelBuff); // But this works too

        if (!fona.postData("GET", URL))
          Serial.println(F("Failed to complete HTTP GET..."));
        
        // POST request
        /*
        sprintf(URL, "http://dweet.io/dweet/for/%s", imei);
        sprintf(body, "{\"temp\":%s,\"batt\":%s}", tempBuff, battLevelBuff);
        
        if (!fona.postData("POST", URL, body)) // Can also add authorization token parameter!
          Serial.println(F("Failed to complete HTTP POST..."));
        */
      
        break;
      }
#endif

#if defined(SIMCOM_3G) || defined(SIMCOM_7500)
    case '3': {
        // Post data to website via 3G or 4G LTE
        float temperature = analogRead(A0) * 1.23; // Change this to suit your needs

        // Voltage in mV, just for testing. Use the read battery function instead for real applications.
        uint16_t battLevel = 3700;

        // Create char buffers for the floating point numbers for sprintf
        // Make sure these buffers are long enough for your request URL
        char URL[150];
        char tempBuff[16];
        char battLevelBuff[16];

        // Format the floating point numbers as needed
        dtostrf(temperature, 1, 2, tempBuff); // float_val, min_width, digits_after_decimal, char_buffer
        dtostrf(battLevel, 1, 0, battLevelBuff);

        // Construct the appropriate URL's and body, depending on request type
        // Use IMEI as device ID for this example

        // GET request
        sprintf(URL, "GET /dweet/for/%s?temp=%s&batt=%s HTTP/1.1\r\nHost: dweet.io\r\n\r\n", imei, tempBuff, battLevelBuff);

        if (!fona.postData("www.dweet.io", 443, "HTTPS", URL)) // Server, port, connection type, URL
          Serial.println(F("Failed to complete HTTP/HTTPS request..."));

        /*
        // POST request
        // You could also try a POST request to something like Ubidots!
        // Make sure to change the device name and token to test it!
        sprintf(URL, "POST /api/v1.6/devices/YOUR_DEVICE_NAME HTTP/1.1\r\nHost: things.ubidots.com\r\nX-Auth-Token: YOUR_TOKEN\r\nContent-Type: application/json\r\nContent-Length: 20\r\n\r\n{\"temperature\":100}\r\n");

        if (!fona.postData("things.ubidots.com", 443, "HTTPS", URL)) // Server, port, connection type, URL
          Serial.println(F("Failed to complete HTTP/HTTPS request..."));
        */
        
        break;
      }
#endif
    /*****************************************/

    case 'S': {
        Serial.println(F("Creating SERIAL TUBE"));
        while (1) {
          while (Serial.available()) {
            delay(1);
            fona.write(Serial.read());
          }
          if (fona.available()) {
            Serial.write(fona.read());
          }
        }
        break;
      }

    default: {
        Serial.println(F("Unknown command"));
        printMenu();
        break;
      }
  }
  // flush input
  flushSerial();
  while (fona.available()) {
    Serial.write(fona.read());
  }

}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
  // See spec sheets for your particular module
  #if defined(SIMCOM_2G)
    delay(1050);
  #elif defined(SIMCOM_3G)
    delay(180); // For SIM5320
  #elif defined(SIMCOM_7000)
    delay(100); // For SIM7000
  #elif defined(SIMCOM_7500)
    delay(500); // For SIM7500
  #endif
  
  digitalWrite(FONA_PWRKEY, HIGH);
}
