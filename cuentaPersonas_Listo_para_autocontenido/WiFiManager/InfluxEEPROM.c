

#include "InfluxEEPROM.h"

char* loadstr (size_t addr, size_t size){
    
    char* ret = (char*) malloc (size);
    char* limit = ret + size;
    
    for (char* p = ret; p != limit && *p; ++p, ++addr)
        *p = EEPROM.read(addr);
    
    ret[size-1] = 0;
    return ret;
    
}

void savestr (size_t addr, char* str, size_t size){
    
    char* limit = addr + size;
    
    for (; str < limit; ++str, ++addr)
        EEPROM.write(addr, *str);
    EEPROM.commit();
    
}

char* loadSSID(){
    
    return loadstr (SSIDADDR, SSIDSIZE);
    
}

char* loadPASS(){
    
    return loadstr (PASSADDR, PASSSIZE);
    
}

void saveSSID(char* ssid){
    
    savestr (SSIDADDR, ssid, strlen(ssid));
    
}

void savePASS(char* pass){
    
    savestr (SSIDADDR, pass, strlen(pass));
    
}

