#include "WiHomeComm.h"
#include "NoBounceButtons.h"
#include "SignalLED.h"
#include "WiHome_Config.h"
#include "EnoughTimePassed.h"

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "TimeLib.h"

// #include "MultiDisp7.h"

WiHomeComm whc;

SignalLED led(PIN_LED,SLED_BLINK_FAST_1,PIN_LED_ACTIVE_LOW);
NoBounceButtons nbb;
char button;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
EnoughTimePassed etp_time(1000);

MultiDisp7 m7(display_count, addr, type, subdigit);

void setup()
{
  if (SERIAL_DEBUG)
    Serial.begin(115200);
  else
    Serial.end();
  Serial.println();
  delay(100);
  button = nbb.create(PIN_BUTTON);

  timeClient.begin();
  timeClient.setTimeOffset(-5*3600); // Easter Timezone in the U.S.: GMT-5h
}


void loop()
{
  DynamicJsonBuffer jsonBuffer;
  // Handling routines for various libraries used:
  JsonObject& root = whc.check(&jsonBuffer);
  nbb.check();
  led.check();

  timeClient.update();
  if (etp_time.enough_time())
  {
    Serial.println(timeClient.getFormattedTime());
    char s[20];
    sprintf(s, "123456");
    m7.print(s);
  }

  // Logic for LED status display:
  if (whc.status()==1)
    led.set(SLED_OFF);
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
    if (whc.softAPmode==true)
      whc.softAPmode=false;
    nbb.reset(button);
  }
}
