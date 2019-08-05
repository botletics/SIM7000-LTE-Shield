/*  This example sketch is for the Botletics SIM7000 shield and Arduino
 *  to collect GPS, temperature, and battery data and send those values via MQTT
 *  to just about any MQTT broker. To test the MQTT subscribe feature, publish
 *  data to the topic the device is subscribed to (see the "SUB_TOPIC" variable)
 *  and it should print out in the serial monitor. Send "yes" to turn on the LED
 *  and "no" to turn it off!
 *  
 *  Just make sure to replace credentials with your own, and change the names of the
 *  topics you want to publish or subscribe to.
 *  
 *  Author: Timothy Woo (www.botletics.com)
 *  Github: https://github.com/botletics/SIM7000-LTE-Shield
 *  Last Updated: 8/5/2019
 *  License: GNU GPL v3.0
 */

#include "Adafruit_FONA.h" // https://github.com/botletics/SIM7000-LTE-Shield/tree/master/Code

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

/************************* PIN DEFINITIONS *********************************/
// For SIM7000 shield
#define FONA_PWRKEY 6
#define FONA_RST 7
//#define FONA_DTR 8 // Connect with solder jumper
//#define FONA_RI 9 // Need to enable via AT commands
#define FONA_TX 10 // Microcontroller RX
#define FONA_RX 11 // Microcontroller TX
//#define T_ALERT 12 // Connect with solder jumper

#define LED 13

#define samplingRate 30 // The time we want to delay after each post (in seconds)

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

// For ESP32 hardware serial use these lines instead
//#include <HardwareSerial.h>
//HardwareSerial fonaSS(1);

Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();

/************************* MQTT PARAMETERS *********************************/
#define MQTT_SERVER      "m10.cloudmqtt.com"
#define MQTT_PORT        16644
#define MQTT_USERNAME    "MQTT_USERNAME"
#define MQTT_PASSWORD    "MQTT_PASSWORD"

// Set topic names to publish and subscribe to
#define GPS_TOPIC       "location"
#define TEMP_TOPIC      "temperature"
#define BATT_TOPIC      "battery"
#define SUB_TOPIC       "command"     // Subscribe topic name

/****************************** OTHER STUFF ***************************************/
// For temperature sensor
#include <Wire.h>
#include "Adafruit_MCP9808.h"

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
char imei[16] = {0}; // Use this for device ID
uint8_t type;
uint16_t battLevel = 0; // Battery level (percentage)
float latitude, longitude, speed_kph, heading, altitude, second;
uint16_t year;
uint8_t month, day, hour, minute;
uint8_t counter = 0;
unsigned long timer = 0;
bool firstTime = true;
//char PIN[5] = "1234"; // SIM card PIN

// NOTE: Keep the buffer sizes as small as possible, espeially on
// Arduino Uno which doesn't have much computing power to handle
// large buffers. On Arduino Mega you shouldn't have to worry much.
char latBuff[12], longBuff[12], locBuff[50], speedBuff[12],
     headBuff[12], altBuff[12], tempBuff[12], battBuff[12];

char replybuffer[255]; // For reading stuff coming through UART, like subscribed topic messages

void setup() {
  Serial.begin(9600);
  Serial.println(F("*** SIM7000 MQTT Example ***"));

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  pinMode(FONA_PWRKEY, OUTPUT);
  powerOn(); // Power on the module
  moduleSetup(); // Establishes first-time serial comm and prints IMEI

  tempsensor.wake(); // Wake up the MCP9808 if it was sleeping
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find the MCP9808!");
    while (1);
  }

  // Unlock SIM card if needed
  // Remember to uncomment the "PIN" variable definition above
  /*
  if (!fona.unlockSIM(PIN)) {
    Serial.println(F("Failed to unlock SIM card"));
  }
  */

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

  // Perform first-time GPS/data setup if the shield is going to remain on,
  // otherwise these won't be enabled in loop() and it won't work!
  // Enable GPS
  while (!fona.enableGPS(true)) {
    Serial.println(F("Failed to turn on GPS, retrying..."));
    delay(2000); // Retry every 2s
  }
  Serial.println(F("Turned on GPS!"));
}

void loop() {
  if (firstTime || millis() - timer > samplingRate * 1000UL) {    
    // Connect to cell network and verify connection
    // If unsuccessful, keep retrying every 2s until a connection is made
    while (!netStatus()) {
      Serial.println(F("Failed to connect to cell network, retrying..."));
      delay(2000); // Retry every 2s
    }
    Serial.println(F("Connected to cell network!"));
  
    // Disable data just to make sure it was actually off so that we can turn it on
  //  fona.openWirelessConnection(false);
  
    // Open wireless connection if not already activated
    if (!fona.wirelessConnStatus()) {
      while (!fona.openWirelessConnection(true)) {
        Serial.println(F("Failed to enable connection, retrying..."));
        delay(2000); // Retry every 2s
      }
      Serial.println(F("Enabled data!"));
    }
    else {
      Serial.println(F("Data already enabled!"));
    }
  
    // Measure battery level
    // Note: on the LTE shield this won't be accurate because the SIM7000
    // is supplied by a regulated 3.6V, not directly from the battery. You
    // can use the Arduino and a voltage divider to measure the battery voltage
    // and use that instead, but for now we will use the function below
    // only for testing.
    battLevel = readVcc(); // Get voltage in mV
  
    // Measure temperature
    tempsensor.wake(); // Wake up the MCP9808 if it was sleeping
    float tempC = tempsensor.readTempC();
    float tempF = tempC * 9.0 / 5.0 + 32;
    Serial.print("Temp: "); Serial.print(tempC); Serial.print("*C\t"); 
    Serial.print(tempF); Serial.println("*F");
    
    Serial.println("Shutting down the MCP9808...");
    tempsensor.shutdown(); // In this mode the MCP9808 draws only about 0.1uA
  
    float temperature = tempC; // Select what unit you want to use for this example
  
    // Turn on GPS if it wasn't on already (e.g., if the module wasn't turned off)
  #ifdef turnOffShield
    while (!fona.enableGPS(true)) {
      Serial.println(F("Failed to turn on GPS, retrying..."));
      delay(2000); // Retry every 2s
    }
    Serial.println(F("Turned on GPS!"));
  #endif
  
    // Get a fix on location, try every 2s
    // Use the top line if you want to parse UTC time data as well, the line below it if you don't care
  //  while (!fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude, &year, &month, &day, &hour, &minute, &second)) {
    while (!fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) {
      Serial.println(F("Failed to get GPS location, retrying..."));
      delay(2000); // Retry every 2s
    }
    Serial.println(F("Found 'eeeeem!"));
    Serial.println(F("---------------------"));
    Serial.print(F("Latitude: ")); Serial.println(latitude, 6);
    Serial.print(F("Longitude: ")); Serial.println(longitude, 6);
    Serial.print(F("Speed: ")); Serial.println(speed_kph);
    Serial.print(F("Heading: ")); Serial.println(heading);
    Serial.print(F("Altitude: ")); Serial.println(altitude);
    /*
    // Uncomment this if you care about parsing UTC time
    Serial.print(F("Year: ")); Serial.println(year);
    Serial.print(F("Month: ")); Serial.println(month);
    Serial.print(F("Day: ")); Serial.println(day);
    Serial.print(F("Hour: ")); Serial.println(hour);
    Serial.print(F("Minute: ")); Serial.println(minute);
    Serial.print(F("Second: ")); Serial.println(second);
    */
    Serial.println(F("---------------------"));
  
    // Format the floating point numbers
    dtostrf(latitude, 1, 6, latBuff); // float_val, min_width, digits_after_decimal, char_buffer
    dtostrf(longitude, 1, 6, longBuff);
    dtostrf(speed_kph, 1, 0, speedBuff);
    dtostrf(heading, 1, 0, headBuff);
    dtostrf(altitude, 1, 1, altBuff);
    dtostrf(temperature, 1, 2, tempBuff);
    dtostrf(battLevel, 1, 0, battBuff);
  
    // Construct a combined, comma-separated location array
    sprintf(locBuff, "%s,%s,%s,%s", speedBuff, latBuff, longBuff, altBuff); // This could look like "10,33.123456,-85.123456,120.5"
    
    // If not already connected, connect to MQTT
    if (! fona.MQTT_connectionStatus()) {
      // Set up MQTT parameters (see MQTT app note for explanation of parameter values)
      fona.MQTT_setParameter("URL", MQTT_SERVER, MQTT_PORT);
      // Set up MQTT username and password if necessary
      fona.MQTT_setParameter("USERNAME", MQTT_USERNAME);
      fona.MQTT_setParameter("PASSWORD", MQTT_PASSWORD);
  //    fona.MQTTsetParameter("KEEPTIME", 30); // Time to connect to server, 60s by default
      
      Serial.println(F("Connecting to MQTT broker..."));
      if (! fona.MQTT_connect(true)) {
        Serial.println(F("Failed to connect to broker!"));
      }
    }
    else {
      Serial.println(F("Already connected to MQTT server!"));
    }
  
    // Now publish all the GPS and temperature data to their respective topics!
    // Parameters for MQTT_publish: Topic, message (0-512 bytes), message length, QoS (0-2), retain (0-1)
    if (!fona.MQTT_publish(GPS_TOPIC, locBuff, strlen(locBuff), 1, 0)) Serial.println(F("Failed to publish!")); // Send GPS location
    if (!fona.MQTT_publish(TEMP_TOPIC, tempBuff, strlen(tempBuff), 1, 0)) Serial.println(F("Failed to publish!")); // Send temperature
    if (!fona.MQTT_publish(BATT_TOPIC, battBuff, strlen(battBuff), 1, 0)) Serial.println(F("Failed to publish!")); // Send battery level
  
    // Note the command below may error out if you're already subscribed to the topic!
    fona.MQTT_subscribe(SUB_TOPIC, 1); // Topic name, QoS
    
    // Unsubscribe from topics if wanted:
  //  fona.MQTT_unsubscribe(SUB_TOPIC);
  
    // Enable MQTT data format to hex
  //  fona.MQTT_dataFormatHex(true); // Input "false" to reverse
  
    // Disconnect from MQTT
  //  fona.MQTT_connect(false);
  
    // Delay until next post but read incoming subscribed topic messages (if any)
    Serial.print(F("Waiting for ")); Serial.print(samplingRate); Serial.println(F(" seconds\r\n"));

    firstTime = false;
    timer = millis(); // Reset timer at the end
  }
  else {
    // The rest of the time, read anything coming over via UART from the SIM7000
    // If it's from an MQTT subscribed topic message, parse it
    uint8_t i = 0;
    if (fona.available()) {
      while (fona.available()) {
        replybuffer[i] = fona.read();
        i++;
      }

      Serial.print(replybuffer); // DEBUG
      delay(100); // Make sure it prints and also allow other stuff to run properly

      // We got an MQTT message! Parse the topic and message
      // Format: +SMSUB: "topic_name","message"
      if (strstr(replybuffer, "+SMSUB:") != NULL) {
        Serial.println(F("*** Received MQTT message! ***"));
        
        char *p = strtok(replybuffer, ",\"");
        char *topic_p = strtok(NULL, ",\"");
        char *message_p = strtok(NULL, ",\"");
        
        Serial.print(F("Topic: ")); Serial.println(topic_p);
        Serial.print(F("Message: ")); Serial.println(message_p);
  
        // Do something with the message
        // For example, if the topic was "command" and we received a "yes", turn on an LED!
        if (strcmp(topic_p, "command") == 0) {
          if (strcmp(message_p, "yes") == 0) {
            Serial.println(F("Turning on LED!"));
            digitalWrite(LED, HIGH);
          }
          else if (strcmp(message_p, "no") == 0) {
            Serial.println(F("Turning off LED!"));
            digitalWrite(LED, HIGH);
          }
        }
      }

      /*
      // Alternatively, could convert to String class and parse that way
      // Format: +SMSUB: "topic_name","message"
      String reply = String(replybuffer);
      Serial.println(reply);
      
      if (reply.indexOf("+SMSUB: ") != -1) {
        Serial.println(F("*** Received MQTT message! ***"));

        // Chop off the "SMSUB: " part plus the beginning quote
        // After this, reply should be: "topic_name","message"
        reply = reply.substring(9);

        uint8_t idx = reply.indexOf("\",\""); // Search for second quote
        String topic = reply.substring(1, idx); // Grab only the text (without quotes)
        String message = reply.substring(idx+3, reply.length()-3);
        
        Serial.print(F("Topic: ")); Serial.println(topic);
        Serial.print(F("Message: ")); Serial.println(message);

        // Do something with the message
        // For example, if the topic was "command" and we received a "yes", turn on an LED!
        if (topic == "command") {
          if (message == "yes") {
            Serial.println(F("Turning on LED!"));
            digitalWrite(LED, HIGH);
          }
          else if (message == "no") {
            Serial.println(F("Turning off LED!"));
            digitalWrite(LED, HIGH);
          }
        }
      }
      */
    }
  }
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
  delay(100); // For SIM7000
  digitalWrite(FONA_PWRKEY, HIGH);
}

void moduleSetup() {
  // Note: The SIM7000A baud rate seems to reset after being power cycled (SIMCom firmware thing)
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
    while(1); // Don't proceed if it couldn't find the device
  }

  // The commented block of code below is an alternative that will find the module at 115200
  // Then switch it to 9600 without having to wait for the module to turn on and manually
  // press the reset button in order to establish communication. However, once the baud is set
  // this method will be much slower.
  /*
  fonaSerial->begin(115200); // Default LTE shield baud rate
  fona.begin(*fonaSerial); // Don't use if statement because an OK reply could be sent incorrectly at 115200 baud

  Serial.println(F("Configuring to 9600 baud"));
  fona.setBaudrate(9600); // Set to 9600 baud
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
}

// Read the module's power supply voltage
float readVcc() {
  // Read battery voltage
  if (!fona.getBattVoltage(&battLevel)) Serial.println(F("Failed to read batt"));
  else Serial.print(F("battery = ")); Serial.print(battLevel); Serial.println(F(" mV"));

  // Read LiPo battery percentage
  // Note: This will NOT work properly on the LTE shield because the voltage
  // is regulated to 3.6V so you will always read about the same value!
//  if (!fona.getBattPercent(&battLevel)) Serial.println(F("Failed to read batt"));
//  else Serial.print(F("BAT % = ")); Serial.print(battLevel); Serial.println(F("%"));

  return battLevel;
}

bool netStatus() {
  int n = fona.getNetworkStatus();
  
  Serial.print(F("Network status ")); Serial.print(n); Serial.print(F(": "));
  if (n == 0) Serial.println(F("Not registered"));
  if (n == 1) Serial.println(F("Registered (home)"));
  if (n == 2) Serial.println(F("Not registered (searching)"));
  if (n == 3) Serial.println(F("Denied"));
  if (n == 4) Serial.println(F("Unknown"));
  if (n == 5) Serial.println(F("Registered roaming"));

  if (!(n == 1 || n == 5)) return false;
  else return true;
}
