

#ifndef InfluxEEPROM_h
#define InfluxEEPROM_h

#include <stdlib.h>
#include <EEPROM.h>
#include <string.h>

#define SSIDSIZE 64
#define PASSSIZE 64

#define SSIDADDR 0
#define PASSADDR 64

char* loadstr (size_t addr, size_t size);
void savestr (size_t addr, const char* str, size_t size);

char* loadSSID();
char* loadPASS();
void saveSSID(const char* ssid);
void savePASS(const char* pass);

#endif

