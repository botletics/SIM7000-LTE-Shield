// Modified version of Adafruit FONA library for Botletics hardware
// Original text below:

/***************************************************
  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_FONA.h"

#ifdef SSL_FONA
  char *server_CA_FONA;
  uint16_t port_CA_FONA = 0;
  int CID_CA_FONA = 0;
  char *rootCA_FONA;
#endif



Adafruit_FONA::Adafruit_FONA(int8_t rst)
{
  _rstpin = rst;

  // apn = F("FONAnet");
  apn = F("");
  apnusername = 0;
  apnpassword = 0;
  mySerial = 0;
  httpsredirect = false;
  useragent = F("FONA");
  ok_reply = F("OK");
}

uint8_t Adafruit_FONA::type(void) {
  return _type;
}

boolean Adafruit_FONA::begin(Stream &port) {
  mySerial = &port;

  if (_rstpin != 99) { // Pulse the reset pin only if it's not an LTE module
    DEBUG_PRINTLN(F("Resetting the module..."));
    pinMode(_rstpin, OUTPUT);
    digitalWrite(_rstpin, HIGH);
    delay(10);
    digitalWrite(_rstpin, LOW);
    delay(100);
    digitalWrite(_rstpin, HIGH);
  }

  DEBUG_PRINTLN(F("Attempting to open comm with ATs"));
  // give 7 seconds to reboot
  int16_t timeout = 7000;

  while (timeout > 0) {
    while (mySerial->available()) mySerial->read();
    if (sendCheckReply(F("AT"), ok_reply))
      break;
    while (mySerial->available()) mySerial->read();
    if (sendCheckReply(F("AT"), F("AT"))) 
      break;
    delay(500);
    timeout-=500;
  }

  if (timeout <= 0) {
#ifdef ADAFRUIT_FONA_DEBUG
    DEBUG_PRINTLN(F("Timeout: No response to AT... last ditch attempt."));
#endif
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
  }

  // turn off Echo!
  sendCheckReply(F("ATE0"), ok_reply);
  delay(100);

  if (! sendCheckReply(F("ATE0"), ok_reply)) {
    return false;
  }

  // turn on hangupitude
  if (_rstpin != 99) sendCheckReply(F("AT+CVHU=0"), ok_reply);

  delay(100);
  flushInput();


  // DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN("ATI");
  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN("AT+GMR"); // This definitely should have the module name, but ATI may not

  // mySerial->println("ATI");
  mySerial->println("AT+GMR");
  readline(500, true);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);



  if (prog_char_strstr(replybuffer, (prog_char *)F("SIM808 R14")) != 0) {
    _type = SIM808_V2;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("1418B03SIM808M32_BT_EAT")) != 0) {
    _type = SIM808_V2; //For Boards with Bluetooth and Extended AT commands for it
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM808 R13")) != 0) {
    _type = SIM808_V1;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM800 R13")) != 0) {
    _type = SIM800L;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIMCOM_SIM5320A")) != 0) {
    _type = SIM5320A;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIMCOM_SIM5320E")) != 0) {
    _type = SIM5320E;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000A")) != 0) {
  //   _type = SIM7000A;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000C")) != 0) {
  //   _type = SIM7000C;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000E")) != 0) {
  //   _type = SIM7000E;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000G")) != 0) {
  //   _type = SIM7000G;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7000")) != 0) {
    _type = SIM7000;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7070")) != 0) {
    _type = SIM7070;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7500A")) != 0) {
    // _type = SIM7500A;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7500E")) != 0) {
  //   _type = SIM7500E;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7500")) != 0) {
    _type = SIM7500;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7600A")) != 0) {
  //   _type = SIM7600A;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7600C")) != 0) {
  //   _type = SIM7600C;
  // } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7600E")) != 0) {
  //   _type = SIM7600E;
  } else if (prog_char_strstr(replybuffer, (prog_char *)F("SIM7600")) != 0) {
    _type = SIM7600;
  }


  if (_type == SIM800L) {
    // determine if L or H

  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN("AT+GMM");

    mySerial->println("AT+GMM");
    readline(500, true);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);


    if (prog_char_strstr(replybuffer, (prog_char *)F("SIM800H")) != 0) {
      _type = SIM800H;
    }
  }

#if defined(FONA_PREF_SMS_STORAGE)
    sendCheckReply(F("AT+CPMS=" FONA_PREF_SMS_STORAGE "," FONA_PREF_SMS_STORAGE "," FONA_PREF_SMS_STORAGE), ok_reply);
#endif

  return true;
}


/********* Serial port ********************************************/
boolean Adafruit_FONA::setBaudrate(uint32_t baud) {
  return sendCheckReply(F("AT+IPREX="), baud, ok_reply);
}

boolean Adafruit_FONA_LTE::setBaudrate(uint32_t baud) {
  return sendCheckReply(F("AT+IPR="), baud, ok_reply);
}


/********* POWER, BATTERY & ADC ********************************************/

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA::getBattVoltage(uint16_t *v) {
  if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
    float f;
    boolean b = sendParseReplyFloat(F("AT+CBC"), F("+CBC: "), &f, ',', 0);
    *v = f*1000;
    return b;
  } else
    return sendParseReply(F("AT+CBC"), F("+CBC: "), v, ',', 2);
}

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA_3G::getBattVoltage(uint16_t *v) {
  float f;
  boolean b = sendParseReply(F("AT+CBC"), F("+CBC: "), &f, ',', 2);
  *v = f*1000;
  return b;
}

/* powers on the module */
void Adafruit_FONA::powerOn(uint8_t FONA_PWRKEY) {
  pinMode(FONA_PWRKEY, OUTPUT);
  digitalWrite(FONA_PWRKEY, LOW);

  // See spec sheets for your particular module
  if (_type <= SIM808_V2)
    delay(1050);
  else if (_type == SIM5320A || _type == SIM5320E)
    delay(180); // For SIM5320
  else if (_type == SIM7000 || _type == SIM7070)
    delay(1100); // At least 1s
  else if (_type == SIM7500 || _type == SIM7600)
    delay(500);
  
  digitalWrite(FONA_PWRKEY, HIGH);
}

/* powers down the SIM module */
boolean Adafruit_FONA::powerDown(void) {
  if (_type == SIM7500 || _type == SIM7600) {
    if (! sendCheckReply(F("AT+CPOF"), ok_reply))
      return false;
  }
  else {
    if (! sendCheckReply(F("AT+CPOWD=1"), F("NORMAL POWER DOWN"))) // Normal power off
        return false;
  }
  

  return true;
}

/* powers down the SIM5320 */
boolean Adafruit_FONA_3G::powerDown(void) {
  if (! sendCheckReply(F("AT+CPOF"), ok_reply))
    return false;

  return true;
}


/* returns the percentage charge of battery as reported by sim800 */
boolean Adafruit_FONA::getBattPercent(uint16_t *p) {
  return sendParseReply(F("AT+CBC"), F("+CBC: "), p, ',', 1);
}

boolean Adafruit_FONA::getADCVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CADC?"), F("+CADC: 1,"), v);
}


/********* NETWORK AND WIRELESS CONNECTION SETTINGS ***********************/

// Uses the AT+CFUN command to set functionality (refer to AT+CFUN in manual)
// 0 --> Minimum functionality
// 1 --> Full functionality
// 4 --> Disable RF
// 5 --> Factory test mode
// 6 --> Restarts module
// 7 --> Offline mode
boolean Adafruit_FONA::setFunctionality(uint8_t option) {
  return sendCheckReply(F("AT+CFUN="), option, ok_reply);
}

// 2  - Automatic
// 13 - GSM only
// 38 - LTE only
// 51 - GSM and LTE only
boolean Adafruit_FONA_LTE::setPreferredMode(uint8_t mode) {
  return sendCheckReply(F("AT+CNMP="), mode, ok_reply);
}

// 1 - CAT-M
// 2 - NB-IoT
// 3 - CAT-M and NB-IoT
boolean Adafruit_FONA_LTE::setPreferredLTEMode(uint8_t mode) {
  return sendCheckReply(F("AT+CMNB="), mode, ok_reply);
}

// Useful for choosing a certain carrier only
// For example, AT&T uses band 12 in the US for LTE CAT-M
// whereas Verizon uses band 13
// Mode: "CAT-M" or "NB-IOT"
// Band: The cellular EUTRAN band number
boolean Adafruit_FONA_LTE::setOperatingBand(const char * mode, uint8_t band) {
  char cmdBuff[24];

  sprintf(cmdBuff, "AT+CBANDCFG=\"%s\",%i", mode, band);

  return sendCheckReply(cmdBuff, ok_reply);
}

// Sleep mode reduces power consumption significantly while remaining registered to the network
// NOTE: USB port must be disconnected before this will take effect
boolean Adafruit_FONA::enableSleepMode(bool onoff) {
  return sendCheckReply(F("AT+CSCLK="), onoff, ok_reply);
}

// Set e-RX parameters
// Mode options:
// 0  Disable the use of eDRX
// 1  Enable the use of eDRX
// 2  Enable the use of eDRX and auto report
// 3  Disable the use of eDRX(Reserved)

// Connection type options:
// 4 - CAT-M
// 5 - NB-IoT

// See AT command manual for eDRX values (options 0-15)

// NOTE: Network must support eDRX mode
boolean Adafruit_FONA::set_eDRX(uint8_t mode, uint8_t connType, char * eDRX_val) {
  if (strlen(eDRX_val) > 4) return false;

  char auxStr[21];

  sprintf(auxStr, "AT+CEDRXS=%i,%i,\"%s\"", mode, connType, eDRX_val);

  return sendCheckReply(auxStr, ok_reply);
}

// NOTE: Network must support PSM and modem needs to restart before it takes effect
boolean Adafruit_FONA::enablePSM(bool onoff) {
  return sendCheckReply(F("AT+CPSMS="), onoff, ok_reply);
}
// Set PSM with custom TAU and active time
// For both TAU and Active time, leftmost 3 bits represent the multiplier and rightmost 5 bits represent the value in bits.

// For TAU, left 3 bits:
// 000 10min
// 001 1hr
// 010 10hr
// 011 2s
// 100 30s
// 101 1min
// For Active time, left 3 bits:
// 000 2s
// 001 1min
// 010 6min
// 111 disabled

// Note: Network decides the final value of the TAU and active time. 
boolean Adafruit_FONA::enablePSM(bool onoff, char * TAU_val, char * activeTime_val) { // AT+CPSMS command
    if (strlen(activeTime_val) > 8) return false;
    if (strlen(TAU_val) > 8) return false;

    char auxStr[35];
    sprintf(auxStr, "AT+CPSMS=%i,,,\"%s\",\"%s\"", onoff, TAU_val, activeTime_val);

    return sendCheckReply(auxStr, ok_reply);
}

// Enable, disable, or set the blinking frequency of the network status LED
// Default settings are the following:
// Not connected to network --> 1,64,800
// Connected to network     --> 2,64,3000
// Data connection enabled  --> 3,64,300
boolean Adafruit_FONA::setNetLED(bool onoff, uint8_t mode, uint16_t timer_on, uint16_t timer_off) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CNETLIGHT=1"), ok_reply)) return false;

    if (mode > 0) {
      char auxStr[25];

      sprintf(auxStr, "AT+SLEDS=%i,%i,%i", mode, timer_on, timer_off);

      return sendCheckReply(auxStr, ok_reply);
    }
    else {
      return false;
    }
  }
  else {
    return sendCheckReply(F("AT+CNETLIGHT=0"), ok_reply);
  }
}

/********* SIM ***********************************************************/

// Return status of PIN requirement
// -2 - Command returned with an error
// -1 - Unknown status
// 0 - MT is not pending for any password
// 1 - MT is waiting SIM PIN to be given
// 2 - MT is waiting for SIM PUK to be given
// 3 - ME is waiting for phone to SIM card (antitheft)
// 4 - ME is waiting for SIM PUK (antitheft)
// 5 - PIN2, e.g. for editing the FDN book possible only if preceding Command was acknowledged with +CME ERROR:17
// 6 - PUK2. Possible only if preceding Command was acknowledged with error +CME ERROR: 18.
int8_t Adafruit_FONA::getPINStatus()
{
  getReply(F("AT+CPIN?"));

  if (strncmp(replybuffer, "+CPIN: ", 7) != 0)
    return FONA_SIM_ERROR;

  char *returnVal = replybuffer + 7;

  if (strcmp(returnVal, "READY") == 0)
    return FONA_SIM_READY;
  else if (strcmp(returnVal, "SIM PIN") == 0)
    return FONA_SIM_PIN;
  else if (strcmp(returnVal, "SIM PUK") == 0)
    return FONA_SIM_PUK;
  else if (strcmp(returnVal, "PH_SIM PIN") == 0)
    return FONA_SIM_PH_PIN;
  else if (strcmp(returnVal, "PH_SIM PUK") == 0)
    return FONA_SIM_PH_PUK;
  else if (strcmp(returnVal, "SIM PIN2") == 0)
    return FONA_SIM_PIN2;
  else if (strcmp(returnVal, "SIM PUK2") == 0)
    return FONA_SIM_PUK2;

  return FONA_SIM_UNKNOWN;
}

uint8_t Adafruit_FONA::unlockSIM(const char *pin)
{
  char sendbuff[14] = "AT+CPIN=";
  sendbuff[8] = pin[0];
  sendbuff[9] = pin[1];
  sendbuff[10] = pin[2];
  sendbuff[11] = pin[3];
  sendbuff[12] = '\0';

  return sendCheckReply(sendbuff, ok_reply);
}

uint8_t Adafruit_FONA::getSIMCCID(char *ccid) {
  getReply(F("AT+CCID"));
  // up to 28 chars for reply, 20 char total ccid
  if (replybuffer[0] == '+') {
    // fona 3g?
    strncpy(ccid, replybuffer+8, 20);
  } else {
    // fona 800 or 800
    strncpy(ccid, replybuffer, 20);
  }
  ccid[20] = 0;

  readline(); // eat 'OK'

  return strlen(ccid);
}

/********* IMEI **********************************************************/

uint8_t Adafruit_FONA::getIMEI(char *imei) {
  getReply(F("AT+GSN"));

  // up to 15 chars
  strncpy(imei, replybuffer, 15);
  imei[15] = 0;

  readline(); // eat 'OK'

  return strlen(imei);
}

/********* NETWORK *******************************************************/

uint8_t Adafruit_FONA::getNetworkStatus(void) {
  uint16_t status;

  if (_type >= SIM7000) {
    if (! sendParseReply(F("AT+CGREG?"), F("+CGREG: "), &status, ',', 1)) return 0;
  }
  else {
    if (! sendParseReply(F("AT+CREG?"), F("+CREG: "), &status, ',', 1)) return 0;
  }

  return status;
}


uint8_t Adafruit_FONA::getRSSI(void) {
  uint16_t reply;

  if (! sendParseReply(F("AT+CSQ"), F("+CSQ: "), &reply) ) return 0;

  return reply;
}

/********* AUDIO *******************************************************/

boolean Adafruit_FONA::setAudio(uint8_t a) {
  // For SIM5320, 1 is headset, 3 is speaker phone, 4 is PCM interface
  if ( (_type == SIM5320A || _type == SIM5320E) && (a != 1 && a != 3 && a != 4) ) return false;
  // For SIM7500, 1 is headset, 3 is speaker phone
  else if ( (_type == SIM7500 || _type == SIM7600) && (a != 1 && a != 3) ) return false;
  // For SIM800, 0 is main audio channel, 1 is aux, 2 is main audio channel (hands-free), 3 is aux channel (hands-free), 4 is PCM channel
  else if (a > 4) return false; // 0 is headset, 1 is external audio

  if (_type <= SIM808_V2) return sendCheckReply(F("AT+CHFA="), a, ok_reply);
  else return sendCheckReply(F("AT+CSDVC="), a, ok_reply);
}

uint8_t Adafruit_FONA::getVolume(void) {
  uint16_t reply;

  if (! sendParseReply(F("AT+CLVL?"), F("+CLVL: "), &reply) ) return 0;

  return reply;
}

boolean Adafruit_FONA::setVolume(uint8_t i) {
  return sendCheckReply(F("AT+CLVL="), i, ok_reply);
}


boolean Adafruit_FONA::playDTMF(char dtmf) {
  char str[4];
  str[0] = '\"';
  str[1] = dtmf;
  str[2] = '\"';
  str[3] = 0;
  return sendCheckReply(F("AT+CLDTMF=3,"), str, ok_reply);
}

boolean Adafruit_FONA::playToolkitTone(uint8_t t, uint16_t len) {
  return sendCheckReply(F("AT+STTONE=1,"), t, len, ok_reply);
}

boolean Adafruit_FONA_3G::playToolkitTone(uint8_t t, uint16_t len) {
  if (! sendCheckReply(F("AT+CPTONE="), t, ok_reply))
    return false;
  delay(len);
  return sendCheckReply(F("AT+CPTONE=0"), ok_reply);
}

boolean Adafruit_FONA::setMicVolume(uint8_t a, uint8_t level) {
  // For SIM800, 0 is main audio channel, 1 is aux, 2 is main audio channel (hands-free), 3 is aux channel (hands-free)
  if (a > 3) return false;

  return sendCheckReply(F("AT+CMIC="), a, level, ok_reply);
}

/********* FM RADIO *******************************************************/


boolean Adafruit_FONA::FMradio(boolean onoff, uint8_t a) {
  if (! onoff) {
    return sendCheckReply(F("AT+FMCLOSE"), ok_reply);
  }

  // 0 is headset, 1 is external audio
  if (a > 1) return false;

  return sendCheckReply(F("AT+FMOPEN="), a, ok_reply);
}

boolean Adafruit_FONA::tuneFMradio(uint16_t station) {
  // Fail if FM station is outside allowed range.
  if ((station < 870) || (station > 1090))
    return false;

  return sendCheckReply(F("AT+FMFREQ="), station, ok_reply);
}

boolean Adafruit_FONA::setFMVolume(uint8_t i) {
  // Fail if volume is outside allowed range (0-6).
  if (i > 6) {
    return false;
  }
  // Send FM volume command and verify response.
  return sendCheckReply(F("AT+FMVOLUME="), i, ok_reply);
}

int8_t Adafruit_FONA::getFMVolume() {
  uint16_t level;

  if (! sendParseReply(F("AT+FMVOLUME?"), F("+FMVOLUME: "), &level) ) return 0;

  return level;
}

int8_t Adafruit_FONA::getFMSignalLevel(uint16_t station) {
  // Fail if FM station is outside allowed range.
  if ((station < 875) || (station > 1080)) {
    return -1;
  }

  // Send FM signal level query command.
  // Note, need to explicitly send timeout so right overload is chosen.
  getReply(F("AT+FMSIGNAL="), station, FONA_DEFAULT_TIMEOUT_MS);
  // Check response starts with expected value.
  char *p = prog_char_strstr(replybuffer, PSTR("+FMSIGNAL: "));
  if (p == 0) return -1;
  p+=11;
  // Find second colon to get start of signal quality.
  p = strchr(p, ':');
  if (p == 0) return -1;
  p+=1;
  // Parse signal quality.
  int8_t level = atoi(p);
  readline();  // eat the "OK"
  return level;
}

/********* PWM/BUZZER **************************************************/

boolean Adafruit_FONA::setPWM(uint16_t period, uint8_t duty) {
  if (period > 2000) return false;
  if (duty > 100) return false;

  return sendCheckReply(F("AT+SPWM=0,"), period, duty, ok_reply);
}

/********* CALL PHONES **************************************************/
boolean Adafruit_FONA::callPhone(char *number) {
  char sendbuff[35] = "ATD";
  strncpy(sendbuff+3, number, min(30, (int)strlen(number)));

  uint8_t x = strlen(sendbuff);

  sendbuff[x] = ';';
  sendbuff[x+1] = 0;
  //DEBUG_PRINTLN(sendbuff);

  if (_type == SIM7500) sendCheckReply(F("AT+CSDVC=3"), ok_reply); // Enable speaker output

  return sendCheckReply(sendbuff, ok_reply);
}


uint8_t Adafruit_FONA::getCallStatus(void) {
  uint16_t phoneStatus;

  if (! sendParseReply(F("AT+CPAS"), F("+CPAS: "), &phoneStatus)) 
    return FONA_CALL_FAILED; // 1, since 0 is actually a known, good reply

  return phoneStatus;  // 0 ready, 2 unknown, 3 ringing, 4 call in progress
}

boolean Adafruit_FONA::hangUp(void) {
  return sendCheckReply(F("ATH0"), ok_reply);
}

boolean Adafruit_FONA_3G::hangUp(void) {
  getReply(F("ATH"));

  return (prog_char_strstr(replybuffer, (prog_char *)F("VOICE CALL: END")) != 0);
}

boolean Adafruit_FONA_LTE::hangUp(void) {
  // return sendCheckReply(F("ATH"), ok_reply); // For SIM7500 this only works when AT+CVHU=0
  return sendCheckReply(F("AT+CHUP"), ok_reply);
}

boolean Adafruit_FONA::pickUp(void) {
  return sendCheckReply(F("ATA"), ok_reply);
}

boolean Adafruit_FONA_3G::pickUp(void) {
  return sendCheckReply(F("ATA"), F("VOICE CALL: BEGIN"));
}


void Adafruit_FONA::onIncomingCall() {

  DEBUG_PRINT(F("> ")); DEBUG_PRINTLN(F("Incoming call..."));

  Adafruit_FONA::_incomingCall = true;
}

boolean Adafruit_FONA::_incomingCall = false;

boolean Adafruit_FONA::callerIdNotification(boolean enable, uint8_t interrupt) {
  if(enable){
    attachInterrupt(interrupt, onIncomingCall, FALLING);
    return sendCheckReply(F("AT+CLIP=1"), ok_reply);
  }

  detachInterrupt(interrupt);
  return sendCheckReply(F("AT+CLIP=0"), ok_reply);
}

boolean Adafruit_FONA::incomingCallNumber(char* phonenum) {
  //+CLIP: "<incoming phone number>",145,"",0,"",0
  if(!Adafruit_FONA::_incomingCall)
    return false;

  readline();
  while(!prog_char_strcmp(replybuffer, (prog_char*)F("RING")) == 0) {
    flushInput();
    readline();
  }

  readline(); //reads incoming phone number line

  parseReply(F("+CLIP: \""), phonenum, '"');


  DEBUG_PRINT(F("Phone Number: "));
  DEBUG_PRINTLN(replybuffer);


  Adafruit_FONA::_incomingCall = false;
  return true;
}

/********* SMS **********************************************************/

uint8_t Adafruit_FONA::getSMSInterrupt(void) {
  uint16_t reply;

  if (! sendParseReply(F("AT+CFGRI?"), F("+CFGRI: "), &reply) ) return 0;

  return reply;
}

boolean Adafruit_FONA::setSMSInterrupt(uint8_t i) {
  return sendCheckReply(F("AT+CFGRI="), i, ok_reply);
}

int8_t Adafruit_FONA::getNumSMS(void) {
  uint16_t numsms;

  // get into text mode
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return -1;

  // ask how many sms are stored
  if (sendParseReply(F("AT+CPMS?"), F(FONA_PREF_SMS_STORAGE ","), &numsms)) 
    return numsms;
  if (sendParseReply(F("AT+CPMS?"), F("\"SM\","), &numsms))
    return numsms;
  if (sendParseReply(F("AT+CPMS?"), F("\"SM_P\","), &numsms))
    return numsms;
  return -1;
}

// Reading SMS's is a bit involved so we don't use helpers that may cause delays or debug
// printouts!
boolean Adafruit_FONA::readSMS(uint8_t i, char *smsbuff,
             uint16_t maxlen, uint16_t *readlen) {
  // text mode
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return false;

  // show all text mode parameters
  if (! sendCheckReply(F("AT+CSDH=1"), ok_reply)) return false;

  // parse out the SMS len
  uint16_t thesmslen = 0;


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+CMGR="));
  DEBUG_PRINTLN(i);


  //getReply(F("AT+CMGR="), i, 1000);  //  do not print debug!
  mySerial->print(F("AT+CMGR="));
  mySerial->println(i);
  readline(1000); // timeout

  //DEBUG_PRINT(F("Reply: ")); DEBUG_PRINTLN(replybuffer);
  // parse it out...


  DEBUG_PRINTLN(replybuffer);

  
  if (! parseReply(F("+CMGR:"), &thesmslen, ',', 11)) {
    *readlen = 0;
    return false;
  }

  readRaw(thesmslen);

  flushInput();

  uint16_t thelen = min(maxlen, (uint16_t)strlen(replybuffer));
  strncpy(smsbuff, replybuffer, thelen);
  smsbuff[thelen] = 0; // end the string


  DEBUG_PRINTLN(replybuffer);

  *readlen = thelen;
  return true;
}

// Retrieve the sender of the specified SMS message and copy it as a string to
// the sender buffer.  Up to senderlen characters of the sender will be copied
// and a null terminator will be added if less than senderlen charactesr are
// copied to the result.  Returns true if a result was successfully retrieved,
// otherwise false.
boolean Adafruit_FONA::getSMSSender(uint8_t i, char *sender, int senderlen) {
  // Ensure text mode and all text mode parameters are sent.
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return false;
  if (! sendCheckReply(F("AT+CSDH=1"), ok_reply)) return false;


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+CMGR="));
  DEBUG_PRINTLN(i);


  // Send command to retrieve SMS message and parse a line of response.
  mySerial->print(F("AT+CMGR="));
  mySerial->println(i);
  readline(1000);


  DEBUG_PRINTLN(replybuffer);


  // Parse the second field in the response.
  boolean result = parseReplyQuoted(F("+CMGR:"), sender, senderlen, ',', 1);
  // Drop any remaining data from the response.
  flushInput();
  return result;
}

boolean Adafruit_FONA::sendSMS(const char *smsaddr, const char *smsmsg) {
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return false;

  char sendcmd[30] = "AT+CMGS=\"";
  strncpy(sendcmd+9, smsaddr, 30-9-2);  // 9 bytes beginning, 2 bytes for close quote + null
  sendcmd[strlen(sendcmd)] = '\"';

  if (! sendCheckReply(sendcmd, F("> "))) return false;

  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(smsmsg);

  // no need for extra NEWLINE characters mySerial->println(smsmsg);
  // no need for extra NEWLINE characters mySerial->println();
  mySerial->print(smsmsg);
  mySerial->write(0x1A);

  // DEBUG_PRINTLN("^Z");

  if ( (_type == SIM5320A) || (_type == SIM5320E) || (_type >= SIM7000) ) {
    // Eat two sets of CRLF
    readline(200);
    //DEBUG_PRINT("Line 1: "); DEBUG_PRINTLN(strlen(replybuffer));
    readline(200);
    //DEBUG_PRINT("Line 2: "); DEBUG_PRINTLN(strlen(replybuffer));
  }
  readline(30000); // read the +CMGS reply, wait up to 30s
  //DEBUG_PRINT("Line 3: "); DEBUG_PRINTLN(strlen(replybuffer));
  if (strstr(replybuffer, "+CMGS") == 0) {
    return false;
  }
  readline(1000); // read OK
  //DEBUG_PRINT("* "); DEBUG_PRINTLN(replybuffer);

  if (strcmp(replybuffer, "OK") != 0) {
    return false;
  }

  return true;
}

boolean Adafruit_FONA::deleteSMS(uint8_t i) {
    if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return false;
  // delete an sms
  char sendbuff[12] = "AT+CMGD=000";
  sendbuff[8] = (i / 100) + '0';
  i %= 100;
  sendbuff[9] = (i / 10) + '0';
  i %= 10;
  sendbuff[10] = i + '0';

  return sendCheckReply(sendbuff, ok_reply, 2000);
}


boolean Adafruit_FONA::deleteAllSMS() {
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return false;
  return sendCheckReply("AT+CMGD=1,4", ok_reply, 2000);
}

/********* USSD *********************************************************/

boolean Adafruit_FONA::sendUSSD(char *ussdmsg, char *ussdbuff, uint16_t maxlen, uint16_t *readlen) {
  if (! sendCheckReply(F("AT+CUSD=1"), ok_reply)) return false;

  char sendcmd[30] = "AT+CUSD=1,\"";
  strncpy(sendcmd+11, ussdmsg, 30-11-2);  // 11 bytes beginning, 2 bytes for close quote + null
  sendcmd[strlen(sendcmd)] = '\"';

  if (! sendCheckReply(sendcmd, ok_reply)) {
    *readlen = 0;
    return false;
  } else {
      readline(10000); // read the +CUSD reply, wait up to 10 seconds!!!
      //DEBUG_PRINT("* "); DEBUG_PRINTLN(replybuffer);
      char *p = prog_char_strstr(replybuffer, PSTR("+CUSD: "));
      if (p == 0) {
        *readlen = 0;
        return false;
      }
      p+=7; //+CUSD
      // Find " to get start of ussd message.
      p = strchr(p, '\"');
      if (p == 0) {
        *readlen = 0;
        return false;
      }
      p+=1; //"
      // Find " to get end of ussd message.
      char *strend = strchr(p, '\"');

      uint16_t lentocopy = min(maxlen-1, strend - p);
      strncpy(ussdbuff, p, lentocopy+1);
      ussdbuff[lentocopy] = 0;
      *readlen = lentocopy;
  }
  return true;
}


/********* TIME **********************************************************/

/*
boolean Adafruit_FONA::enableNetworkTimeSync(boolean onoff) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CLTS=1"), ok_reply))
      return false;
  } else {
    if (! sendCheckReply(F("AT+CLTS=0"), ok_reply))
      return false;
  }

  flushInput(); // eat any 'Unsolicted Result Code'

  return true;
}
*/

// Returns the status of the NTP module:
// 1 Network time synchronization is successful
// 61 Network Error
// 62 DNS resolution error
// 63 Connection Erro
// 64 Service response error
// 65 Service Response Timeout
// see AT Command manual 1.04 p.204
uint8_t Adafruit_FONA::getNTPstatus()
{
    if (! sendCheckReply(F("AT+CNTP"), ok_reply, 10000))
      return 0;

    uint16_t status;
    readline(10000);
    if (! parseReply(F("+CNTP: "), &status))
      return 0;

    return status;
}

boolean Adafruit_FONA::enableNTPTimeSync(boolean onoff, FONAFlashStringPtr ntpserver) {
  if (onoff) {
    if (! sendCheckReply(F("AT+CNTPCID=1"), ok_reply))
      return false;

    mySerial->print(F("AT+CNTP=\""));
    if (ntpserver != 0) {
      mySerial->print(ntpserver);
    } else {
      mySerial->print(F("pool.ntp.org"));
    }
    mySerial->println(F("\",0"));
    readline(FONA_DEFAULT_TIMEOUT_MS);
    if (strcmp(replybuffer, "OK") != 0)
      return false;

    if (! sendCheckReply(F("AT+CNTP"), ok_reply, 10000))
      return false;

    uint16_t status;
    readline(10000);
    if (! parseReply(F("+CNTP:"), &status))
      return false;
  } else {
    if (! sendCheckReply(F("AT+CNTPCID=0"), ok_reply))
      return false;
  }

  return true;
}

boolean Adafruit_FONA::getTime(char *buff, uint16_t maxlen) {
  getReply(F("AT+CCLK?"), (uint16_t) 10000);
  if (strncmp(replybuffer, "+CCLK: ", 7) != 0)
    return false;

  char *p = replybuffer+7;
  uint16_t lentocopy = min(maxlen-1, (int)strlen(p));
  strncpy(buff, p, lentocopy+1);
  buff[lentocopy] = 0;

  readline(); // eat OK

  return true;
}

/********* Real Time Clock ********************************************/

boolean Adafruit_FONA::readRTC(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hr, uint8_t *min, uint8_t *sec, int8_t *tz) {
  getReply(F("AT+CCLK?"), (uint16_t) 10000); //Get RTC timeout 10 sec
  if (strncmp(replybuffer, "+CCLK: ", 7) != 0)
    return false;

  char *p = replybuffer+8;   // skip +CCLK: "
  // Parse date
  int reply = atoi(p);     // get year
  *year = (uint8_t) reply;   // save as year
  p+=3;              // skip 3 char
  reply = atoi(p);
  *month = (uint8_t) reply;
  p+=3;
  reply = atoi(p);
  *date = (uint8_t) reply;
  p+=3;
  reply = atoi(p);
  *hr = (uint8_t) reply;
  p+=3;
  reply = atoi(p);
  *min = (uint8_t) reply;
  p+=3;
  reply = atoi(p);
  *sec = (uint8_t) reply;
  p+=3;
  reply = atoi(p);
  *tz = reply;

  readline(); // eat OK

  return true;
}

boolean Adafruit_FONA::enableRTC(uint8_t i) {
  if (! sendCheckReply(F("AT+CLTS="), i, ok_reply))
    return false;
  return sendCheckReply(F("AT&W"), ok_reply);
}

/********* GPS **********************************************************/


boolean Adafruit_FONA::enableGPS(boolean onoff) {
  uint16_t state;

  // First check if its already on or off

  if (_type == SIM808_V2 || _type == SIM7000 || _type == SIM7070) {
    if (! sendParseReply(F("AT+CGNSPWR?"), F("+CGNSPWR: "), &state) )
      return false;
  } else if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
    if (! Adafruit_FONA::sendParseReply(F("AT+CGPS?"), F("+CGPS: "), &state) )
      return false;
  } else {
    if (! sendParseReply(F("AT+CGPSPWR?"), F("+CGPSPWR: "), &state))
      return false;
  }

  if (onoff && !state) {
    if (_type == SIM808_V2 || _type == SIM7000 || _type == SIM7070) {
      if (! sendCheckReply(F("AT+CGNSPWR=1"), ok_reply))
        return false;
    } else if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
      if (! sendCheckReply(F("AT+CGPS=1"), ok_reply))
        return false;
    } else {
      if (! sendCheckReply(F("AT+CGPSPWR=1"), ok_reply))
        return false;
    }
  } else if (!onoff && state) {
    if (_type == SIM808_V2 || _type == SIM7000 || _type == SIM7070) {
      if (! sendCheckReply(F("AT+CGNSPWR=0"), ok_reply))
        return false;
    } else if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
      if (! sendCheckReply(F("AT+CGPS=0"), ok_reply))
        return false;
      // this takes a little time
      readline(2000); // eat '+CGPS: 0'
    } else {
      if (! sendCheckReply(F("AT+CGPSPWR=0"), ok_reply))
        return false;
    }
  }
  return true;
}

/*
boolean Adafruit_FONA_3G::enableGPS(boolean onoff) {
  uint16_t state;

  // first check if its already on or off
  if (! Adafruit_FONA::sendParseReply(F("AT+CGPS?"), F("+CGPS: "), &state) )
    return false;

  if (onoff && !state) {
    if (! sendCheckReply(F("AT+CGPS=1"), ok_reply))
      return false;
  } else if (!onoff && state) {
    if (! sendCheckReply(F("AT+CGPS=0"), ok_reply))
      return false;
    // this takes a little time
    readline(2000); // eat '+CGPS: 0'
  }
  return true;
}
*/

int8_t Adafruit_FONA::GPSstatus(void) {
  if (_type == SIM808_V2 || _type == SIM7000 || _type == SIM7070) {
    // 808 V2 uses GNS commands and doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGNSINF"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGNSINF: "));
    if (p == 0) return -1;
    p+=10;
    readline(); // eat 'OK'
    if (p[0] == '0') return 0; // GPS is not even on!

    p+=2; // Skip to second value, fix status.
    //DEBUG_PRINTLN(p);
    // Assume if the fix status is '1' then we have a 3D fix, otherwise no fix.
    if (p[0] == '1') return 3;
    else return 1;
  }
  if (_type == SIM5320A || _type == SIM5320E) {
    // FONA 3G doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGPSINFO"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGPSINFO:"));
    if (p == 0) return -1;
    if (p[10] != ',') return 3; // if you get anything, its 3D fix
    return 0;
  }
  else {
    // 808 V1 looks for specific 2D or 3D fix state.
    getReply(F("AT+CGPSSTATUS?"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("SSTATUS: Location "));
    if (p == 0) return -1;
    p+=18;
    readline(); // eat 'OK'
    //DEBUG_PRINTLN(p);
    if (p[0] == 'U') return 0;
    if (p[0] == 'N') return 1;
    if (p[0] == '2') return 2;
    if (p[0] == '3') return 3;
  }
  // else
  return 0;
}

uint8_t Adafruit_FONA::getGPS(uint8_t arg, char *buffer, uint8_t maxbuff) {
  int32_t x = arg;

  if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
    getReply(F("AT+CGPSINFO"));
  } else if (_type == SIM808_V1) {
    getReply(F("AT+CGPSINF="), x);
  } else {
    getReply(F("AT+CGNSINF"));
  }

  char *p = prog_char_strstr(replybuffer, (prog_char*)F("SINF"));
  if (p == 0) {
    buffer[0] = 0;
    return 0;
  }

  p+=6;

  uint8_t len = max(maxbuff-1, (int)strlen(p));
  strncpy(buffer, p, len);
  buffer[len] = 0;

  readline(); // eat 'OK'
  return len;
}

// boolean Adafruit_FONA::getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude) {
boolean Adafruit_FONA::getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude,
                              uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *min, float *sec) {

  char gpsbuffer[120];

  // we need at least a 2D fix
  if (_type < SIM7000) { // SIM7500 doesn't support AT+CGPSSTATUS? command
    if (GPSstatus() < 2)
      return false;
  }
  
  // grab the mode 2^5 gps csv from the sim808
  uint8_t res_len = getGPS(32, gpsbuffer, 120);

  // make sure we have a response
  if (res_len == 0)
    return false;

  if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
    // Parse 3G respose
    // +CGPSINFO:4043.000000,N,07400.000000,W,151015,203802.1,-12.0,0.0,0
    // skip beginning
    char *tok;

   // grab the latitude
    char *latp = strtok(gpsbuffer, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    // skip date & time
    tok = strtok(NULL, ",");
    tok = strtok(NULL, ",");

   // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;
      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

    (void) tok;

  } else if (_type == SIM808_V2 || _type >= SIM7000) {
    // Parse 808 V2 response.  See table 2-3 from here for format:
    // http://www.adafruit.com/datasheets/SIM800%20Series_GNSS_Application%20Note%20V1.00.pdf

    // skip GPS run status
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip fix status
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip date
    // tok = strtok(NULL, ",");
    // if (! tok) return false;

    // only grab date and time if needed
    if ((year != NULL) && (month != NULL) && (day != NULL) && (hour != NULL) && (min != NULL) && (sec != NULL)) {
      char *date = strtok(NULL, ",");
      if (! date) return false;
      
      // Seconds
      char *ptr = date + 12;
      *sec = atof(ptr);
      
      // Minutes
      ptr[0] = 0;
      ptr = date + 10;
      *min = atoi(ptr);

      // Hours
      ptr[0] = 0;
      ptr = date + 8;
      *hour = atoi(ptr);

      // Day
      ptr[0] = 0;
      ptr = date + 6;
      *day = atoi(ptr);

      // Month
      ptr[0] = 0;
      ptr = date + 4;
      *month = atoi(ptr);

      // Year
      ptr[0] = 0;
      ptr = date;
      *year = atoi(ptr);
    }
    else
    {
      // skip date
      tok = strtok(NULL, ",");
      if (! tok) return false;
    }

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    *lat = atof(latp);
    *lon = atof(longp);

    // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;

      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }

    (void) tok;
  }
  else {
    // Parse 808 V1 response.

    // skip mode
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip date
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip fix
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab latitude direction
    char *latdir = strtok(NULL, ",");
    if (! latdir) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

    // grab longitude direction
    char *longdir = strtok(NULL, ",");
    if (! longdir) return false;

    double latitude = atof(latp);
    double longitude = atof(longp);

    // convert latitude from minutes to decimal
    float degrees = floor(latitude / 100);
    double minutes = latitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (latdir[0] == 'S') degrees *= -1;

    *lat = degrees;

    // convert longitude from minutes to decimal
    degrees = floor(longitude / 100);
    minutes = longitude - (100 * degrees);
    minutes /= 60;
    degrees += minutes;

    // turn direction into + or -
    if (longdir[0] == 'W') degrees *= -1;

    *lon = degrees;

    // only grab speed if needed
    if (speed_kph != NULL) {

      // grab the speed in knots
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      // convert to kph
      *speed_kph = atof(speedp) * 1.852;

    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);

    }

    // no need to continue
    if (altitude == NULL)
      return true;

    // we need at least a 3D fix for altitude
    if (GPSstatus() < 3)
      return false;

    // grab the mode 0 gps csv from the sim808
    res_len = getGPS(0, gpsbuffer, 120);

    // make sure we have a response
    if (res_len == 0)
      return false;

    // skip mode
    tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip lat
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // skip long
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab altitude
    char *altp = strtok(NULL, ",");
    if (! altp) return false;

    *altitude = atof(altp);

    (void) tok;
  }

  return true;

}

boolean Adafruit_FONA::enableGPSNMEA(uint8_t i) {

  char sendbuff[15] = "AT+CGPSOUT=000";
  sendbuff[11] = (i / 100) + '0';
  i %= 100;
  sendbuff[12] = (i / 10) + '0';
  i %= 10;
  sendbuff[13] = i + '0';

  if (_type == SIM808_V2 || _type == SIM7000 || _type == SIM7070) {
    if (i) {
      sendCheckReply(F("AT+CGNSCFG=1"), ok_reply);
      sendCheckReply(F("AT+CGNSTST=1"), ok_reply);
      return true;
    }
    else
      return sendCheckReply(F("AT+CGNSTST=0"), ok_reply);
  } else {
    return sendCheckReply(sendbuff, ok_reply, 2000);
  }
}


/********* GPRS **********************************************************/


boolean Adafruit_FONA::enableGPRS(boolean onoff) {
  if (_type == SIM5320A || _type == SIM5320E || _type == SIM7500 || _type == SIM7600) {
    if (onoff) {
      // disconnect all sockets
      //sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000);

      if (! sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
        return false;


      // set bearer profile access point name
      if (apn) {
        // Send command AT+CGSOCKCONT=1,"IP","<apn value>" where <apn value> is the configured APN name.
        if (! sendCheckReplyQuoted(F("AT+CGSOCKCONT=1,\"IP\","), apn, ok_reply, 10000))
          return false;

        // set username/password
        if (apnusername) {
          char authstring[100] = "AT+CGAUTH=1,1,\"";
          // char authstring[100] = "AT+CSOCKAUTH=1,1,\""; // For 3G
          char *strp = authstring + strlen(authstring);
          prog_char_strcpy(strp, (prog_char *)apnusername);
          strp+=prog_char_strlen((prog_char *)apnusername);
          strp[0] = '\"';
          strp++;
          strp[0] = 0;

          if (apnpassword) {
            strp[0] = ','; strp++;
            strp[0] = '\"'; strp++;
            prog_char_strcpy(strp, (prog_char *)apnpassword);
            strp+=prog_char_strlen((prog_char *)apnpassword);
            strp[0] = '\"';
            strp++;
            strp[0] = 0;
          }

          if (! sendCheckReply(authstring, ok_reply, 10000))
            return false;
        }
      }

      // connect in transparent mode
      if (! sendCheckReply(F("AT+CIPMODE=1"), ok_reply, 10000))
        return false;
      // open network
      if (_type == SIM5320A || _type == SIM5320E) {
        if (! sendCheckReply(F("AT+NETOPEN=,,1"), F("Network opened"), 10000))
          return false;
      }
      else if (_type == SIM7500 || _type == SIM7600) {
        if (! sendCheckReply(F("AT+NETOPEN"), ok_reply, 10000))
          return false;
      }
      readline(); // eat 'OK'
    } else {
      // close GPRS context
      if (_type == SIM5320A || _type == SIM5320E) {
        if (! sendCheckReply(F("AT+NETCLOSE"), F("Network closed"), 10000))
          return false;
      }
      else if (_type == SIM7500 || _type == SIM7600) {
        getReply(F("AT+NETCLOSE"));
        getReply(F("AT+CHTTPSSTOP"));
        // getReply(F("AT+CHTTPSCLSE"));

        // if (! sendCheckReply(F("AT+NETCLOSE"), ok_reply, 10000))
       //   return false;
      //    if (! sendCheckReply(F("AT+CHTTPSSTOP"), F("+CHTTPSSTOP: 0"), 10000))
        //  return false;
        // if (! sendCheckReply(F("AT+CHTTPSCLSE"), ok_reply, 10000))
        //  return false;
      }

      readline(); // eat 'OK'
    }
  }
  else if (_type == SIM7070) {
    // getNetworkInfo();

    if (! openWirelessConnection(onoff)) return false;
    // if (! wirelessConnStatus()) return false;
  }
  else {
    if (onoff) {
      // if (_type < SIM7000) { // UNCOMMENT FOR LTE ONLY!
        // disconnect all sockets
        sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000);

        if (! sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
          return false;

      // set bearer profile! connection type GPRS
      if (! sendCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), ok_reply, 10000))
        return false;
      // } // UNCOMMENT FOR LTE ONLY!

      delay(200); // This seems to help the next line run the first time
      
      // set bearer profile access point name
      if (apn) {
        // Send command AT+SAPBR=3,1,"APN","<apn value>" where <apn value> is the configured APN value.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"APN\","), apn, ok_reply, 10000))
          return false;

        // if (_type < SIM7000) { // UNCOMMENT FOR LTE ONLY!
          // send AT+CSTT,"apn","user","pass"
          flushInput();

          mySerial->print(F("AT+CSTT=\""));
          mySerial->print(apn);
          if (apnusername) {
            mySerial->print("\",\"");
            mySerial->print(apnusername);
          }
          if (apnpassword) {
            mySerial->print("\",\"");
            mySerial->print(apnpassword);
          }
          mySerial->println("\"");

          DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(F("AT+CSTT=\""));
          DEBUG_PRINT(apn);
          
          if (apnusername) {
            DEBUG_PRINT("\",\"");
            DEBUG_PRINT(apnusername); 
          }
          if (apnpassword) {
            DEBUG_PRINT("\",\"");
            DEBUG_PRINT(apnpassword); 
          }
          DEBUG_PRINTLN("\"");
          
          if (! expectReply(ok_reply)) return false;
        // } // UNCOMMENT FOR LTE ONLY!
      
        // set username/password
        if (apnusername) {
          // Send command AT+SAPBR=3,1,"USER","<user>" where <user> is the configured APN username.
          if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"USER\","), apnusername, ok_reply, 10000))
            return false;
        }
        if (apnpassword) {
          // Send command AT+SAPBR=3,1,"PWD","<password>" where <password> is the configured APN password.
          if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"PWD\","), apnpassword, ok_reply, 10000))
            return false;
        }
      }

      // open bearer
      if (! sendCheckReply(F("AT+SAPBR=1,1"), ok_reply, 30000))
        return false;

      // if (_type < SIM7000) { // UNCOMMENT FOR LTE ONLY!
        // bring up wireless connection
        if (! sendCheckReply(F("AT+CIICR"), ok_reply, 10000))
          return false;
      // } // UNCOMMENT FOR LTE ONLY!

      // if (! openWirelessConnection(true)) return false;
      // if (! wirelessConnStatus()) return false;

    } else {
      // disconnect all sockets
      if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000))
        return false;

      // close bearer
      if (! sendCheckReply(F("AT+SAPBR=0,1"), ok_reply, 10000))
        return false;

      // if (_type < SIM7000) { // UNCOMMENT FOR LTE ONLY!
        if (! sendCheckReply(F("AT+CGATT=0"), ok_reply, 10000))
          return false;
    // } // UNCOMMENT FOR LTE ONLY!

    }
  }
  return true;
}

/*
boolean Adafruit_FONA_3G::enableGPRS(boolean onoff) {

  if (onoff) {
    // disconnect all sockets
    //sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 5000);

    if (! sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
      return false;


    // set bearer profile access point name
    if (apn) {
      // Send command AT+CGSOCKCONT=1,"IP","<apn value>" where <apn value> is the configured APN name.
      if (! sendCheckReplyQuoted(F("AT+CGSOCKCONT=1,\"IP\","), apn, ok_reply, 10000))
        return false;

      // set username/password
      if (apnusername) {
        char authstring[100] = "AT+CGAUTH=1,1,\"";
        char *strp = authstring + strlen(authstring);
        prog_char_strcpy(strp, (prog_char *)apnusername);
        strp+=prog_char_strlen((prog_char *)apnusername);
        strp[0] = '\"';
        strp++;
        strp[0] = 0;

        if (apnpassword) {
          strp[0] = ','; strp++;
          strp[0] = '\"'; strp++;
          prog_char_strcpy(strp, (prog_char *)apnpassword);
          strp+=prog_char_strlen((prog_char *)apnpassword);
          strp[0] = '\"';
          strp++;
          strp[0] = 0;
        }

        if (! sendCheckReply(authstring, ok_reply, 10000))
          return false;
      }
    }

    // connect in transparent
    if (! sendCheckReply(F("AT+CIPMODE=1"), ok_reply, 10000))
      return false;
    // open network (?)
    if (! sendCheckReply(F("AT+NETOPEN=,,1"), F("Network opened"), 10000))
      return false;

    readline(); // eat 'OK'
  } else {
    // close GPRS context
    if (! sendCheckReply(F("AT+NETCLOSE"), F("Network closed"), 10000))
      return false;

    readline(); // eat 'OK'
  }

  return true;
}
*/

// Returns type of the network the module is connected to
// 0 no service
// 1 GSM
// 3 EGPRS
// 7 LTE M1
// 9 LTE NB
// You can pass a string of sufficient length to receive a text copy as well
// NOTE: Only tested on SIM7000E
int8_t Adafruit_FONA::getNetworkType(char *typeStringBuffer, size_t bufferLength)
{
  uint16_t type;

  if (! sendParseReply(F("AT+CNSMOD?"), F("+CNSMOD:"), &type, ',', 1))
    return -1;
  
  if (typeStringBuffer != NULL)
  {
    switch (type)
    {
      case 0:
        strncpy(typeStringBuffer, "no service", bufferLength);
        break;
      case 1:
        strncpy(typeStringBuffer, "GSM", bufferLength);
        break;
      case 3:
        strncpy(typeStringBuffer, "EGPRS", bufferLength);
        break;
      case 7:
        strncpy(typeStringBuffer, "LTE M1", bufferLength);
        break;
      case 9:
        strncpy(typeStringBuffer, "LTE NB", bufferLength);
        break;
      default:
        strncpy(typeStringBuffer, "unknown", bufferLength);
        break;
    }
  }

  return (int8_t)type;
} 

// Returns bearer status
// -1 Command returned with an error
// 0 Bearer is connecting
// 1 Bearer is connected
// 2 Bearer is closing
// 3 Bearer is closed
int8_t Adafruit_FONA::getBearerStatus(void)
{
  uint16_t state;

  if (! sendParseReply(F("AT+SAPBR=2,1"), F("+SAPBR: "), &state, ',', 1))
    return -1;

  return (int8_t)state;
}

// Query IP address and copy it into the passed buffer.
// Buffer needs to be at least 16 chars long.
// Returns true on success.
boolean Adafruit_FONA::getIPv4(char *ipStringBuffer, size_t bufferLength)
{
  if (ipStringBuffer == NULL || bufferLength < 16)
    return false;

  getReply(F("AT+SAPBR=2,1"));

  strtok(replybuffer, "\"");
  char *temp = strtok(NULL, "\"");

  if (temp == NULL)
    return false;

  strncpy(ipStringBuffer, temp, bufferLength);

  return true;
}

void Adafruit_FONA::getNetworkInfo(void) {
  getReply(F("AT+CPSI?"));
  getReply(F("AT+COPS?"));
  readline(); // Eat 'OK'
}

bool Adafruit_FONA::getNetworkInfoLong(void) {
  if (!sendCheckReply(F("AT+COPS=?"), ok_reply, 2000))
     return false;

  return true;
}

int8_t Adafruit_FONA::GPRSstate(void) {
  uint16_t state;

  if (! sendParseReply(F("AT+CGATT?"), F("+CGATT: "), &state) )
    return -1;

  return state;
}

void Adafruit_FONA::setNetworkSettings(FONAFlashStringPtr apn,
              FONAFlashStringPtr username, FONAFlashStringPtr password) {
  this->apn = apn;
  this->apnusername = username;
  this->apnpassword = password;

  if (_type >= SIM7000) sendCheckReplyQuoted(F("AT+CGDCONT=1,\"IP\","), apn, ok_reply, 10000);
}

boolean Adafruit_FONA::getGSMLoc(uint16_t *errorcode, char *buff, uint16_t maxlen) {

  getReply(F("AT+CIPGSMLOC=1,1"), (uint16_t)10000);

  if (! parseReply(F("+CIPGSMLOC: "), errorcode))
    return false;

  char *p = replybuffer+14;
  uint16_t lentocopy = min(maxlen-1, (int)strlen(p));
  strncpy(buff, p, lentocopy+1);

  readline(); // eat OK

  return true;
}

boolean Adafruit_FONA::getGSMLoc(float *lat, float *lon) {

  uint16_t returncode;
  char gpsbuffer[120];

  // make sure we could get a response
  if (! getGSMLoc(&returncode, gpsbuffer, 120))
    return false;

  // make sure we have a valid return code
  if (returncode != 0)
    return false;

  // +CIPGSMLOC: 0,-74.007729,40.730160,2015/10/15,19:24:55
  // tokenize the gps buffer to locate the lat & long
  char *longp = strtok(gpsbuffer, ",");
  if (! longp) return false;

  char *latp = strtok(NULL, ",");
  if (! latp) return false;

  *lat = atof(latp);
  *lon = atof(longp);

  return true;

}

// Open or close wireless data connection
boolean Adafruit_FONA::openWirelessConnection(bool onoff) {
  if (!onoff) { // Disconnect wireless
    if (_type == SIM7070) {
      if (! sendCheckReply(F("AT+CNACT=0,0"), ok_reply)) return false;
    }
    else {
      if (! sendCheckReply(F("AT+CNACT=0"), ok_reply)) return false;
    }

    readline();
    DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

    if (_type == SIM7070 && strstr(replybuffer, ",DEACTIVE") == NULL) return false; // +APP PDP: <pdpidx>,DEACTIVE
    else if (_type == SIM7000 && strstr(replybuffer, "PDP: DEACTIVE") == NULL) return false; // +APP PDP: DEACTIVE
  }
  else {
    if (_type == SIM7070) {
      if (! sendCheckReply(F("AT+CNACT=0,1"), ok_reply)) return false;
    }
    else {
      if (! sendCheckReplyQuoted(F("AT+CNACT=1,"), apn, ok_reply)) return false;
    }
    
    readline();
    DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

    if (_type == SIM7070 && strstr(replybuffer, ",ACTIVE") == NULL) return false; // +APP PDP: <pdpidx>,ACTIVE
    else if (_type == SIM7000 && strstr(replybuffer, "PDP: ACTIVE") == NULL) return false; // +APP PDP: ACTIVE
  }

  return true;
}

// Query wireless connection status
boolean Adafruit_FONA::wirelessConnStatus(void) {
  getReply(F("AT+CNACT?"));
  // Format of response:
  // +CNACT: <status>,<ip_addr>  (ex.SIM7000)
  // +CNACT: <pdpidx>,<status>,<ip_addr>  (ex.SIM7070)  
  if(_type == SIM7070) {
    if (strstr(replybuffer, "+CNACT: 0,1") == NULL) return false;
  } else {
    if (strstr(replybuffer, "+CNACT: 1") == NULL) return false; 
  }
  return true;
}

boolean Adafruit_FONA::postData(const char *request_type, const char *URL, const char *body, const char *token, uint32_t bodylen) {
  // NOTE: Need to open socket/enable GPRS before using this function
  // char auxStr[64];

  // Make sure HTTP service is terminated so initialization will run
  sendCheckReply(F("AT+HTTPTERM"), ok_reply, 10000);

  // Initialize HTTP service
  if (! sendCheckReply(F("AT+HTTPINIT"), ok_reply, 10000))
    return false;

  // Set HTTP parameters
  if (! sendCheckReply(F("AT+HTTPPARA=\"CID\",1"), ok_reply, 10000))
    return false;

  // Specify URL
  char urlBuff[strlen(URL) + 22];

  sprintf(urlBuff, "AT+HTTPPARA=\"URL\",\"%s\"", URL);

  if (! sendCheckReply(urlBuff, ok_reply, 10000))
    return false;

  // Perform request based on specified request Type
  if (strlen(body) > 0) bodylen = strlen(body);

  if (strcmp(request_type, "GET") == 0) {
    if (! sendCheckReply(F("AT+HTTPACTION=0"), ok_reply, 10000))
      return false;
  }
  else if (strcmp(request_type, "POST") == 0 && bodylen > 0 ) { // POST with content body
    if (! sendCheckReply(F("AT+HTTPPARA=\"CONTENT\",\"application/json\""), ok_reply, 10000))
      return false;

    if (strlen(token) > 0) {
      char tokenStr[strlen(token) + 55];

      sprintf(tokenStr, "AT+HTTPPARA=\"USERDATA\",\"Authorization: Bearer %s\"", token);

      if (! sendCheckReply(tokenStr, ok_reply, 10000))
        return false;
    }

    char dataBuff[sizeof(bodylen) + 20];

    sprintf(dataBuff, "AT+HTTPDATA=%lu,9900", (long unsigned int)bodylen);
    if (! sendCheckReply(dataBuff, "DOWNLOAD", 10000))
      return false;

    delay(100); // Needed for fast baud rates (ex: 115200 baud with SAMD21 hardware serial)

    if (! sendCheckReply(body, ok_reply, 10000))
      return false;

    if (! sendCheckReply(F("AT+HTTPACTION=1"), ok_reply, 10000))
      return false;
  }
  else if (strcmp(request_type, "POST") == 0 && bodylen == 0) { // POST with query parameters
    if (! sendCheckReply(F("AT+HTTPACTION=1"), ok_reply, 10000))
      return false;
  }
  else if (strcmp(request_type, "HEAD") == 0) {
    if (! sendCheckReply(F("AT+HTTPACTION=2"), ok_reply, 10000))
      return false;
  }

  // Parse response status and size
  uint16_t status, datalen;
  readline(10000);
  if (! parseReply(F("+HTTPACTION:"), &status, ',', 1))
    return false;
  if (! parseReply(F("+HTTPACTION:"), &datalen, ',', 2))
    return false;

  DEBUG_PRINT("HTTP status: "); DEBUG_PRINTLN(status);
  DEBUG_PRINT("Data length: "); DEBUG_PRINTLN(datalen);

  if (status != 200) return false;

  getReply(F("AT+HTTPREAD"));

  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer); // Print out server reply

  // Terminate HTTP service
  sendCheckReply(F("AT+HTTPTERM"), ok_reply, 10000);

  return true;
}

/********************************* HTTPS FUNCTION *********************************/
boolean Adafruit_FONA::postData(const char *server, uint16_t port, const char *connType, const char *URL, const char *body) {
  // Sample request URL for SIM5320/7500/7600:
  // "GET /dweet/for/{deviceID}?temp={temp}&batt={batt} HTTP/1.1\r\nHost: dweet.io\r\n\r\n"

  // Start HTTPS stack
  if (_type == SIM7500 || _type == SIM7600) {
    getReply(F("AT+CHTTPSSTART")); // Don't check if true/false since it will return false if already started (not stopped before)

    // if (! sendCheckReply(F("AT+CHTTPSSTART"), F("+CHTTPSSTART: 0"), 10000))
    //  return false;
  }
  else {
    if (! sendCheckReply(F("AT+CHTTPSSTART"), ok_reply, 10000))
      return false;
  }

  DEBUG_PRINTLN(F("Waiting 1s to ensure connection..."));
  delay(1000);
  
  // Construct the AT command based on function parameters
  char auxStr[200];
  uint8_t connTypeNum = 1;
  
  if (strcmp(connType, "HTTP") == 0) {
    connTypeNum = 1;
  }
  if (strcmp(connType, "HTTPS") == 0) {
    connTypeNum = 2;
  }

  sprintf(auxStr, "AT+CHTTPSOPSE=\"%s\",%d,%d", server, port, connTypeNum);

  if (_type == SIM7500 || _type == SIM7600) {
    // sendParseReply(auxStr, F("+CHTTPSOPSE: "), &reply);
    // if (reply != 0) return false;

    if (! sendCheckReply(auxStr, ok_reply, 10000))
      return false;

    readline(10000);
    DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
    // if (strcmp(replybuffer, "+CHTTPSOPSE: 0") != 0) return false;
    // SIM7500E v1.1 firmware apparently doesn't have the space:
    if ((strstr(replybuffer, "+CHTTPSOPSE: 0") == NULL) || (strstr(replybuffer, "+CHTTPSOPSE:0") == NULL)) return false;
  }
  else {
    if (! sendCheckReply(auxStr, ok_reply, 10000))
      return false;
  }

  DEBUG_PRINTLN(F("Waiting 1s to make sure it works..."));
  delay(1000);

  // Send data to server
  sprintf(auxStr, "AT+CHTTPSSEND=%i", strlen(URL) + strlen(body)); // URL and body must include \r\n as needed

  if (! sendCheckReply(auxStr, ">", 10000))
    return false;

  if (_type == SIM7500 || _type == SIM7600) {
    // sendParseReply(URL, F("+CHTTPSSEND: "), &reply);
    // if (reply != 0) return false;

    // Less efficient method
    // char dataBuff[strlen(URL)+strlen(body)+1];
    // if (strlen(body) > 0) {
    //   strcpy(dataBuff, URL);
    //   strcat(dataBuff, body);
    // }

    if (strlen(body) == 0) {
      if (! sendCheckReply(URL, ok_reply, 10000))
        return false;
    }
    else {
      mySerial->print(URL);
      DEBUG_PRINT("\t---> ");
      DEBUG_PRINTLN(URL);

      if (! sendCheckReply(body, ok_reply, 10000))
        return false;
    }
    
    // if (! sendCheckReply(dataBuff, ok_reply, 10000))
    //   return false;

    readline(10000);
    DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);
    // if (strcmp(replybuffer, "+CHTTPSSEND: 0") != 0) return false;
    if (strstr(replybuffer, "+CHTTPSSEND: 0") == NULL) return false;
  }
  else {
    if (! sendCheckReply(URL, ok_reply, 10000))
      return false;
  }

  delay(1000);
  
  if (_type == SIM5320A || _type == SIM5320E) {
    if (! sendCheckReply(F("AT+CHTTPSSEND"), ok_reply, 10000))
      return false;

    readline(10000);
    DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);
    // if (strcmp(replybuffer, "+CHTTPSSEND: 0") != 0) return false;
    if (strstr(replybuffer, "+CHTTPSSEND: 0") == NULL) return false;

    delay(1000); // Needs to be here otherwise won't get server reply properly
  }

  // Check server response length
  uint16_t replyLen;
  sendParseReply(F("AT+CHTTPSRECV?"), F("+CHTTPSRECV: LEN,"), &replyLen);

  // Get server response content
  sprintf(auxStr, "AT+CHTTPSRECV=%i", replyLen);
  getReply(auxStr, 2000);

  if (replyLen > 0) {
    readRaw(replyLen);
    flushInput();
    DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);
  }
  
  // Close HTTP/HTTPS session
  sendCheckReply(F("AT+CHTTPSCLSE"), ok_reply, 10000);
  // if (! sendCheckReply(F("AT+CHTTPSCLSE"), ok_reply, 10000))
  //   return false;

  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

  // Stop HTTP/HTTPS stack
  if (! sendCheckReply(F("AT+CHTTPSSTOP"), F("+CHTTPSSTOP: 0"), 10000))
    return false;

  readline(); // Eat OK
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

  return (replyLen > 0);
}

boolean Adafruit_FONA_LTE::HTTP_connect(const char *server) {
  // Set up server URL
  char urlBuff[100];

  sendCheckReply(F("AT+SHDISC"), ok_reply, 10000); // Disconnect HTTP
  
  if (SSL_FONA) {
    sendCheckReply(F("AT+CSSLCFG=\"sslversion\",1,3"), ok_reply);
    sendCheckReply(F("AT+SHSSL=1,\"\""), ok_reply, 10000);
  }

  sprintf(urlBuff, "AT+SHCONF=\"URL\",\"%s\"", server);

  if (! sendCheckReply(urlBuff, ok_reply, 10000))
    return false;

  // Set max HTTP body length
  sendCheckReply(F("AT+SHCONF=\"BODYLEN\",1024"), ok_reply, 10000); // Max 1024 for SIM7070G

  // Set max HTTP header length
  sendCheckReply(F("AT+SHCONF=\"HEADERLEN\",350"), ok_reply, 10000); // Max 350 for SIM7070G

  // HTTP build
  sendCheckReply(F("AT+SHCONN"), ok_reply, 20000);

  // Get HTTP status
  if (!sendCheckReply(F("AT+SHSTATE?"), F("+SHSTATE: 1"))) return false;

  // getReply(F("AT+SHSTATE?"));
  // readline();
  // if (strstr(replybuffer, "+SHSTATE: 1") == NULL) return false;
  readline(); // Eat 'OK'

  // Clear HTTP header (HTTP header is appended)
  if (! sendCheckReply(F("AT+SHCHEAD"), ok_reply, 10000)) return false;

  return true;
}

boolean Adafruit_FONA_LTE::HTTP_GET(const char *URI) {
  // Use fona.HTTP_addHeader() as needed before using this function
  // Then use fona.HTTP_connect() to connect to the server first
  char cmdBuff[150];

  sprintf(cmdBuff, "AT+SHREQ=\"%s\",1", URI);

  sendCheckReply(cmdBuff, ok_reply, 10000);

  // Parse response status and size
  // Example reply --> "+SHREQ: "GET",200,387"
  uint16_t status, datalen;
  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

  if (! parseReply(F("+SHREQ: \"GET\""), &status, ',', 1))
    return false;
  if (! parseReply(F("+SHREQ: \"GET\""), &datalen, ',', 2))
    return false;

  DEBUG_PRINT("HTTP status: "); DEBUG_PRINTLN(status);
  DEBUG_PRINT("Data length: "); DEBUG_PRINTLN(datalen);

  if (status != 200) return false;

  // Read server response
  getReply(F("AT+SHREAD=0,"), datalen, 10000);
  readline();
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer); // +SHREAD: <datalen>
  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer); // Print out server reply

  sendCheckReply(F("AT+SHDISC"), ok_reply, 10000); // Disconnect HTTP

  return true;
}

boolean Adafruit_FONA_LTE::HTTP_POST(const char *URI, const char *body, uint8_t bodylen) {
  // Use fona.HTTP_addHeader() as needed before using this function
  // Then use fona.HTTP_connect() to connect to the server first
  char cmdBuff[150]; // Make sure this is large enough for URI

  // Example 2 in HTTP(S) app note for SIM7070 POST request
  // if (_type == SIM7070) {
  //   sprintf(cmdBuff, "AT+SHBOD=%i,10000", bodylen);
  //   getReply(cmdBuff, 10000);
  //   if (strstr(replybuffer, ">") == NULL) return false; // Wait for ">" to send message
  //   sendCheckReply(body, ok_reply, 2000);

  //   // if (! strcmp(replybuffer, "OK") != 0) return false; // Now send the JSON body
  // }
  // else { // For ex, SIM7000
  //   sprintf(cmdBuff, "AT+SHBOD=\"%s\",%i", body, bodylen);
  //   if (! sendCheckReply(cmdBuff, ok_reply, 10000)) return false;
  // }
  
  memset(cmdBuff, 0, sizeof(cmdBuff)); // Clear URI char array
  sprintf(cmdBuff, "AT+SHREQ=\"%s\",3", URI);

  if (! sendCheckReply(cmdBuff, ok_reply, 10000)) return false;



  // Parse response status and size
  // Example reply --> "+SHREQ: "POST",200,452"
  uint16_t status, datalen;
  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

  if (! parseReply(F("+SHREQ: \"POST\""), &status, ',', 1))
    return false;
  if (! parseReply(F("+SHREQ: \"POST\""), &datalen, ',', 2))
    return false;

  DEBUG_PRINT("HTTP status: "); DEBUG_PRINTLN(status);
  DEBUG_PRINT("Data length: "); DEBUG_PRINTLN(datalen);

  if (status != 200) return false;

  // Read server response
  getReply(F("AT+SHREAD=0,"), datalen, 10000);
  readline();
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer); // +SHREAD: <datalen>
  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer); // Print out server reply

  sendCheckReply(F("AT+SHDISC"), ok_reply, 10000); // Disconnect HTTP

  return true;
}

/********* FTP FUNCTIONS  ************************************/
boolean Adafruit_FONA::FTP_Connect(const char* serverIP, uint16_t port, const char* username, const char* password) {
  char auxStr[100];

  // if (! sendCheckReply(F("AT+FTPCID=1"), ok_reply, 10000))
  //   return false;

  sendCheckReply(F("AT+FTPCID=1"), ok_reply, 10000); // Don't return false in case this is a reconnect attempt

  sprintf(auxStr, "AT+FTPSERV=\"%s\"", serverIP);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  if (port != 21) {
    sprintf(auxStr, "AT+FTPPORT=%i", port);

    if (! sendCheckReply(auxStr, ok_reply, 10000))
      return false;
  }

  if (strlen(username) > 0) {
    sprintf(auxStr, "AT+FTPUN=\"%s\"", username);

    if (! sendCheckReply(auxStr, ok_reply, 10000))
      return false;
  }

  if (strlen(password) > 0) {
    sprintf(auxStr, "AT+FTPPW=\"%s\"", password);

    if (! sendCheckReply(auxStr, ok_reply, 10000))
      return false;
  }

  return true;
}

boolean Adafruit_FONA::FTP_Quit() {
  if (! sendCheckReply(F("AT+FTPQUIT"), ok_reply, 10000))
    return false;

  return true;
}

boolean Adafruit_FONA::FTP_Rename(const char* filePath, const char* oldName, const char* newName) {
  char auxStr[50];

  sprintf(auxStr, "AT+FTPGETPATH=\"%s\"", filePath);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  sprintf(auxStr, "AT+FTPGETNAME=\"%s\"", oldName);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  sprintf(auxStr, "AT+FTPPUTNAME=\"%s\"", newName);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  if (! sendCheckReply(F("AT+FTPRENAME"), ok_reply, 2000))
    return false;

  if (! expectReply(F("+FTPRENAME: 1,0"))) return false;

  return true;
}

boolean Adafruit_FONA::FTP_Delete(const char* fileName, const char* filePath) {
  char auxStr[50];

  sprintf(auxStr, "AT+FTPGETNAME=\"%s\"", fileName);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  sprintf(auxStr, "AT+FTPGETPATH=\"%s\"", filePath);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  if (! sendCheckReply(F("AT+FTPDELE"), ok_reply, 2000)) // It's NOT AT+FTPDELE=1
    return false;

  if (! expectReply(F("+FTPDELE: 1,0"))) return false;

  return true;
}

// boolean Adafruit_FONA::FTP_MDTM(const char* fileName, const char* filePath, char & timestamp) {
boolean Adafruit_FONA::FTP_MDTM(const char* fileName, const char* filePath, uint16_t* year,
                                uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* minute, uint8_t* second) {
  char auxStr[50];

  sprintf(auxStr, "AT+FTPGETNAME=\"%s\"", fileName);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  sprintf(auxStr, "AT+FTPGETPATH=\"%s\"", filePath);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  if (! sendCheckReply(F("AT+FTPMDTM"), ok_reply, 2000))
    return false;

  readline(10000);
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  if (strstr(replybuffer, "+FTPMDTM: 1,0,") == NULL) return false;

  char timestamp[20];
  strcpy(timestamp, replybuffer + 14);
  // DEBUG_PRINTLN(timestamp); // DEBUG

  // Timestamp format for SIM7000 is YYYYMMDDHHMMSS
  memset(auxStr, 0, sizeof(auxStr)); // Clear auxStr contents
  strncpy(auxStr, timestamp, 4);
  *year = atoi(auxStr);

  memset(auxStr, 0, sizeof(auxStr));
  strncpy(auxStr, timestamp + 4, 2);
  *month = atoi(auxStr);

  strncpy(auxStr, timestamp + 6, 2);
  *day = atoi(auxStr);

  strncpy(auxStr, timestamp + 8, 2);
  *hour = atoi(auxStr);

  strncpy(auxStr, timestamp + 10, 2);
  *minute = atoi(auxStr);

  strncpy(auxStr, timestamp + 12, 2);
  *second = atoi(auxStr);

  return true;
}

const char * Adafruit_FONA::FTP_GET(const char* fileName, const char* filePath, uint16_t numBytes) {
  char auxStr[100];
  const char *err = "error";

  sprintf(auxStr, "AT+FTPGETNAME=\"%s\"", fileName);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return err;

  sprintf(auxStr, "AT+FTPGETPATH=\"%s\"", filePath);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return err;

  if (! sendCheckReply(F("AT+FTPGET=1"), ok_reply, 10000))
    return err;

  if (! expectReply(F("+FTPGET: 1,1"))) return err;

  if (numBytes <= 1024) {
    sprintf(auxStr, "AT+FTPGET=2,%i", numBytes);
    getReply(auxStr, 10000);
    if (strstr(replybuffer, "+FTPGET: 2,") == NULL) return err;
  } 
  else {
    sprintf(auxStr, "AT+FTPEXTGET=2,%i,10000", numBytes);
    getReply(auxStr, 10000);
    if (strstr(replybuffer, "+FTPEXTGET: 2,") == NULL) return err;
  }

  readline(10000);
  DEBUG_PRINT("\t<--- "); DEBUG_PRINTLN(replybuffer);

  // if (! expectReply(ok_reply)) return err;

  // if (! expectReply(F("+FTPGET: 1,0"))) return err;

  return replybuffer;
}

boolean Adafruit_FONA::FTP_PUT(const char* fileName, const char* filePath, char* content, size_t numBytes) {
  char auxStr[100];

  sprintf(auxStr, "AT+FTPPUTNAME=\"%s\"", fileName);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  sprintf(auxStr, "AT+FTPPUTPATH=\"%s\"", filePath);

  if (! sendCheckReply(auxStr, ok_reply, 10000))
    return false;

  // Use extended PUT method if there's more than 1024 bytes to send
  if (numBytes >= 1024) {
    // Repeatedly PUT data until all data is sent
    uint32_t remBytes = numBytes;
    uint16_t offset = 0; // Data offset
    char sendArray[strlen(content)+1];
    strcpy(sendArray, content);

    while (remBytes > 0) {
      if (! sendCheckReply(F("AT+FTPEXTPUT=1"), ok_reply, 10000))
        return false;

      if (remBytes >= 300000) {
        sprintf(auxStr, "AT+FTPEXTPUT=2,%i,300000,10000", offset); // Extended PUT handles up to 300k
        offset = offset + 300000;
        remBytes = remBytes - 300000;

        strcpy(sendArray, content - offset); // Chop off the beginning
        if (strlen(sendArray) > 300000) strcpy(sendArray, sendArray - 300000); // Chop off the end
      }
      else {
        sprintf(auxStr, "AT+FTPEXTPUT=2,%i,%i,10000", offset, remBytes);
        remBytes = 0;
      }

      if (! sendCheckReply(auxStr, F("+FTPEXTPUT: 0,"), 10000))
        return false;

      if (! sendCheckReply(sendArray, ok_reply, 10000))
        return false;
    }
  }

  if (! sendCheckReply(F("AT+FTPPUT=1"), ok_reply, 10000))
    return false;

  uint16_t maxlen;
  readline(10000);
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  
  // Use regular FTPPUT method if there is less than 1024 bytes of data to send
  if (numBytes < 1024) {
    if (! parseReply(F("+FTPPUT: 1,1"), &maxlen, ',', 1))
        return false;

    // DEBUG_PRINTLN(maxlen); // DEBUG

    // Repeatedly PUT data until all data is sent
    uint16_t remBytes = numBytes;

    while (remBytes > 0) {
      if (remBytes > maxlen) sprintf(auxStr, "AT+FTPPUT=2,%i", maxlen);
      else sprintf(auxStr, "AT+FTPPUT=2,%i", remBytes);

      getReply(auxStr);

      uint16_t sentBytes;
      if (! parseReply(F("+FTPPUT: 2"), &sentBytes, ',', 1))
        return false;

      // DEBUG_PRINTLN(sentBytes); // DEBUG

      if (! sendCheckReply(content, ok_reply, 10000))
        return false;

      remBytes = remBytes - sentBytes; // Decrement counter

      // Check again for max length to send, repeat if needed
      // readline(10000);
      // DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
      // if (! parseReply(F("+FTPPUT: 1,1"), &maxlen, ',', 1))
      //   return false;
    }

    // No more data to be uploaded
    if (! sendCheckReply(F("AT+FTPPUT=2,0"), ok_reply, 10000))
      return false;

    if (! expectReply(F("+FTPPUT: 1,0"))) return false;
  }
  else {
    if (strcmp(replybuffer, "+FTPPUT: 1,0") != 0) return false;

    if (! sendCheckReply(F("AT+FTPEXTPUT=0"), ok_reply, 10000))
      return false;
  }

  return true;
}

/********* MQTT FUNCTIONS  ************************************/

////////////////////////////////////////////////////////////
void Adafruit_FONA::mqtt_connect_message(const char *protocol, byte *mqtt_message, const char *clientID, const char *username, const char *password) {
  uint8_t i = 0;
  byte protocol_length = strlen(protocol);
  byte ID_length = strlen(clientID);
  byte username_length = strlen(username);
  byte password_length = strlen(password);

  mqtt_message[0] = 16;                      // MQTT message type CONNECT

  byte rem_length = 6 + protocol_length;
  // Each parameter will add 2 bytes + parameter length
  if (ID_length > 0) {
    rem_length += 2 + ID_length;
  }
  if (username_length > 0) {
    rem_length += 2 + username_length;
  }
  if (password_length > 0) {
    rem_length += 2 + password_length;
  }

  mqtt_message[1] = rem_length;              // Remaining length of message
  mqtt_message[2] = 0;                       // Protocol name length MSB
  mqtt_message[3] = protocol_length;         // Protocol name length LSB

  // Use the given protocol name (for example, "MQTT" or "MQIsdp")
  for (int i=0; i<protocol_length; i++) {
    mqtt_message[4 + i] = byte(protocol[i]);
  }

  mqtt_message[4 + protocol_length] = 3;                      // MQTT protocol version

  if (username_length > 0 && password_length > 0) { // has everything
    mqtt_message[5 + protocol_length] = 194;                  // Connection flag with username and password (11000010)
  }
  else if (password_length == 0) { // Only has username
    mqtt_message[5 + protocol_length] = 130;                  // Connection flag with username only (10000010)
  }
  else if (username_length == 0) {  // Only has password
    mqtt_message[5 + protocol_length] = 66;                   // Connection flag with password only (01000010)
  }
  
  mqtt_message[6 + protocol_length] = 0;                      // Keep-alive time MSB
  mqtt_message[7 + protocol_length] = 15;                     // Keep-alive time LSB
  mqtt_message[8 + protocol_length] = 0;                      // Client ID length MSB
  mqtt_message[9 + protocol_length] = ID_length;              // Client ID length LSB

  // Client ID
  for(i = 0; i < ID_length; i++) {
    mqtt_message[10 + protocol_length + i] = clientID[i];
  }

  // Username
  if (username_length > 0) {
    mqtt_message[10 + protocol_length + ID_length] = 0;                     // username length MSB
    mqtt_message[11 + protocol_length + ID_length] = username_length;       // username length LSB

    for(i = 0; i < username_length; i++) {
      mqtt_message[12 + protocol_length + ID_length + i] = username[i];
    }
  }
  
  // Password
  if (password_length > 0) {
    mqtt_message[12 + protocol_length + ID_length + username_length] = 0;                     // password length MSB
    mqtt_message[13 + protocol_length + ID_length + username_length] = password_length;       // password length LSB

    for(i = 0; i < password_length; i++) {
      mqtt_message[14 + protocol_length + ID_length + username_length + i] = password[i];
    }
  }
}

void Adafruit_FONA::mqtt_publish_message(byte *mqtt_message, const char *topic, const char *message) {
  uint8_t i = 0;
  byte topic_length = strlen(topic);
  byte message_length = strlen(message);

  mqtt_message[0] = 48;                                  // MQTT Message Type PUBLISH
  mqtt_message[1] = 2 + topic_length + message_length;   // Remaining length
  mqtt_message[2] = 0;                                   // Topic length MSB
  mqtt_message[3] = topic_length;                        // Topic length LSB

  // Topic
  for(i = 0; i < topic_length; i++) {
    mqtt_message[4 + i] = topic[i];
  }

  // Message
  for(i = 0; i < message_length; i++) {
    mqtt_message[4 + topic_length + i] = message[i];
  }
}

void Adafruit_FONA::mqtt_subscribe_message(byte *mqtt_message, const char *topic, byte QoS) {
  uint8_t i = 0;
  byte topic_length = strlen(topic);

  mqtt_message[0] = 130;                // MQTT Message Type SUBSCRIBE
  mqtt_message[1] = 5 + topic_length;   // Remaining length
  mqtt_message[2] = 0;                  // Packet ID MSB   
  mqtt_message[3] = 1;                  // Packet ID LSB
  mqtt_message[4] = 0;                  // Topic length MSB      
  mqtt_message[5] = topic_length;       // Topic length LSB          

  // Topic
  for(i = 0; i < topic_length; i++) {
      mqtt_message[6 + i] = topic[i];
  }

  mqtt_message[6 + topic_length] = QoS;   // QoS byte
}

void Adafruit_FONA::mqtt_disconnect_message(byte *mqtt_message) {
  mqtt_message[0] = 0xE0; // msgtype = connect
  mqtt_message[1] = 0x00; // length of message (?)
}

boolean Adafruit_FONA::mqtt_sendPacket(byte *packet, byte len) {
  // Send packet and get response
  DEBUG_PRINT(F("\t---> "));

  for (int j = 0; j < len; j++) {
    // if (packet[j] == NULL) break; // We've reached the end of the actual content
    mySerial->write(packet[j]); // Needs to be "write" not "print"
    DEBUG_PRINT(packet[j]); // Message contents
    DEBUG_PRINT(" "); // Space out the bytes
  }
  mySerial->write(byte(26)); // End of packet
  DEBUG_PRINT(byte(26));

  readline(3000); // Wait up to 3 seconds to send the data
  DEBUG_PRINTLN("");
  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, "SEND OK") == 0);
}

////////////////////////////////////////////////////////////

boolean Adafruit_FONA::MQTTconnect(const char *protocol, const char *clientID, const char *username, const char *password) {
  flushInput();
  mySerial->println(F("AT+CIPSEND"));
  readline();
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  if (replybuffer[0] != '>') return false;

  byte mqtt_message[127];
  mqtt_connect_message(protocol, mqtt_message, clientID, username, password);


  if (! mqtt_sendPacket(mqtt_message, 14+strlen(protocol)+strlen(clientID)+strlen(username)+strlen(password))) return false;

  return true;
}

boolean Adafruit_FONA::MQTTpublish(const char* topic, const char* message) {
  flushInput();
  mySerial->println(F("AT+CIPSEND"));
  readline();
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  if (replybuffer[0] != '>') return false;

  byte mqtt_message[127];
  mqtt_publish_message(mqtt_message, topic, message);

  if (!mqtt_sendPacket(mqtt_message, 4+strlen(topic)+strlen(message))) return false;

  return true;
}

boolean Adafruit_FONA::MQTTsubscribe(const char* topic, byte QoS) {
  flushInput();
  mySerial->println(F("AT+CIPSEND"));
  readline();
  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);
  if (replybuffer[0] != '>') return false;

  byte mqtt_message[127];
  mqtt_subscribe_message(mqtt_message, topic, QoS);

  if (!mqtt_sendPacket(mqtt_message, 7+strlen(topic))) return false;

  return true;
}

boolean Adafruit_FONA::MQTTunsubscribe(const char* topic) {
  return false;
}

boolean Adafruit_FONA::MQTTreceive(const char* topic, const char* buf, int maxlen) {
  return false;
}

boolean Adafruit_FONA::MQTTdisconnect(void) {
  return false;
}

/********* SIM7000 MQTT FUNCTIONS  ************************************/
// Set MQTT parameters
// Parameter tags can be "CLIENTID", "URL", "KEEPTIME", "CLEANSS", "USERNAME",
// "PASSWORD", "QOS", "TOPIC", "MESSAGE", or "RETAIN"
boolean Adafruit_FONA_LTE::MQTT_setParameter(const char* paramTag, const char* paramValue, uint16_t port) {
  char cmdStr[255];

  if (port == 0) sprintf(cmdStr, "AT+SMCONF=\"%s\",\"%s\"", paramTag, paramValue); // Quoted paramValue
  else sprintf(cmdStr, "AT+SMCONF=\"%s\",\"%s\",%i", paramTag, paramValue, port);
  if (! sendCheckReply(cmdStr, ok_reply)) return false;

  // if (strcmp(paramTag, "CLIENTID") == 0 || strcmp(paramTag, "URL") == 0 || strcmp(paramTag, "TOPIC") == 0 || strcmp(paramTag, "MESSAGE") == 0) {
  //   if (port == 0) sprintf(cmdStr, "AT+SMCONF=\"%s\",\"%s\"", paramTag, paramValue); // Quoted paramValue
  //   else sprintf(cmdStr, "AT+SMCONF=\"%s\",\"%s\",%i", paramTag, paramValue, port);
  //   if (! sendCheckReply(cmdStr, ok_reply)) return false;
  // }
  // else {
  //   sprintf(cmdStr, "AT+SMCONF=\"%s\",%s", paramTag, paramValue); // Unquoted paramValue
  //   if (! sendCheckReply(cmdStr, ok_reply)) return false;
  // }
  
  return true;
}

// Connect or disconnect MQTT
boolean Adafruit_FONA_LTE::MQTT_connect(bool yesno) {
  if (yesno) return sendCheckReply(F("AT+SMCONN"), ok_reply, 5000);
  else return sendCheckReply(F("AT+SMDISC"), ok_reply);
}

// Query MQTT connection status
boolean Adafruit_FONA_LTE::MQTT_connectionStatus(void) {
  if (! sendCheckReply(F("AT+SMSTATE?"), F("+SMSTATE: 1"))) return false;
  return true;
}

// Subscribe to specified MQTT topic
// QoS can be from 0-2
boolean Adafruit_FONA_LTE::MQTT_subscribe(const char* topic, byte QoS) {
  char cmdStr[127];
  sprintf(cmdStr, "AT+SMSUB=\"%s\",%i", topic, QoS);

  if (! sendCheckReply(cmdStr, ok_reply)) return false;
  return true;
}

// Unsubscribe from specified MQTT topic
boolean Adafruit_FONA_LTE::MQTT_unsubscribe(const char* topic) {
  char cmdStr[64];
  sprintf(cmdStr, "AT+SMUNSUB=\"%s\"", topic);
  if (! sendCheckReply(cmdStr, ok_reply)) return false;
  return true;
}

// Publish to specified topic
// Message length can be from 0-512 bytes
// QoS can be from 0-2
// Server hold message flag can be 0 or 1
boolean Adafruit_FONA_LTE::MQTT_publish(const char* topic, const char* message, uint16_t contentLength, byte QoS, byte retain) {
  char cmdStr[127];
  sprintf(cmdStr, "AT+SMPUB=\"%s\",%i,%i,%i", topic, contentLength, QoS, retain);

  getReply(cmdStr, 2000);
  if (strstr(replybuffer, ">") == NULL) return false; // Wait for "> " to send message
  if (! sendCheckReply(message, ok_reply, 5000)) return false; // Now send the message

  return true;
}

// Change MQTT data format to hex
// Enter "true" if you want hex, "false" if you don't
boolean Adafruit_FONA_LTE::MQTT_dataFormatHex(bool yesno) {
  return sendCheckReply(F("AT+SMPUBHEX="), yesno, ok_reply);
}

/********* SSL FUNCTIONS  ************************************/
boolean Adafruit_FONA::addRootCA(const char *root_cert) {
  char rootCA[10240];
  strcpy(rootCA,root_cert);
  rootCA_FONA = rootCA;
  if (!strlen(rootCA_FONA)) return false;

  return true;
}

/********* UDP FUNCTIONS  ************************************/

boolean Adafruit_FONA::UDPconnect(char *server, uint16_t port) {
  flushInput();

  // close all old connections
  if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 8000) ) return false;

  // single connection at a time
  if (! sendCheckReply(F("AT+CIPMUX=0"), ok_reply) ) return false;

  // manually read data
  if (! sendCheckReply(F("AT+CIPRXGET=1"), ok_reply) ) return false;


  DEBUG_PRINT(F("AT+CIPSTART=\"UDP\",\""));
  DEBUG_PRINT(server);
  DEBUG_PRINT(F("\",\""));
  DEBUG_PRINT(port);
  DEBUG_PRINTLN(F("\""));


  mySerial->print(F("AT+CIPSTART=\"TCP\",\""));
  mySerial->print(server);
  mySerial->print(F("\",\""));
  mySerial->print(port);
  mySerial->println(F("\""));

  if (! expectReply(ok_reply)) return false;
  if (! expectReply(F("CONNECT OK"))) return false;

  // looks like it was a success (?)
  return true;
}

/**************** TCP FUNCTIONS + SSL *************************/


boolean Adafruit_FONA::TCPconnect(char *server, uint16_t port) {
  if (SSL_FONA) {
    flushInput();
    
    //  Report Mobile Equipment Error
    if (! sendCheckReply(F("AT+CMEE=2"), ok_reply) ) return false;
    
    // Check config error
    if (! sendCheckReply(F("AT+CMEE?"), F("+CMEE: 2"), 20000) ) return false;
    
    //Set TCP/UDP Identifier
    if (! sendCheckReply(F("AT+CACID=1"), ok_reply) ) return false;
    CID_CA_FONA = 1;
    
    //Configure SSL Parameters of a Context Identifier
    if (! sendCheckReply(F("AT+CSSLCFG=\"sslversion\",1,3"), ok_reply) ) return false;
    if (! sendCheckReply(F("AT+CSSLCFG=\"protocol\",1,1"), ok_reply) ) return false;
    mySerial->println(F("AT+CSSLCFG=\"ctxindex\",1"));
    readline();
    if (! expectReply(ok_reply)) return false;
    
    if (! sendCheckReply(F("AT+CFSINIT"), ok_reply) ) return false;
    
    //Load CA
    mySerial->print(F("AT+CFSWFILE=3,\"ca.crt\",0,\""));
    mySerial->print(strlen(rootCA_FONA));
    mySerial->print(F("\",\""));
    mySerial->print(5000);
    mySerial->println(F("\""));
    
    if (! expectReply(F("DOWNLOAD"))) return false;
    
    mySerial->print(rootCA_FONA);
    readline(2000, true);
    if (!((replybuffer[0] == 'O') && (replybuffer[1] == 'K'))) return false;
    
    
    if (! sendCheckReply(F("AT+CFSTERM"), ok_reply) ) return false;
    if (! sendCheckReply(F("AT+CFSINIT"), ok_reply) ) return false;
    
    char CF[20] = "+CFSGFIS: ";
    itoa((int)strlen(rootCA_FONA), CF+10, 10);
    
    // if (! sendCheckReply(F("AT+CFSGFIS=3,\"ca.crt\""), (char*)CF, 300)) return false; // Get cert file size
    if (! sendCheckReply(F("AT+CFSTERM"), ok_reply) ) return false;

    if (! sendCheckReply(F("AT+CSSLCFG=\"convert\",2,\"ca.crt\""), ok_reply) ) return false;
    
    //Set SSL certificate and timeout parameters
    if (! sendCheckReply(F("AT+CASSLCFG=1,\"cacert\",\"ca.crt\""), ok_reply) ) return false;
    if (! sendCheckReply(F("AT+CASSLCFG=1,\"ssl\",1"), ok_reply) ) return false;
    if (! sendCheckReply(F("AT+CASSLCFG=1,\"crindex\",1"), ok_reply) ) return false;
    if (! sendCheckReply(F("AT+CASSLCFG=1,\"protocol\",0"), ok_reply) ) return false;
    
    // if (! openWirelessConnection(true)) return false;
    // if (! wirelessConnStatus()) return false;
    
    char server_f[100];
    strcpy(server_f,server);
    server_CA_FONA = server_f;
    port_CA_FONA = port;
    
    mySerial->print(F("AT+CAOPEN=1,\""));
    mySerial->print(server);
    mySerial->print(F("\",\""));
    mySerial->print(port);
    mySerial->println(F("\""));
    if (! expectReply(F("+CAOPEN: 1,0"))) return false;
  }
  else {
    flushInput();

    // close all old connections
    if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000) ) return false;

    // single connection at a time
    if (! sendCheckReply(F("AT+CIPMUX=0"), ok_reply) ) return false;

    // manually read data
    if (! sendCheckReply(F("AT+CIPRXGET=1"), ok_reply) ) return false;


    DEBUG_PRINT(F("AT+CIPSTART=\"TCP\",\""));
    DEBUG_PRINT(server);
    DEBUG_PRINT(F("\",\""));
    DEBUG_PRINT(port);
    DEBUG_PRINTLN(F("\""));


    mySerial->print(F("AT+CIPSTART=\"TCP\",\""));
    mySerial->print(server);
    mySerial->print(F("\",\""));
    mySerial->print(port);
    mySerial->println(F("\""));

    if (! expectReply(ok_reply)) return false;
    if (! expectReply(F("CONNECT OK"))) return false;
  }

  // looks like it was a success (?)
  return true;
}

boolean Adafruit_FONA::TCPclose(void) {
  return sendCheckReply(F("AT+CIPCLOSE"), F("CLOSE OK"));
}

boolean Adafruit_FONA::TCPconnected(void) {
  if (SSL_FONA) {  
    char CA[100] = "+CAOPEN: ";
    itoa(CID_CA_FONA, CA+9, 10);
    strcat(CA,",\""); 
    strcat(CA,server_CA_FONA);  
    strcat(CA,"\",");
    char port_CA_FONA_p[10];
    itoa((int)port_CA_FONA,port_CA_FONA_p, 10);
    strcat(CA,port_CA_FONA_p);

    getReply(F("AT+CAOPEN?"));

    if (strstr(replybuffer, CA) == NULL) return false;
  }
  else {
    if (! sendCheckReply(F("AT+CIPSTATUS"), ok_reply, 100) ) return false;
  }

  readline(100);
  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (strcmp(replybuffer, "STATE: CONNECT OK") == 0);
}

boolean Adafruit_FONA::TCPsend(char *packet, uint8_t len) {

  DEBUG_PRINT(F("AT+CIPSEND="));
  DEBUG_PRINTLN(len);
#ifdef ADAFRUIT_FONA_DEBUG
  for (uint16_t i=0; i<len; i++) {
  DEBUG_PRINT(F(" 0x"));
  DEBUG_PRINT(packet[i], HEX);
  }
#endif
  DEBUG_PRINTLN();

  if (SSL_FONA) {
    flushInput();
    mySerial->print(F("AT+CASEND=1,\""));
    mySerial->print(len);
    mySerial->println(F("\""));
    readline();
  } 
  else {
    mySerial->print(F("AT+CIPSEND="));
    mySerial->println(len);
    readline();
  }

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  if (replybuffer[0] != '>') return false;

  mySerial->write(packet, len);
  readline(3000); // wait up to 3 seconds to send the data

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  if (SSL_FONA) return (strcmp(replybuffer, "OK") == 0);
  else return (strcmp(replybuffer, "SEND OK") == 0);
}

uint16_t Adafruit_FONA::TCPavailable(void) {
  uint16_t avail;

  if (! sendParseReply(F("AT+CIPRXGET=4"), F("+CIPRXGET: 4,"), &avail, ',', 0) ) return false;


  DEBUG_PRINT (avail); DEBUG_PRINTLN(F(" bytes available"));


  return avail;
}


uint16_t Adafruit_FONA::TCPread(uint8_t *buff, uint8_t len) {
  uint16_t avail;

  mySerial->print(F("AT+CIPRXGET=2,"));
  mySerial->println(len);
  readline();
  if (! parseReply(F("+CIPRXGET: 2,"), &avail, ',', 0)) return false;

  readRaw(avail);

#ifdef ADAFRUIT_FONA_DEBUG
  DEBUG_PRINT (avail); DEBUG_PRINTLN(F(" bytes read"));
  for (uint8_t i=0;i<avail;i++) {
  DEBUG_PRINT(F(" 0x")); DEBUG_PRINT(replybuffer[i], HEX);
  }
  DEBUG_PRINTLN();
#endif

  memcpy(buff, replybuffer, avail);

  return avail;
}

boolean Adafruit_FONA::TCPdns(char *hostname, char *buff, uint8_t len) {
  mySerial->print(F("AT+CDNSGIP="));
  mySerial->println(hostname);

  if (! expectReply(ok_reply))
     return false;

  readline();

  if (! parseReplyQuoted(F("+CDNSGIP: 1,"), buff, len, ',', 1)) {
     return false;
  }

  return true;
}


/********* HTTP LOW LEVEL FUNCTIONS  ************************************/

boolean Adafruit_FONA::HTTP_init() {
  return sendCheckReply(F("AT+HTTPINIT"), ok_reply);
}

boolean Adafruit_FONA::HTTP_term() {
  return sendCheckReply(F("AT+HTTPTERM"), ok_reply);
}

void Adafruit_FONA::HTTP_para_start(FONAFlashStringPtr parameter,
                                    boolean quoted) {
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPPARA=\""));
  DEBUG_PRINT(parameter);
  DEBUG_PRINTLN('"');


  mySerial->print(F("AT+HTTPPARA=\""));
  mySerial->print(parameter);
  if (quoted)
    mySerial->print(F("\",\""));
  else
    mySerial->print(F("\","));
}

boolean Adafruit_FONA::HTTP_para_end(boolean quoted) {
  if (quoted)
    mySerial->println('"');
  else
    mySerial->println();

  return expectReply(ok_reply);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 const char *value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 FONAFlashStringPtr value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 int32_t value) {
  HTTP_para_start(parameter, false);
  mySerial->print(value);
  return HTTP_para_end(false);
}

boolean Adafruit_FONA::HTTP_data(uint32_t size, uint32_t maxTime) {
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPDATA="));
  DEBUG_PRINT(size);
  DEBUG_PRINT(',');
  DEBUG_PRINTLN(maxTime);


  mySerial->print(F("AT+HTTPDATA="));
  mySerial->print(size);
  mySerial->print(",");
  mySerial->println(maxTime);

  return expectReply(F("DOWNLOAD"));
}

boolean Adafruit_FONA::HTTP_action(uint8_t method, uint16_t *status,
                                   uint16_t *datalen, int32_t timeout) {
  // Send request.
  if (! sendCheckReply(F("AT+HTTPACTION="), method, ok_reply))
    return false;

  // Parse response status and size.
  readline(timeout);
  if (! parseReply(F("+HTTPACTION:"), status, ',', 1))
    return false;
  if (! parseReply(F("+HTTPACTION:"), datalen, ',', 2))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_readall(uint16_t *datalen) {
  getReply(F("AT+HTTPREAD"));
  if (! parseReply(F("+HTTPREAD:"), datalen, ',', 0))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_ssl(boolean onoff) {
  return sendCheckReply(F("AT+HTTPSSL="), onoff ? 1 : 0, ok_reply);
}

boolean Adafruit_FONA_LTE::HTTP_addHeader(const char *type, const char *value, uint16_t maxlen) {
  char cmdStr[2*maxlen+17];

  sprintf(cmdStr, "AT+SHAHEAD=\"%s\",\"%s\"", type, value);

  if (! sendCheckReply(cmdStr, ok_reply, 10000))
    return false;
  return true;
}

boolean Adafruit_FONA_LTE::HTTP_addPara(const char *key, const char *value, uint16_t maxlen) {
  char cmdStr[2*maxlen+16];

  sprintf(cmdStr, "AT+SHPARA=\"%s\",\"%s\"", key, value);

  if (! sendCheckReply(cmdStr, ok_reply, 10000))
    return false;
  return true;
}

/********* HTTP HIGH LEVEL FUNCTIONS ***************************/

boolean Adafruit_FONA::HTTP_GET_start(char *url, uint16_t *status, uint16_t *datalen) {
  if (! HTTP_setup(url))
    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen, 30000))
    return false;

  DEBUG_PRINT(F("Status: ")); DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: ")); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

/*
boolean Adafruit_FONA_3G::HTTP_GET_start(char *ipaddr, char *path, uint16_t port
              uint16_t *status, uint16_t *datalen){
  char send[100] = "AT+CHTTPACT=\"";
  char *sendp = send + strlen(send);
  memset(sendp, 0, 100 - strlen(send));

  strcpy(sendp, ipaddr);
  sendp+=strlen(ipaddr);
  sendp[0] = '\"';
  sendp++;
  sendp[0] = ',';
  itoa(sendp, port);
  getReply(send, 500);

  return;

  if (! HTTP_setup(url))

    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen))
    return false;

  DEBUG_PRINT("Status: "); DEBUG_PRINTLN(*status);
  DEBUG_PRINT("Len: "); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}
*/

void Adafruit_FONA::HTTP_GET_end(void) {
  HTTP_term();
}

boolean Adafruit_FONA::HTTP_POST_start(char *url,
              FONAFlashStringPtr contenttype,
              const uint8_t *postdata, uint16_t postdatalen,
              uint16_t *status, uint16_t *datalen){
  if (! HTTP_setup(url))
    return false;

  if (! HTTP_para(F("CONTENT"), contenttype)) {
    return false;
  }

  // HTTP POST data
  if (! HTTP_data(postdatalen, 10000))
    return false;
  mySerial->write(postdata, postdatalen);
  if (! expectReply(ok_reply))
    return false;

  // HTTP POST
  if (! HTTP_action(FONA_HTTP_POST, status, datalen))
    return false;

  DEBUG_PRINT(F("Status: ")); DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: ")); DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_POST_end(void) {
  HTTP_term();
}

void Adafruit_FONA::setUserAgent(FONAFlashStringPtr useragent) {
  this->useragent = useragent;
}

void Adafruit_FONA::setHTTPSRedirect(boolean onoff) {
  httpsredirect = onoff;
}

/********* HTTP HELPERS ****************************************/

boolean Adafruit_FONA::HTTP_setup(char *url) {
  // Handle any pending
  HTTP_term();

  // Initialize and set parameters
  if (! HTTP_init())
    return false;
  if (! HTTP_para(F("CID"), 1))
    return false;
  if (! HTTP_para(F("UA"), useragent))
    return false;
  if (! HTTP_para(F("URL"), url))
    return false;

  // HTTPS redirect
  if (httpsredirect) {
    if (! HTTP_para(F("REDIR"), 1))
      return false;

    if (! HTTP_ssl(true))
      return false;
  }

  return true;
}

/********* HELPERS *********************************************/

boolean Adafruit_FONA::expectReply(FONAFlashStringPtr reply,
                                   uint16_t timeout) {
  readline(timeout);

  DEBUG_PRINT(F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

/********* LOW LEVEL *******************************************/

inline int Adafruit_FONA::available(void) {
  return mySerial->available();
}

inline size_t Adafruit_FONA::write(uint8_t x) {
  return mySerial->write(x);
}

inline int Adafruit_FONA::read(void) {
  return mySerial->read();
}

inline int Adafruit_FONA::peek(void) {
  return mySerial->peek();
}

inline void Adafruit_FONA::flush() {
  mySerial->flush();
}

void Adafruit_FONA::flushInput() {
    // Read all available serial input to flush pending data.
    uint16_t timeoutloop = 0;
    while (timeoutloop++ < 40) {
        while(available()) {
            read();
            timeoutloop = 0;  // If char was received reset the timer
        }
        delay(1);
    }
}

uint16_t Adafruit_FONA::readRaw(uint16_t b) {
  uint16_t idx = 0;

  while (b && (idx < sizeof(replybuffer)-1)) {
    if (mySerial->available()) {
      replybuffer[idx] = mySerial->read();
      idx++;
      b--;
    }
  }
  replybuffer[idx] = 0;

  return idx;
}

uint8_t Adafruit_FONA::readline(uint16_t timeout, boolean multiline) {
  uint16_t replyidx = 0;

  while (timeout--) {
    if (replyidx >= 254) {
      //DEBUG_PRINTLN(F("SPACE"));
      break;
    }

    while(mySerial->available()) {
      char c =  mySerial->read();
      if (c == '\r') continue;
      if (c == 0xA) {
        if (replyidx == 0)   // the first 0x0A is ignored
          continue;

        if (!multiline) {
          timeout = 0;         // the second 0x0A is the end of the line
          break;
        }
      }
      replybuffer[replyidx] = c;
      //DEBUG_PRINT(c, HEX); DEBUG_PRINT("#"); DEBUG_PRINTLN(c);

      if (++replyidx >= 254)
        break;
    }

    if (timeout == 0) {
      //DEBUG_PRINTLN(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  replybuffer[replyidx] = 0;  // null term
  return replyidx;
}

uint8_t Adafruit_FONA::getReply(const char *send, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);


  mySerial->println(send);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr send, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINTLN(send);


  mySerial->println(send);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, char *suffix, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix); DEBUG_PRINTLN(suffix);


  mySerial->print(prefix);
  mySerial->println(suffix);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, int32_t suffix, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix); DEBUG_PRINTLN(suffix, DEC);


  mySerial->print(prefix);
  mySerial->println(suffix, DEC);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, suffix2, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix);
  DEBUG_PRINT(suffix1, DEC); DEBUG_PRINT(','); DEBUG_PRINTLN(suffix2, DEC);


  mySerial->print(prefix);
  mySerial->print(suffix1);
  mySerial->print(',');
  mySerial->println(suffix2, DEC);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, ", suffix, ", and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, uint16_t timeout) {
  flushInput();


  DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(prefix);
  DEBUG_PRINT('"'); DEBUG_PRINT(suffix); DEBUG_PRINTLN('"');


  mySerial->print(prefix);
  mySerial->print('"');
  mySerial->print(suffix);
  mySerial->println('"');

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- ")); DEBUG_PRINTLN(replybuffer);

  return l;
}

boolean Adafruit_FONA::sendCheckReply(const char *send, const char *reply, uint16_t timeout) {
  if (! getReply(send, timeout) )
    return false;
/*
  for (uint8_t i=0; i<strlen(replybuffer); i++) {
  DEBUG_PRINT(replybuffer[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  for (uint8_t i=0; i<strlen(reply); i++) {
    DEBUG_PRINT(reply[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  */
  return (strcmp(replybuffer, reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr send, FONAFlashStringPtr reply, uint16_t timeout) {
  if (! getReply(send, timeout) )
    return false;

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(const char* send, FONAFlashStringPtr reply, uint16_t timeout) {
  if (! getReply(send, timeout) )
    return false;
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}


// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, char *suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, suffix2, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix1, suffix2, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, ", suffix, ", and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReplyQuoted(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}


boolean Adafruit_FONA::parseReply(FONAFlashStringPtr toreply,
          uint16_t *v, char divider, uint8_t index) {
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //DEBUG_PRINTLN(p);

  }
  *v = atoi(p);

  return true;
}

boolean Adafruit_FONA::parseReply(FONAFlashStringPtr toreply,
          char *v, char divider, uint8_t index) {
  uint8_t i=0;
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);

  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  for(i=0; i<strlen(p);i++) {
    if(p[i] == divider)
      break;
    v[i] = p[i];
  }

  v[i] = '\0';

  return true;
}

// Parse a quoted string in the response fields and copy its value (without quotes)
// to the specified character array (v).  Only up to maxlen characters are copied
// into the result buffer, so make sure to pass a large enough buffer to handle the
// response.
boolean Adafruit_FONA::parseReplyQuoted(FONAFlashStringPtr toreply,
          char *v, int maxlen, char divider, uint8_t index) {
  uint8_t i=0, j;
  // Verify response starts with toreply.
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);

  // Find location of desired response field.
  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
  }

  // Copy characters from response field into result string.
  for(i=0, j=0; j<maxlen && i<strlen(p); ++i) {
    // Stop if a divier is found.
    if(p[i] == divider)
      break;
    // Skip any quotation marks.
    else if(p[i] == '"')
      continue;
    v[j++] = p[i];
  }

  // Add a null terminator if result string buffer was not filled.
  if (j < maxlen)
    v[j] = '\0';

  return true;
}

boolean Adafruit_FONA::sendParseReply(FONAFlashStringPtr tosend,
              FONAFlashStringPtr toreply,
              uint16_t *v, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReply(toreply, v, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}

boolean Adafruit_FONA::parseReplyFloat(FONAFlashStringPtr toreply,
          float *f, char divider, uint8_t index) {
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //DEBUG_PRINTLN(p);

  }
  *f = atof(p);

  return true;
}

// needed for CBC and others

boolean Adafruit_FONA::sendParseReplyFloat(FONAFlashStringPtr tosend,
              FONAFlashStringPtr toreply,
              float *f, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReplyFloat(toreply, f, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}


// needed for CBC and others

boolean Adafruit_FONA_3G::sendParseReply(FONAFlashStringPtr tosend,
              FONAFlashStringPtr toreply,
              float *f, char divider, uint8_t index) {
  getReply(tosend);

  if (! parseReply(toreply, f, divider, index)) return false;

  readline(); // eat 'OK'

  return true;
}


boolean Adafruit_FONA_3G::parseReply(FONAFlashStringPtr toreply,
          float *f, char divider, uint8_t index) {
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0) return false;
  p+=prog_char_strlen((prog_char*)toreply);
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p) return false;
    p++;
    //DEBUG_PRINTLN(p);

  }
  *f = atof(p);

  return true;
}
