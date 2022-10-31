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
#ifndef ADAFRUIT_FONA_H
#define ADAFRUIT_FONA_H

#include "includes/FONAConfig.h"
#include "includes/FONAExtIncludes.h"
#include "includes/platform/FONAPlatform.h"



#define SIM800L 1
#define SIM800H 2

#define SIM808_V1 3
#define SIM808_V2 4

#define SIM5320A 5
#define SIM5320E 6

#define SIM7000  7
#define SIM7070  12
#define SIM7500  13
#define SIM7600  16

// Keep these variants here for legacy code
#define SIM7000A 8
#define SIM7000C 9
#define SIM7000E 10
#define SIM7000G 11

#define SIM7500A 14
#define SIM7500E 15

#define SIM7600A 17
#define SIM7600C 18
#define SIM7600E 19


// Set the preferred SMS storage.
//   Use "SM" for storage on the SIM.
//   Use "ME" for internal storage on the FONA chip
#define FONA_PREF_SMS_STORAGE "\"SM\""
//#define FONA_PREF_SMS_STORAGE "\"ME\""

#define FONA_HEADSETAUDIO 0
#define FONA_EXTAUDIO 1

#define FONA_STTONE_DIALTONE 1
#define FONA_STTONE_BUSY 2
#define FONA_STTONE_CONGESTION 3
#define FONA_STTONE_PATHACK 4
#define FONA_STTONE_DROPPED 5
#define FONA_STTONE_ERROR 6
#define FONA_STTONE_CALLWAIT 7
#define FONA_STTONE_RINGING 8
#define FONA_STTONE_BEEP 16
#define FONA_STTONE_POSTONE 17
#define FONA_STTONE_ERRTONE 18
#define FONA_STTONE_INDIANDIALTONE 19
#define FONA_STTONE_USADIALTONE 20

#define FONA_DEFAULT_TIMEOUT_MS 500
#define FONA_NO_RST_PIN 99

#define FONA_HTTP_GET   0
#define FONA_HTTP_POST  1
#define FONA_HTTP_HEAD  2

#define FONA_CALL_READY 0
#define FONA_CALL_FAILED 1
#define FONA_CALL_UNKNOWN 2
#define FONA_CALL_RINGING 3
#define FONA_CALL_INPROGRESS 4

#define FONA_SIM_ERROR -2
#define FONA_SIM_UNKNOWN -1
#define FONA_SIM_READY 0
#define FONA_SIM_PIN 1
#define FONA_SIM_PUK 2
#define FONA_SIM_PH_PIN 3
#define FONA_SIM_PH_PUK 4
#define FONA_SIM_PIN2 5
#define FONA_SIM_PUK2 6

#define SSL_FONA 0

class Adafruit_FONA : public FONAStreamType {
 public:
  Adafruit_FONA(int8_t);
  boolean begin(FONAStreamType &port);
  uint8_t type();

  // Stream
  int available(void);
  size_t write(uint8_t x);
  int read(void);
  int peek(void);
  void flush();

  // FONA 3G requirements
  boolean setBaudrate(uint32_t baud);

  // Power, battery, and ADC
  void powerOn(uint8_t FONA_PWRKEY);
  boolean powerDown(void);
  boolean getADCVoltage(uint16_t *v);
  boolean getBattPercent(uint16_t *p);
  boolean getBattVoltage(uint16_t *v);

  // Functionality and operation mode settings
  boolean setFunctionality(uint8_t option); // AT+CFUN command
  boolean enableSleepMode(bool onoff); // AT+CSCLK command
  boolean set_eDRX(uint8_t mode, uint8_t connType, char * eDRX_val); // AT+CEDRXS command
  boolean enablePSM(bool onoff); // AT+CPSMS command
  boolean enablePSM(bool onoff, char * TAU_val, char * activeTime_val); // AT+CPSMS command
  boolean setNetLED(bool onoff, uint8_t mode = 0, uint16_t timer_on = 64, uint16_t timer_off = 3000); // AT+CNETLIGHT and AT+SLEDS commands

  // SIM query
  uint8_t unlockSIM(const char *pin);
  int8_t getPINStatus();
  uint8_t getSIMCCID(char *ccid);
  uint8_t getNetworkStatus(void);
  uint8_t getRSSI(void);

  // IMEI
  uint8_t getIMEI(char *imei);

  // Audio output
  boolean setAudio(uint8_t a);
  boolean setVolume(uint8_t i);
  uint8_t getVolume(void);
  boolean playToolkitTone(uint8_t t, uint16_t len);
  boolean setMicVolume(uint8_t a, uint8_t level);
  boolean playDTMF(char tone);

  // FM radio functions.
  boolean tuneFMradio(uint16_t station);
  boolean FMradio(boolean onoff, uint8_t a = FONA_HEADSETAUDIO);
  boolean setFMVolume(uint8_t i);
  int8_t getFMVolume();
  int8_t getFMSignalLevel(uint16_t station);

  // SMS handling
  boolean setSMSInterrupt(uint8_t i);
  uint8_t getSMSInterrupt(void);
  int8_t getNumSMS(void);
  boolean readSMS(uint8_t i, char *smsbuff, uint16_t max, uint16_t *readsize);
  boolean sendSMS(const char *smsaddr, const char *smsmsg);
  boolean deleteSMS(uint8_t i);
  boolean deleteAllSMS(void);
  boolean getSMSSender(uint8_t i, char *sender, int senderlen);
  boolean sendUSSD(char *ussdmsg, char *ussdbuff, uint16_t maxlen, uint16_t *readlen);

  // Time
  // boolean enableNetworkTimeSync(boolean onoff);
  uint8_t getNTPstatus();
  boolean enableNTPTimeSync(boolean onoff, FONAFlashStringPtr ntpserver=0);
  boolean getTime(char *buff, uint16_t maxlen);
  
  // RTC
  boolean enableRTC(uint8_t i);
  boolean readRTC(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *hr, uint8_t *min, uint8_t *sec, int8_t *tz);

  // GPRS handling
  boolean enableGPRS(boolean onoff);
  int8_t GPRSstate(void);
  boolean getGSMLoc(uint16_t *replycode, char *buff, uint16_t maxlen);
  boolean getGSMLoc(float *lat, float *lon);
  void setNetworkSettings(FONAFlashStringPtr apn, FONAFlashStringPtr username=0, FONAFlashStringPtr password=0);
  boolean postData(const char *request_type, const char *URL, const char *body = "", const char *token = "", uint32_t bodylen = 0);
  boolean postData(const char *server, uint16_t port, const char *connType, const char *URL, const char *body = "");
  int8_t getNetworkType(char *typeStringBuffer, size_t bufferLength);
  int8_t getBearerStatus(void);
  boolean getIPv4(char *ipStringBuffer, size_t bufferLength);
  void getNetworkInfo(void);
  bool getNetworkInfoLong(void);

  // Network connection (AT+CNACT)
  boolean openWirelessConnection(bool onoff);
  boolean wirelessConnStatus(void);

  // GPS handling
  boolean enableGPS(boolean onoff);
  int8_t GPSstatus(void);
  uint8_t getGPS(uint8_t arg, char *buffer, uint8_t maxbuff);
  // boolean getGPS(float *lat, float *lon, float *speed_kph=0, float *heading=0, float *altitude=0);
  boolean getGPS(float *lat, float *lon, float *speed_kph, float *heading, float *altitude,
                              uint16_t *year = NULL, uint8_t *month = NULL, uint8_t *day = NULL, uint8_t *hour = NULL, uint8_t *min = NULL, float *sec = NULL);
  boolean enableGPSNMEA(uint8_t nmea);

  // UDP raw connections
  boolean UDPconnect(char *server, uint16_t port);

  // TCP raw connections
  boolean TCPconnect(char *server, uint16_t port);
  boolean TCPclose(void);
  boolean TCPconnected(void);
  boolean TCPsend(char *packet, uint8_t len);
  uint16_t TCPavailable(void);
  uint16_t TCPread(uint8_t *buff, uint8_t len);
  boolean TCPdns(char *hostname, char *buff, uint8_t len);
  boolean addRootCA(const char *root_cert);

  // MQTT
  boolean MQTTconnect(const char *protocol, const char *clientID, const char *username = "", const char *password = "");
  boolean MQTTdisconnect(void);
  boolean MQTTpublish(const char* topic, const char* message);
  boolean MQTTsubscribe(const char* topic, byte QoS);
  boolean MQTTunsubscribe(const char* topic);
  boolean MQTTreceive(const char* topic, const char* buf, int maxlen);

  // FTP
  boolean FTP_Connect(const char* serverIP, uint16_t port, const char* username, const char* password);
  boolean FTP_Quit();
  boolean FTP_Rename(const char* filePath, const char* oldName, const char* newName);
  boolean FTP_Delete(const char* fileName, const char* filePath);
  boolean FTP_MDTM(const char* fileName, const char* filePath, uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* minute, uint8_t* second);
  // boolean FTP_GET(const char* fileName, const char* filePath, uint16_t numBytes, char * replybuffer);
  const char * FTP_GET(const char* fileName, const char* filePath, uint16_t numBytes);
  boolean FTP_PUT(const char* fileName, const char* filePath, char* content, size_t numBytes);

  // HTTP low level interface (maps directly to SIM800 commands).
  boolean HTTP_init();
  boolean HTTP_term();
  void HTTP_para_start(FONAFlashStringPtr parameter, boolean quoted = true);
  boolean HTTP_para_end(boolean quoted = true);
  boolean HTTP_para(FONAFlashStringPtr parameter, const char *value);
  boolean HTTP_para(FONAFlashStringPtr parameter, FONAFlashStringPtr value);
  boolean HTTP_para(FONAFlashStringPtr parameter, int32_t value);
  boolean HTTP_data(uint32_t size, uint32_t maxTime=10000);
  boolean HTTP_action(uint8_t method, uint16_t *status, uint16_t *datalen, int32_t timeout = 10000);
  boolean HTTP_readall(uint16_t *datalen);
  boolean HTTP_ssl(boolean onoff);

  // HTTP high level interface (easier to use, less flexible).
  boolean HTTP_GET_start(char *url, uint16_t *status, uint16_t *datalen);
  void HTTP_GET_end(void);
  boolean HTTP_POST_start(char *url, FONAFlashStringPtr contenttype, const uint8_t *postdata, uint16_t postdatalen,  uint16_t *status, uint16_t *datalen);
  void HTTP_POST_end(void);
  void setUserAgent(FONAFlashStringPtr useragent);

  // HTTPS
  void setHTTPSRedirect(boolean onoff);

  // PWM (buzzer)
  boolean setPWM(uint16_t period, uint8_t duty = 50);

  // Phone calls
  boolean callPhone(char *phonenum);
  uint8_t getCallStatus(void);
  boolean hangUp(void);
  boolean pickUp(void);
  boolean callerIdNotification(boolean enable, uint8_t interrupt = 0);
  boolean incomingCallNumber(char* phonenum);

  // Helper functions to verify responses.
  boolean expectReply(FONAFlashStringPtr reply, uint16_t timeout = 10000);
  boolean sendCheckReply(const char *send, const char *reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(FONAFlashStringPtr send, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(const char* send, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);


 protected:
  int8_t _rstpin;
  uint8_t _type;

  char replybuffer[255];
  FONAFlashStringPtr apn;
  FONAFlashStringPtr apnusername;
  FONAFlashStringPtr apnpassword;
  boolean httpsredirect;
  FONAFlashStringPtr useragent;
  FONAFlashStringPtr ok_reply;

  // HTTP helpers
  boolean HTTP_setup(char *url);

  void flushInput();
  uint16_t readRaw(uint16_t b);
  uint8_t readline(uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS, boolean multiline = false);
  uint8_t getReply(const char *send, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr send, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr prefix, char *suffix, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr prefix, int32_t suffix, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  uint8_t getReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, uint16_t timeout); // Don't set default value or else function call is ambiguous.
  uint8_t getReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);

  boolean sendCheckReply(FONAFlashStringPtr prefix, char *suffix, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, int32_t suffix2, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);
  boolean sendCheckReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, FONAFlashStringPtr reply, uint16_t timeout = FONA_DEFAULT_TIMEOUT_MS);

  void mqtt_connect_message(const char *protocol, byte *mqtt_message, const char *client_id, const char *username, const char *password);
  void mqtt_publish_message(byte *mqtt_message, const char *topic, const char *message);
  void mqtt_subscribe_message(byte *mqtt_message, const char *topic, byte QoS);
  void mqtt_disconnect_message(byte *mqtt_message);
  boolean mqtt_sendPacket(byte *packet, byte len);


  boolean parseReply(FONAFlashStringPtr toreply,
          uint16_t *v, char divider  = ',', uint8_t index=0);
  boolean parseReplyFloat(FONAFlashStringPtr toreply,
           float *f, char divider, uint8_t index);
  boolean parseReply(FONAFlashStringPtr toreply,
          char *v, char divider  = ',', uint8_t index=0);
  boolean parseReplyQuoted(FONAFlashStringPtr toreply,
          char *v, int maxlen, char divider, uint8_t index);

  boolean sendParseReply(FONAFlashStringPtr tosend,
       FONAFlashStringPtr toreply,
       uint16_t *v, char divider = ',', uint8_t index=0);

  boolean sendParseReplyFloat(FONAFlashStringPtr tosend,
         FONAFlashStringPtr toreply,
         float *f, char divider = ',', uint8_t index=0);

  static boolean _incomingCall;
  static void onIncomingCall();

  FONAStreamType *mySerial;
};

class Adafruit_FONA_3G : public Adafruit_FONA {

 public:
  Adafruit_FONA_3G (int8_t r) : Adafruit_FONA(r) { _type = SIM5320A; }

    boolean getBattVoltage(uint16_t *v);
    boolean powerDown(void);
    boolean playToolkitTone(uint8_t t, uint16_t len);
    boolean hangUp(void);
    boolean pickUp(void);
    // boolean enableGPRS(boolean onoff);
    // boolean enableGPS(boolean onoff);
    // boolean postData3G(const char *server, uint16_t port, const char *connType, const char *URL);

 protected:
    boolean parseReply(FONAFlashStringPtr toreply,
           float *f, char divider, uint8_t index);

    boolean sendParseReply(FONAFlashStringPtr tosend,
         FONAFlashStringPtr toreply,
         float *f, char divider = ',', uint8_t index=0);
};

class Adafruit_FONA_LTE : public Adafruit_FONA {

 public:
  Adafruit_FONA_LTE () : Adafruit_FONA(FONA_NO_RST_PIN) { _type = SIM7000; _type = SIM7070; _type = SIM7500; }

  boolean setPreferredMode(uint8_t mode);
  boolean setPreferredLTEMode(uint8_t mode);
  boolean setOperatingBand(const char * mode, uint8_t band);
  boolean setBaudrate(uint32_t baud);
  boolean hangUp(void);

  // MQTT
  boolean MQTT_setParameter(const char* paramTag, const char* paramValue, uint16_t port = 0);
  boolean MQTT_connect(bool yesno);
  boolean MQTT_connectionStatus(void);
  boolean MQTT_subscribe(const char* topic, byte QoS);
  boolean MQTT_unsubscribe(const char* topic);
  boolean MQTT_publish(const char* topic, const char* message, uint16_t contentLength, byte QoS, byte retain);
  boolean MQTT_dataFormatHex(bool yesno);

  // HTTP
  boolean HTTP_connect(const char *server);
  boolean HTTP_addHeader(const char *type, const char *value, uint16_t maxlen); // max length of value
  boolean HTTP_addPara(const char *key, const char *value, uint16_t maxlen); // max length of value
  boolean HTTP_GET(const char *URI);
  boolean HTTP_POST(const char *URI, const char *body, uint8_t bodylen);
};

#endif