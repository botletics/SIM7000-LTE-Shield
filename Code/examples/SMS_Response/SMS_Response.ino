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

/*
THIS CODE IS STILL IN PROGRESS!

Open up the serial console on the Arduino at 115200 baud to interact with FONA


This code will receive an SMS, identify the sender's phone number, and automatically send a response

*/

#include "Adafruit_FONA.h"

// Default
//#define FONA_RX 2
//#define FONA_TX 3
//#define FONA_RST 4

// For LTE shield v3
#define FONA_PWRKEY 3
//#define FONA_DTR 4 // Can be used to wake up SIM7000 from sleep
#define FONA_RI 5 // Need to enable via AT commands
#define FONA_RX 7
#define FONA_TX 6
#define FONA_RST 8
//#define T_ALERT 12 // Connect with solder jumper

// For LTE shield v4
//#define FONA_PWRKEY 6
//#define FONA_RST 7
////#define FONA_DTR 8 // Connect with solder jumper
////#define FONA_RI 9 // Need to enable via AT commands
//#define FONA_TX 10 // Microcontroller RX
//#define FONA_RX 11 // Microcontroller TX
////#define T_ALERT 12 // Connect with solder jumper

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

// Use this for FONA 800 and 808s
//Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
// Use this one for FONA 3G
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);
// Use this one for FONA LTE
// Notice how we don't include the reset pin because it's reserved for emergencies on the LTE module!
Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!

void setup() {
  while (!Serial);

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state
  
  pinMode(FONA_PWRKEY, OUTPUT);
  // Turn on the SIM7000 by pulsing PWRKEY low for at least 72ms
  // This won't hurt even if the module is already on, because you need
  // to pulse PWRKEY for at least 1.2s to turn it off!
  pinMode(FONA_PWRKEY, LOW);
  delay(100);
  pinMode(FONA_PWRKEY, HIGH);

  Serial.begin(115200);
  Serial.println(F("FONA SMS caller ID test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  // The baud rate always resets back to default (115200) after
  // being powered down so let's try 115200 first. Hats off to
  // anyone who can figure out how to make it remember the new
  // baud rate even after being power cycled! If you are using
  // hardware serial then this shouldn't be an issue because
  // you can just use the default 115200 baud.
  fonaSerial->begin(115200); // Default LTE shield baud rate
  fona.begin(*fonaSerial); // Don't use if statement because an OK reply could be sent incorrectly at 115200 baud
  // If you are using hardware serial you can uncomment the lines below
  // and comment the one right above
//  if (!fona.begin(*fonaSerial)) { 
//    Serial.println(F("Couldn't find FONA at 115200 baud"));
//  }

  Serial.println(F("Configuring to 4800 baud"));
  fona.setBaudrate(4800); // Set to 4800 baud
  fonaSerial->begin(4800);
  if (!fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while(1); // Don't proceed if it couldn't find the device
  }
  
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    case FONA_LTE_A:
      Serial.println(F("FONA 4G LTE (American)")); break;
    case FONA_LTE_C:
      Serial.println(F("FONA 4G LTE (Chinese)")); break;
    case FONA_LTE_E:
      Serial.println(F("FONA 4G LTE (European)")); break;
    default: 
      Serial.println(F("???")); break;
  }
  
  // Print module IMEI number.
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }

//  // make it slow so its easy to read!
//  fonaSerial->begin(4800);
//  if (! fona.begin(*fonaSerial)) {
//    Serial.println(F("Couldn't find FONA"));
//    while(1);
//  }
//  Serial.println(F("FONA is OK"));
//
//  // Print SIM card IMEI number.
//  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
//  uint8_t imeiLen = fona.getIMEI(imei);
//  if (imeiLen > 0) {
//    Serial.print("SIM card IMEI: "); Serial.println(imei);
//  }

  fonaSerial->print("AT+CNMI=2,1\r\n");  //set up the FONA to send a +CMTI notification when an SMS is received

  Serial.println("FONA Ready");
}

  
char fonaNotificationBuffer[64];  //for notifications from the FONA
char smsBuffer[250];

void loop() {
  
  char* bufPtr = fonaNotificationBuffer;    //handy buffer pointer
  
  if (fona.available())      //any data available from the FONA?
  {
    int slot = 0;            //this will be the slot number of the SMS
    int charCount = 0;
    //Read the notification into fonaInBuffer
    do  {
      *bufPtr = fona.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaNotificationBuffer)-1)));
    
    //Add a terminal NULL to the notification string
    *bufPtr = 0;

    //Scan the notification string for an SMS received notification.
    //  If it's an SMS message, we'll get the slot number in 'slot'
    if (1 == sscanf(fonaNotificationBuffer, "+CMTI: " FONA_PREF_SMS_STORAGE ",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);
      
      char callerIDbuffer[32];  //we'll store the SMS sender number in here
      
      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(slot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);

        // Retrieve SMS value.
        uint16_t smslen;
        if (fona.readSMS(slot, smsBuffer, 250, &smslen)) { // pass in buffer and max len!
          Serial.println(smsBuffer);
        }

      //Send back an automatic response
      Serial.println("Sending reponse...");
      
      // Canned response:
//      if (!fona.sendSMS(callerIDbuffer, "Hey, I got your text!")) {
//        Serial.println(F("Failed"));
//      } else {
//        Serial.println(F("Sent!"));
//      }

      // Reply with sensor data instead!
      float sensorVal = analogRead(A0) * 1.123; // For testing
      char textMessage[100]; // Make sure this is long enough!
      char sensorBuff[16];
      dtostrf(sensorVal, 1, 2, sensorBuff); // float_val, min_width, digits_after_decimal, char_buffer
      strcpy(textMessage, "Sensor reading: ");
      strcat(textMessage, sensorBuff);
      
      if (!fona.sendSMS(callerIDbuffer, textMessage)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("Sent!"));
      }
      
      // delete the original msg after it is processed
      //   otherwise, we will fill up all the slots
      //   and then we won't be able to receive SMS anymore
      if (fona.deleteSMS(slot)) {
        Serial.println(F("OK!"));
      } else {
        Serial.print(F("Couldn't delete SMS in slot ")); Serial.println(slot);
        fona.print(F("AT+CMGD=?\r\n"));
      }
    }
  }
}
