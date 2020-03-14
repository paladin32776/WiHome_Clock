#define SERIAL_DEBUG true

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

#define CCS811_ADDR 0x5A // CCS811 default I2C address
