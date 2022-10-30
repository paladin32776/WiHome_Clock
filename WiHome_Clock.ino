#include "WiHomeComm.h"
#include "NoBounceButtons.h"
#include "SignalLED.h"
#include "WiHome_Config.h"
#include "EnoughTimePassed.h"

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "TimeLib.h"

//#include "HDC1080.h"
//#include "CCS811.h"
#include <Wire.h>
#include "SparkFunBME280.h"
#include "BH1750.h"

#define TIMEZONE_OFFSET 1

WiHomeComm whc(false); // argument turns WiHome UDP communication with WiHome hub off


SignalLED led(PIN_LED,SLED_BLINK_FAST_1,PIN_LED_ACTIVE_LOW);
NoBounceButtons nbb;
char button;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
time_t t, t_last;

// Object for display:
MultiDisp7 m7(display_count, addr, type, subdigit);

// Objects and variables for sensors:
//HDC1080 hdc;
//float T,RH;
//CCS811 ccs;
//unsigned int CO2, TVOC;
BME280 bme;
float T,RH,P,TK,PN,PNN;
bool bme_ok = false;
BH1750 bh;
unsigned int LX, LX_dim;

bool timeset = false;
EnoughTimePassed etp_timeset(10*1000); // ms
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
  m7.print("                    ");
  m7.print("boot");
  // Set internal clock and timezone:
  timeClient.begin();
  // timeClient.setTimeOffset(2*3600); // Mitteleuropäische Sommerzeit
  t_last = timeClient.getEpochTime();
}

bool DST(time_t t)
{
  bool lastweekinMarch = ( month(t)==3 ) && ( day(t)+(8-weekday(t))>31 );
  // bool notlastweekinOctober = ( month(t)==10 ) && ( day(t)+(8-weekday(t))<31 );
  bool lastweekinOctober = ( month(t)==10 ) && ( day(t)+(8-weekday(t))>31 );

  bool isDST = ( month(t)>3 && month(t)<10 ) ||
               ( month(t)==3 && lastweekinMarch && weekday(t)>1 ) ||
               ( month(t)==3 && lastweekinMarch && weekday(t)==1 && hour(t)>=2 ) ||
               ( month(t)==10 && !lastweekinOctober ) ||
               ( month(t)==10 && lastweekinOctober && weekday(t)==1 && hour(t)<2 );

  return isDST;
}


void loop()
{
  char s[20];

  // Handling routines for various libraries used:
  whc.check();
  nbb.check();
  led.check();

  if (whc.status()==1 && (timeset == false || etp_timeset.enough_time()))
  {
    Serial.printf("Getting time from NTP server.\n");
    Serial.printf("BEFORE: %02d.%02d.%02d  %02d.%02d.%02d\n",
            hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
    // Update time from NTP server:
    timeset = timeClient.update();
    // Get time:
    t = timeClient.getEpochTime();
    Serial.printf("NTP:    %02d.%02d.%02d  %02d.%02d.%02d\n",
            hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
    // Accouting for timezone and DST
    t += TIMEZONE_OFFSET*3600;
    if (DST(t))
      t += 3600;
    Serial.printf("DST:    %02d.%02d.%02d  %02d.%02d.%02d\n",
            hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
  }
  else
  {
    // Get time:
    t = timeClient.getEpochTime();
    // Accouting for timezone and DST
    t += TIMEZONE_OFFSET*3600;
    if (DST(t))
      t += 3600;
  }
  if (abs(t-t_last)>3000)
    Serial.printf("ERROR\n");

  if (t!=t_last)
  {
    t_last = t;
    etp_blink.event();

    LX = bh.read();
    LX_dim = LX*0.4+1;//Pink 0.2  Blau 0.7 Grün 0.5 Hellgrün 0.2 Gelb 0.1 Orange 0.2 Rot 0.7
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
      TK = 273.15+T;
      PN = TK/(TK+0.0065*640);
      PNN = P*pow(PN,-5.255);
      // Serial.printf("T = %1.1f  RH = %2.1f\n  PNN = %4.0f", T, RH, PNN);
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
      sprintf(s, "%02d%02d%02d%02d.%02d.%02dconn    ",
              hour(t), minute(t), second(t), day(t), month(t),  year(t)%100);
    // Serial.println(s);
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
