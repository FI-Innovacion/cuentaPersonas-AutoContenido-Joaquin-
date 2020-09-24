#include "InfluxEEPROM.h"

void loadstr (size_t addr, char* str, size_t size){
    size_t lim = addr + size - 1;
    for (; addr < lim; ++addr, ++str) {
        *str = EEPROM.read(addr);
        if (*str == 0) break;
	}
    *str = 0;
}

void savestr (size_t addr, const char* str, size_t size){
    size_t lim = addr + size - 1;
    for (; addr < lim; ++addr, ++str) {
        EEPROM.write(addr, *str);
        if (*str == 0) break;
    }
    EEPROM.commit();
}

void loadSSID (char* ssid) {
    loadstr (SSIDADDR, ssid, SSIDSIZE);
}

void loadPASS (char* pass) {
    loadstr (PASSADDR, pass, PASSSIZE);
}

void saveSSID (const char* ssid) {
    savestr (SSIDADDR, ssid, SSIDSIZE);
}

void savePASS (const char* pass) {
    savestr (PASSADDR, pass, PASSSIZE);
}
