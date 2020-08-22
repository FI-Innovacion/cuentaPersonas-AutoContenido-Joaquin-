

#include "InfluxEEPROM.h"

char* loadstr (size_t addr, size_t size){
    
    char* ret = (char*) malloc (size);
    
    for (size_t i = 0; i < size; ++i){
        ret[i] = EEPROM.read(addr + i);
        if (ret[i] == 0) break;
	}
    
    ret[size-1] = 0;
    return ret;
    
}

void savestr (size_t addr, const char* str, size_t size){
    
    for (size_t i = 0; i <= size; ++i)
        EEPROM.write(addr + i, str[i]);
    EEPROM.commit();
    
}

char* loadSSID(){
    
    return loadstr (SSIDADDR, SSIDSIZE);
    
}

char* loadPASS(){
    
    return loadstr (PASSADDR, PASSSIZE);
    
}

void saveSSID(const char* ssid){
    
    savestr (SSIDADDR, ssid, strlen(ssid));
    
}

void savePASS(const char* pass){
    
    savestr (SSIDADDR, pass, strlen(pass));
    
}

