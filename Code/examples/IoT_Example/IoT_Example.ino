/* This is an example sketch to send battery, temperature, and GPS location data to
 *  dweet.io, a free cloud API. You can choose to post only once or to post periodically
 *  by commenting/uncommenting line 57 ("#define samplingRate 30"). When this line is 
 *  commented out the AVR microcontroller and MCP9808 temperature sensor are put to 
 *  sleep to conserve power, but when the line is being used data will be sent to the
 *  cloud periodically. This makes it operate like a GPS tracker!
 *  
 *  To check if the data was successfully sent, go to http://dweet.io/get/latest/dweet/for/{IMEI}
 *  and the IMEI number is printed at the beginning of the code but can also be found printed
 *  on the SIMCOM module itself.
 *  
 *  Author: Timothy Woo (www.botletics.com)
 *  Github: https://github.com/botletics/NB-IoT-Shield
 *  Last Updated: 12/5/2017
 *  License: GNU GPL v3.0
  */

#include "Adafruit_FONA.h"
#include <avr/sleep.h>
#include <avr/power.h>

// Libraries for temperature sensor
#include <Wire.h>
#include "Adafruit_MCP9808.h"

// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

// Default
//#define FONA_RX 2
//#define FONA_TX 3
//#define FONA_RST 4
//#define PWRKEY 5

// For Feather FONA (SIM800) specifically
//#define FONA_RX  9
//#define FONA_TX  8
//#define FONA_RST 4
//#define FONA_RI  7
// For the PWRKEY pin, use any available digital pin, but cut the
// PWRKEY trace on the Feather FONA for this to work. YOu can't sleep
// the SIM800 if the trace isn't cut.
//#define PWRKEY 0

// For LTE shield
#define FONA_PWRKEY 6
#define FONA_RST 7
//#define FONA_DTR 8 // Connect with solder jumper
//#define FONA_RI 9 // Need to enable via AT commands
#define FONA_TX 10 // Microcontroller RX
#define FONA_RX 11 // Microcontroller TX
//#define T_ALERT 12 // Connect with solder jumper

// Using SoftwareSerial:
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

// Use this for FONA 800 and 808s
//Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
// Use this one for FONA 3G. Run the "FONA3G_setBaud" example before running code for 3G or it may not work.
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);
// Use this one for FONA LTE
// Notice how we don't include the reset pin because it's reserved for emergencies on the LTE module!
Adafruit_FONA_LTE fona = Adafruit_FONA_LTE();

// The following line is used for applications that require repeated data posting, like GPS trackers
// Comment it out if you only want it to post once, not repeatedly every so often
#define samplingRate 30 // The time in between posts, in seconds

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
char imei[16] = {0}; // Use this for device ID
char replybuffer[255]; // Large buffer for replies
uint8_t type;
uint16_t battLevel = 0; // Battery level (percentage)
float latitude, longitude, speed_kph, heading, altitude;

void setup() {
  Serial.begin(115200);

  pinMode(FONA_RST, OUTPUT);
  digitalWrite(FONA_RST, HIGH); // Default state

  tempsensor.wake(); // Wake up the MCP9808 if it was sleeping
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find the MCP9808!");
    while (1);
  }

  pinMode(FONA_PWRKEY, OUTPUT);
  powerOn(); // Power on the module

  // Configure a GPRS APN, username, and password.
  // You might need to do this to access your network's GPRS/data
  // network.  Contact your provider for the exact APN, username,
  // and password values.  Username and password are optional and
  // can be removed, but APN is required.
  //fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));
  //fona.setGPRSNetworkSettings(F("phone"); // This worked fine for a standard AT&T 3G SIM card (US)
  fona.setGPRSNetworkSettings(F("m2m.com.attz")); // For AT&T IoT SIM card

  // Optionally configure HTTP gets to follow redirects over SSL.
  // Default is not to follow SSL redirects, however if you uncomment
  // the following line then redirects over SSL will be followed.
  //fona.setHTTPSRedirect(true);
}

void loop() {
  powerOn(); // Powers on the module if it was off previously

  fonaSerial->begin(115200); // Default LTE shield baud rate
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
  }

  // Baud rate setup
  fona.setBaudrate(4800); // Set to 4800 baud
  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1); // This makes the code freeze when it can't find the device
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
  
  // Connect to cell network and verify connection
  // If unsuccessful, keep retrying every 2s until a connection is made
  while (!netStatus()) {
    Serial.println(F("Failed to connect to cell network, retrying..."));
    delay(2000); // Retry every 2s
  }
  Serial.println(F("Connected to cell network!"));
  delay(1000); // Short delay to help GPRS enable successfully

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

  // Turn on GPS
  while (!fona.enableGPS(true)) {
    Serial.println(F("Failed to turn on GPS, retrying..."));
    delay(2000); // Retry every 2s
  }
  Serial.println(F("Turned on GPS!"));

  // Get a fix on location, try every 2s
  while (!fona.getGPS(&latitude, &longitude, &speed_kph, &heading, &altitude)) {
    Serial.println(F("Failed to get GPS location, retrying..."));
    delay(2000); // Retry every 2s
  }
  Serial.println(F("Found 'eeeeem!"));
  Serial.println("---------------------");
  Serial.print("Latitude: "); Serial.println(latitude);
  Serial.print("Longitude: "); Serial.println(longitude);
  Serial.print("Speed: "); Serial.println(speed_kph);
  Serial.print("Heading: "); Serial.println(heading);
  Serial.print("Altitude: "); Serial.println(altitude);
  Serial.println("---------------------");

  // Disable GPRS just to make sure it was actually off so that we can turn it on
  if (!fona.enableGPRS(false)) Serial.println(F("Failed to disable GPRS!"));
  
  // Turn on GPRS
  while (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to enable GPRS, retrying..."));
    delay(2000); // Retry every 2s
  }
  Serial.println(F("Enabled GPRS!"));
  delay(1000); // A short delay is needed so the next part runs properly!

  // Post something like temperature and battery level to the web API
  // Construct URL and post the data to the web API
  char URL[200]; // Make sure this is long enough for your request URL
  char latBuff[16], longBuff[16], speedBuff[16], headBuff[16], altBuff[16],
       tempBuff[16], battBuff[16];

  // Format the floating point numbers
  dtostrf(latitude, 1, 6, latBuff);
  dtostrf(longitude, 1, 6, longBuff);
  dtostrf(speed_kph, 1, 0, speedBuff);
  dtostrf(heading, 1, 0, headBuff);
  dtostrf(altitude, 1, 1, altBuff);
  dtostrf(temperature, 1, 2, tempBuff); // float_val, min_width, digits_after_decimal, char_buffer
  dtostrf(battLevel, 1, 0, battBuff);

  // Construct the appropriate URL's and body, depending on request type
  // In this example we use the IMEI as device ID

  // GET request
  // You can adjust the contents of the request if you don't need certain things like speed, altitude, etc.
  sprintf(URL, "http://dweet.io/dweet/for/%s?lat=%s&long=%s&speed=%s&head=%s&alt=%s&temp=%s&batt=%s", imei, latBuff, longBuff,
          speedBuff, headBuff, altBuff, tempBuff, battBuff);

  int counter = 0; // This counts the number of failed attempts tries
  // Try a total of three times if the post was unsuccessful (try additional 2 times)
  while (counter < 3 && !fona.postData("GET", URL, "")) { // Add the quotes "" as third input because for GET request there's no "body"
    Serial.println(F("Failed to post data, retrying..."));
    counter++; // Increment counter
    delay(1000);
  }
  
  // You can also do a POST request instead
  /*
  sprintf(URL, "http://dweet.io/dweet/for/%s", imei);
  sprintf(body, "{\"temp\":%s,\"batt\":%s}", tempBuff, battLevelBuff);

  int counter = 0;
  while (!fona.postData("POST", URL, body)) {
    Serial.println(F("Failed to complete HTTP POST..."));
    counter++
    delay(1000);
  }
  */

  // Disable GPRS
  // Note that you might not want to check if this was successful, but just run it
  // since the next command is to turn off the module anyway
  if (!fona.enableGPRS(false)) Serial.println(F("Failed to disable GPRS!"));

  // Turn off GPS
  if (!fona.enableGPS(false)) Serial.println(F("Failed to turn off GPS!"));

  // Power off the module. Note that you could instead put it in minimum functionality mode
  // instead of completely turning it off. Experiment different ways depending on your application!
  // You should see the "PWR" LED turn off after this command
  if (!fona.powerDown()) Serial.println(F("Failed to power down FONA!"));
  
  // Alternative to the AT command method above:
  // If your FONA has a PWRKEY pin connected to your MCU, you can pulse PWRKEY
  // LOW for a little bit, then pull it back HIGH, like this:
//  digitalWrite(PWRKEY, LOW);
//  delay(600); // Minimum of 64ms to turn on and 500ms to turn off for FONA3G. Check spec sheet for other types
//  delay(1300); // Minimum of 1.2s for LTE shield
//  digitalWrite(PWRKEY, HIGH);
  
  // Shut down the MCU to save power
  // Comment out this line if you want it to keep posting periodically
  #ifndef samplingRate
    delay(5); // This is just to read the response of the last AT command before shutting down
    MCU_powerDown(); // You could also write your own function to make it sleep for a certain duration instead
  #else
    // The following lines are for if you want to periodically post data (like GPS tracker)
    Serial.print("Waiting for "); Serial.print(samplingRate); Serial.println(" seconds");
    delay(samplingRate*1000); // Delay
  #endif
}

// Power on the module
void powerOn() {
  digitalWrite(FONA_PWRKEY, LOW);
//  delay(1050); // Pull low at least 1s to turn on SIM800
  delay(100); // At least 72ms to turn on the SIM7000 (LTE)
  digitalWrite(FONA_PWRKEY, HIGH);
}

// Read the battery level percentage
float readVcc() {
  // Read battery voltage
  if (!fona.getBattVoltage(&battLevel)) Serial.println(F("Failed to read batt"));
  else Serial.print(F("battery = ")); Serial.print(battLevel); Serial.println(F(" mV"));

  // Read battery percentage
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

// Turn off the MCU completely. Can only wake up from RESET button
// However, this can be altered to wake up via a pin change interrupt
void MCU_powerDown() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ADCSRA = 0; // Turn off ADC
  power_all_disable ();  // Power off ADC, Timer 0 and 1, serial interface
  sleep_enable();
  sleep_cpu();
}
