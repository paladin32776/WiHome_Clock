#define SERIAL_DEBUG true

// // Pin config Gosund wall switch V1
// #define PIN_LED     13
// #define PIN_BUTTON  0
// #define PIN_RELAY   12
// #define PIN_LED_ACTIVE_LOW true
// #define PIN_RELAY_ACTIVE_LOW false

// // Pin config Gosund wall switch SW1_V1.2
// // #define PIN_LED     16 // RED
// #define PIN_LED     1 // GREEN but also Serial TX
// #define PIN_BUTTON  0
// #define PIN_RELAY   14
// #define PIN_LED_ACTIVE_LOW true
// #define PIN_RELAY_ACTIVE_LOW false

// // Pin config Sonoff Basic RF R2 POWER V1.0
// #define PIN_LED     13
// #define PIN_BUTTON  0
// #define PIN_RELAY   12
// #define PIN_LED_ACTIVE_LOW true
// #define PIN_RELAY_ACTIVE_LOW false

// // Pin config Sonoff Touch EU WIFI V1.0 (2016-8-3)
// #define PIN_LED     13
// #define PIN_BUTTON  0
// #define PIN_RELAY   12
// #define PIN_LED_ACTIVE_LOW true
// #define PIN_RELAY_ACTIVE_LOW false

// // Pin config Sonoff Basic R2 V1.0 (2017-10-11)
// #define PIN_LED     13
// #define PIN_BUTTON  0
// #define PIN_RELAY   12
// #define PIN_LED_ACTIVE_LOW true
// #define PIN_RELAY_ACTIVE_LOW false

// // Pin config Sonoff POW Ver 2.0
// #define PIN_LED     15
// #define PIN_BUTTON  0
// #define PIN_RELAY   12
// #define PIN_LED_ACTIVE_LOW false
// #define PIN_RELAY_ACTIVE_LOW false

// Pin config WiHome_Clock
#define PIN_LED     13
#define PIN_BUTTON  0
#define PIN_LED_ACTIVE_LOW false

#include "MultiDisp7.h"

unsigned const char display_count = 20;
unsigned const char addr[] = {96,96,97,97,98,98,99,99,100,100,101,101,102,102,103,103,105,105,106,106};
unsigned const char type[] = {//Time HH MM ss:
                              MultiDisp7::LARGE,MultiDisp7::LARGE,MultiDisp7::LARGE,MultiDisp7::LARGE,
                              MultiDisp7::SMALL,MultiDisp7::SMALL,
                              // Date dd mm yy:
                              MultiDisp7::SMALL,MultiDisp7::SMALL,MultiDisp7::SMALL,MultiDisp7::SMALL,
                              MultiDisp7::SMALL,MultiDisp7::SMALL,
                              // First 4-digit sensor data:
                              MultiDisp7::SMALL,MultiDisp7::SMALL,MultiDisp7::SMALL,MultiDisp7::SMALL,
                              // Second 4-digit sensor data:
                              MultiDisp7::SMALL,MultiDisp7::SMALL,MultiDisp7::SMALL,MultiDisp7::SMALL
                             };
unsigned const char subdigit[] = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
