#include "WiHomeComm.h"
#include "NoBounceButtons.h"
#include "SignalLED.h"
#include "WiHome_Config.h"
#include "EnoughTimePassed.h"

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "TimeLib.h"

#include "HDC1080.h"
#include "CCS811.h"
#include "BH1750.h"

WiHomeComm whc(false); // argument turns WiHome UDP communication with WiHome hub off

SignalLED led(PIN_LED,SLED_BLINK_FAST_1,PIN_LED_ACTIVE_LOW);
NoBounceButtons nbb;
char button;
bool led_status=false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
time_t t, t_last;

// Object for display:
MultiDisp7 m7(display_count, addr, type, subdigit);

// Objects and variables for sensors:
HDC1080 hdc;
float T,RH;
CCS811 ccs;
unsigned int CO2, TVOC;
BH1750 bh;
unsigned int LX, LX_dim;


void setup()
{
  if (SERIAL_DEBUG)
    Serial.begin(115200);
  else
    Serial.end();
  Serial.println();
  delay(100);
  button = nbb.create(PIN_BUTTON);

  m7.dim(100);
  m7.print("                    ");
  m7.print("boot");

  timeClient.begin();
  timeClient.setTimeOffset(-5*3600); // Easter Timezone in the U.S.: GMT-5h
  t_last = timeClient.getEpochTime();

}


void loop()
{
  // Handling routines for various libraries used:
  whc.check();
  nbb.check();
  led.check();

  if (whc.status()==1)
    timeClient.update();
  t = timeClient.getEpochTime();

  if (t!=t_last)
  {
    t_last = t;
    LX = bh.read();
    LX_dim = LX*0.1+1;
    if (LX_dim>255)
      LX_dim=255;
    m7.dim(LX_dim);

    hdc.read(&T, &RH);
    Serial.printf("LX = %d  Lx_dim = %d\n", LX, LX_dim);
    // ccs.set_env_data(T, RH);
    ccs.read(&CO2, &TVOC);

    char s[20];
    if (whc.status()==1)
    {
      if (int(second(t)/5) % 2)
        sprintf(s, "%02d%02d%02d%02d.%02d.%02d%2.0f%cC%2.0frH",
                hour(t), minute(t), second(t), day(t), month(t), year(t)-2000,
                T, 176, RH);
      else
        sprintf(s, "%02d%02d%02d%02d.%02d.%02d%4d%4d",
                hour(t), minute(t), second(t), day(t), month(t), year(t)-2000,
                CO2, TVOC);
    }
    else
      sprintf(s, "%02d%02d%02d%02d.%02d.%02d  noLAn ",
              hour(t), minute(t), second(t), day(t), month(t), year(t)-2000);
    Serial.println(s);
    m7.print(s);
  }

  // Logic for LED status display:
  if (whc.status()==1)
    led.set(led_status?SLED_ON:SLED_OFF);
  else if (whc.status()==2)
    led.set(SLED_BLINK_FAST_3);
  else if (whc.status()==3)
    led.set(SLED_BLINK_FAST_1);
  else if (whc.status()==4)
    led.set(SLED_BLINK_SLOW);
  else
    led.set(SLED_BLINK_FAST);

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
    Serial.printf("button action 1\nled_status=%d ==> ",led_status);
    if (whc.softAPmode==true)
      whc.softAPmode=false;
    else
      led_status = !led_status;
    Serial.printf("%d\n",led_status);
    nbb.reset(button);
  }
}
