// Board: "NodeMCU 1.0 (ESP-12E Module)"
// Flash Size: "4MB (FS:1MB OTA:1019KB)"

//Letztes Update 1.7.2021

#include "WiHomeComm.h"
#include "NoBounceButtons.h"
#include "SignalLED.h"
#include "WiHome_Config.h"
#include "EnoughTimePassed.h"

// #include <NTPClient.h>
#include <WiFiUdp.h>
#include "TimeLib.h"
#include <Wire.h>
#include "SparkFunBME280.h"
#include "BH1750.h"

#define TIMEZONE_OFFSET 1

WiHomeComm whc(false); // argument turns WiHome UDP communication with WiHome hub off


SignalLED led(PIN_LED,SLED_BLINK_FAST_1,PIN_LED_ACTIVE_LOW);
NoBounceButtons nbb;
char button;

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
static const char ntpServerName[] = "at.pool.ntp.org";
// NTPClient timeClient(ntpUDP);
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
time_t t, t_last;

// Object for display:
MultiDisp7 m7(display_count, addr, type, subdigit);

// Objects and variables for sensors:
BME280 bme;
float T,RH,P,TK,PN,PNN;
bool bme_ok = false;
BH1750 bh;
unsigned int LX, LX_dim;

bool timeset = false;
EnoughTimePassed etp_timeset(300*1000); // ms
EnoughTimePassed etp_sensors(15*1000); // ms
EnoughTimePassed etp_blink(500); // ms
bool blink_on = false;

void setup()
{
  // Setting up serial port if SERIAL_DEBUG flag is set to true:
  if (SERIAL_DEBUG)
    Serial.begin(115200);
  else
    Serial.end();
  for (int n=0; n<100; n++)
  {
    Serial.print(".");
    delay(20);
  }
  Serial.println();
  delay(100);
  // Initialize BME280 sensor:
  Wire.begin(14,2);
  bme.setI2CAddress(0x76);
  if (bme.beginI2C() == true) //Begin communication over I2C
    bme_ok = true;
  else
    Serial.println("The sensor did not respond. Please check wiring.");
  // Telling WiHomeComm library which led (library SignaleLED) to use as status led:
  whc.set_status_led(&led);
  // Creating debounced button input on pin PIN_BUTTON:
  button = nbb.create(PIN_BUTTON);
  // Set 7-Segment display to 100% brightness, overwire all digits with spaces, and then write "boot":
  m7.dim(100);
  m7.print((char*)"                    ");
  m7.print((char*)"boot");
  
  // Setup NTP callback and time interval for clock
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(5);
  // Set internal clock and timezone:
  // timeClient.begin();
  // timeClient.setTimeOffset(2*3600); // Mitteleuropäische Sommerzeit
  // t_last = timeClient.getEpochTime();
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + TIMEZONE_OFFSET * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

bool DST(time_t t)
{
  bool lastweekinMarch = ( month(t)==3 ) && ( day(t)+(8-weekday(t))>31 );
  bool notlastweekinOctober = ( month(t)==10 ) && ( day(t)+(8-weekday(t))<31 );

  bool isDST = ( month(t)>3 && month(t)<10 ) ||
               ( month(t)==3 && lastweekinMarch && weekday(t)>1 ) ||
               ( month(t)==3 && lastweekinMarch && weekday(t)==1 && hour(t)>=1 ) ||
               ( month(t)==10 && notlastweekinOctober ) ||
               ( month(t)==10 && (!notlastweekinOctober) && weekday(t)==1 && hour(t)<1 );

  return isDST;
}

void loop()
{
  char s[20];

  // Handling routines for various libraries used:
  whc.check();
  nbb.check();
  led.check();

  t = now();


  if (timeStatus()==timeSet)
    setSyncInterval(600);

  // Accounting DST
  if (DST(t))
    t += 3600;

  if (t!=t_last)
  {
    t_last = t;
    etp_blink.event();

    LX = bh.read();
    LX_dim = LX*0.5+1; //Hellgrün 0.1 Gelb 0.1 Pink 0.3 Orange 0.3 Dunkelgrün 0.5 Dunkelblau 0.7 Dunkelrot 0.7
    if (LX_dim>255)
      LX_dim=255;
    m7.dim(LX_dim);

    if (etp_sensors.enough_time())
    {
//      hdc.read(&T, &RH);
      if (bme_ok)
      {
        T = bme.readTempC();
        RH = bme.readFloatHumidity();
        P = bme.readFloatPressure()/100;
      }
      else
      {
        T = 0;
        RH = 0;
        P = 0;
      }
      // Math:
      T=T-2;
      TK = 273.15+T;
      PN = TK/(TK+0.0065*635 );
      PNN = P*pow(PN,-5.255);
      Serial.printf("T = %1.1f  RH = %2.1f\n  PNN = %4.0f", T, RH, PNN);
//      ccs.set_env_data(T, RH);
//      ccs.read(&CO2, &TVOC);
    }

    if (whc.status()==1)
    {
      if (bme_ok==true)
        sprintf(s, "%02d.%02d.%02d%02d.%02d.%02d%4.0f%2.0f%2.0f",
                hour(t), minute(t), second(t), day(t), month(t), year(t)%100, PNN, RH, T);
      else
        sprintf(s, "%02d.%02d.%02d%02d.%02d.%02d5En5FA1L",
                hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
      blink_on = true;
    }
    else
    {
      if (whc.softAPmode)
        sprintf(s, "%02d%02d%02d%02d.%02d.%02d5oFtAP  ",
                hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
      else
        sprintf(s, "%02d%02d%02d%02d.%02d.%02dconn    ",
                hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
    }
    Serial.println(s);
    m7.print(s);
  }
  if (etp_blink.enough_time() && blink_on && whc.status()==1)
  {
    sprintf(s, "%02d%02d",
                hour(t), minute(t));
    m7.print(s);
    blink_on = false;
  }

  // React to button actions:
  if (nbb.action(button)==2)
  {
    Serial.printf("Button1 pressed (action=2).\n");
    Serial.printf("Attempting to go to SoftAP mode.\n");
    whc.softAPmode=true;
    nbb.reset(button);
  }
  if (nbb.action(button)==1)
  {
    Serial.printf("button action 1\n");
    if (whc.softAPmode==true)
      whc.softAPmode=false;
    nbb.reset(button);
  }
}
