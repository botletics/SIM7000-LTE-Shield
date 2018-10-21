#include <Adafruit_SleepyDog.h>
#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"

#define halt(s) { Serial.println(F( s )); while(1);  }

extern Adafruit_FONA fona;
extern SoftwareSerial fonaSS;

boolean FONAconnect(const __FlashStringHelper *apn, const __FlashStringHelper *username, const __FlashStringHelper *password) {
  Watchdog.reset();

  Serial.println(F("Initializing FONA....(May take 3 seconds)"));

  // The baud rate seems to reset back to default (115200) after
  // being powered down so let's try 115200 first. Hats off to
  // anyone who can figure out how to make it remember the new
  // baud rate even after being power cycled! If you are using
  // hardware serial then this shouldn't be an issue because
  // you can just use the default 115200 baud.
  fonaSS.begin(115200); // Default SIM7000 shield baud rate

  Serial.println(F("Configuring to 9600 baud"));
  fonaSS.println("AT+IPR=9600");
  delay(100); // Short pause to let the command run
  fonaSS.begin(9600);
  if (! fona.begin(fonaSS)) {
    Serial.println(F("Couldn't find FONA"));
    return false; // Don't proceed if it couldn't find the device
  }

  fonaSS.println("AT+CMEE=2"); // Verbose mode
  Serial.println(F("FONA is OK"));
  Watchdog.reset();
  Serial.println(F("Checking for network connection..."));
  while (fona.getNetworkStatus() != 1 && fona.getNetworkStatus() != 5) {
    delay(500);
  }

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();
  
  fona.setNetworkSettings(apn, username, password);

  Serial.println(F("Disabling GPRS"));
  fona.enableGPRS(false);

  Watchdog.reset();
  delay(5000);  // wait a few seconds to stabilize connection
  Watchdog.reset();

  Serial.println(F("Enabling GPRS"));
  if (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to turn GPRS on"));  
    return false;
  }
  Watchdog.reset();

  return true;
}
