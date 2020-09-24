#ifndef InfluxEEPROM_h
#define InfluxEEPROM_h

#define SSIDADDR 0
#define SSIDSIZE 64

#define PASSADDR 64
#define PASSSIZE 64

#include <stdlib.h>
#include <EEPROM.h>

void loadstr (size_t addr, char* str, size_t size);
void savestr (size_t addr, const char* str, size_t size);

void loadSSID(char* ssid);
void loadPASS(char* pass);
void saveSSID(const char* ssid);
void savePASS(const char* pass);

#endif

