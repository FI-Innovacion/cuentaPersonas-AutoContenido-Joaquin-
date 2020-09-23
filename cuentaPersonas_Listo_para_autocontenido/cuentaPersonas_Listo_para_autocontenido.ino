#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h> 
#include<string.h>

#define INTDELAY 30
#define RDELAY 100

#define APNAME "Configurar_Sensores"
#define APPASS "ESP2866"

//const char* fingerpr = "ED BB DB 12 54 FD E1 A4 10 79 BC CF FE 06 42 63 80 2E 17 CE"; //Codigo SSL actualizado para acceder por https si hace falta.
const char* fingerpr = "ED:BB:DB:12:54:FD:E1:A4:10:79:BC:CF:FE:06:42:63:80:2E:17:CE";
int httpCode = 0;
char influx_host [128];

char influx_db [32];
char influx_user [32];
char influx_pass [32];
char device_name [32];
char sensor_name [32];

const byte pinExt = D5;
const byte pinInt = D7;
volatile bool intExt = false;
volatile bool intInt = false;
bool exterior = 0;
bool interior = 0;
int loopCount = 0;
int dtimeExt = 0;
int dtimeInt = 0;

int Trigger_der = 12;
int Echo_der = D3;
int Trigger_izq = 5;
int Echo_izq = 4;
int detectado = 1;

int derecho = 0;
int derecho_max = 0;
int derecho_dist_disparo = 0;

int izquierdo = 0;;
int izquierdo_max = 0;
int izquierdo_dist_disparo = 0;

int tiempo = 0;
int izqInt = 1;
int derInt = 1;
int izquierdo_duracion,izquierdo_distancia;
int derecho_duracion, derecho_distancia;
int inactivo = 0;
int cuenta = 0;
String pulsadores = "";

bool newData = false;


void SaveDataCallback () {
  newData = true;
}

ICACHE_RAM_ATTR void pinIntExt() {
  intExt = true;
}

ICACHE_RAM_ATTR void pinIntInt() {
  intInt = true;
}

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}


void ResetState () {
  Serial.print("Reset");
  intExt = false;
  intInt = false;
  exterior = false;
  interior = false;
  loopCount = 0;
}



bool IntStateInt () {
  if (intInt) {
    intInt = false;
    if (dtimeInt == 0) {
      dtimeInt = INTDELAY;
      return true;
    }
  }
  return false;
}

bool IntStateExt () {
  if (intExt) {
    intExt = false;
    if (dtimeExt == 0) {
      dtimeExt = INTDELAY;
      return true;
    }
  }
  return false;
}

void setup() {
  EEPROM.begin(512);
  pinMode(Trigger_der, OUTPUT);
  pinMode(Echo_der, INPUT); 
  pinMode(Trigger_izq, OUTPUT);
  pinMode(Echo_der, INPUT);
  Serial.begin(115200);


  WiFiManager wifiManager;
  
  WiFiManagerParameter param_host("influx_host", "influx host", "", 128);
  WiFiManagerParameter param_db("influx_db", "influx database", "", 32);
  WiFiManagerParameter param_user("influx_user", "influx user", "", 32);
  WiFiManagerParameter param_pass("influx_pass", "influx pass", "", 32);
  WiFiManagerParameter param_device_name("sender_device_name", "sender device name", "", 32);
  WiFiManagerParameter param_sensor_name("sender_sensor_name", "sender sensor name", "", 32);
  wifiManager.addParameter(&param_host);
  wifiManager.addParameter(&param_db);
  wifiManager.addParameter(&param_user);
  wifiManager.addParameter(&param_pass);
  wifiManager.addParameter(&param_device_name);
  wifiManager.addParameter(&param_sensor_name);

  wifiManager.setSaveConfigCallback(SaveDataCallback);
  
  wifiManager.autoConnect(APNAME, APPASS);

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  if (newData){
    
    strcpy (influx_host, param_host.getValue());
    strcpy (influx_db, param_db.getValue());
    strcpy (influx_user, param_user.getValue());
    strcpy (influx_pass, param_pass.getValue());
    strcpy (device_name, param_device_name.getValue());
    strcpy (sensor_name, param_sensor_name.getValue());
    
    savestr (128, influx_host, 128);
    savestr (160, influx_db, 32);
    savestr (192, influx_user, 32);
    savestr (224, influx_pass, 32);
    savestr (256, device_name, 32);
    savestr (288, sensor_name, 32);
      
  } else {
    
    char* temp;

    temp = loadstr(128, 128);
    strcpy (influx_host, temp);
    free(temp);


    temp = loadstr(160, 32);
    strcpy (influx_db, temp);
    free(temp);

    temp = loadstr(192, 32);
    strcpy (influx_user, temp);
    free(temp);

    temp = loadstr(224, 32);
    strcpy (influx_pass, temp);
    free(temp);

    temp = loadstr(256, 32);
    strcpy (device_name, temp);
    free(temp);

    temp = loadstr(288, 32);
    strcpy (sensor_name, temp);
    free(temp);
    
  }

 

  pinMode(pinExt, INPUT_PULLUP);
  pinMode(pinInt, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinExt), pinIntExt, RISING);
  attachInterrupt(digitalPinToInterrupt(pinInt), pinIntInt, RISING);

  

  for (int i=0;i<=50;i++) {
    Serial.print("Actual:");

    digitalWrite (Trigger_izq, LOW);
    delayMicroseconds(2);
    digitalWrite (Trigger_izq, HIGH);
    delayMicroseconds (10);
    digitalWrite (Trigger_izq, LOW);
    izquierdo_duracion = pulseIn (Echo_izq, HIGH);
    izquierdo_distancia = (izquierdo_duracion/2) / 29.1;

    if (izquierdo_distancia > izquierdo_max || izquierdo_distancia < 3000) {
      izquierdo_max=izquierdo_distancia;
      }
    Serial.print(izquierdo_distancia);Serial.print(",");
    
    digitalWrite (Trigger_izq, LOW);
    delayMicroseconds(2);
    digitalWrite (Trigger_izq, HIGH);
    delayMicroseconds (10);
    digitalWrite (Trigger_izq, LOW);
    derecho_duracion = pulseIn (Echo_der, HIGH);
    derecho_distancia = (derecho_duracion/2) / 29.1;
    
    if (derecho_distancia > derecho_max || derecho_distancia < 3000) {
      derecho_max=derecho_distancia;
      }

    Serial.print(derecho_distancia);Serial.println(".");
    
    
    
    delay(100);
    }

    derecho_dist_disparo = derecho_max*0.8;
    izquierdo_dist_disparo = izquierdo_max*0.8;

    
  attachInterrupt(digitalPinToInterrupt(Echo_izq), echo_int_fall, FALLING); //Se vincula la funcion a la subida de señal del pin de interrupcion.
  
  attachInterrupt(digitalPinToInterrupt(Echo_der), echo_2_int_fall, FALLING); //Se vincula la funcion a la subida de señal del pin de interrupcion.

  
  Serial.print("Fin de calibracion. Distancia de disparo: "); Serial.print(izquierdo_dist_disparo); Serial.print(" y "); Serial.println(derecho_dist_disparo);
  Serial.println(); 
  Serial.println("Configurando conexion a base de datos...");
  Serial.print("Datos ingresados: ");
  Serial.print("Host: ");Serial.println(influx_host);
  Serial.print("DB: ");Serial.println(influx_db);
  Serial.print("User: ");Serial.println(influx_user);
  Serial.print("Password: ");Serial.println(influx_pass);

  if (!influx_send_startup_info()){//Si no recibo un codigo success del server, limpiar datos de conexion y reconfigurar en el siguiente reinicio.
    Serial.println("Error de configuracion, limpiado configuracion y reiniciando..");
    Serial.print("Server returned code: ");Serial.println(httpCode);
    WiFi.disconnect(true);
    delay(2000);
    Serial.println("Configuracion actualizada, reiniciando...");
    ESP.reset();
  }else{
     Serial.println("Configuracion y conexion correctas, el sensor comenzará a enviar datos a la base de datos.");
    }

}


int influx_send_startup_info(){
  if(WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    char url[160];
    sprintf(url,"%s/write?db=%s&u=%s&p=%s",influx_host,influx_db,influx_user,influx_pass);
    char postData[128];
    sprintf(postData,"Personas,accion=inicio_dispositivo,device=%s,sensor=%s value=0",device_name,sensor_name);

   if (prefix("https://",influx_host)){ //Si es https
      http.begin(url, fingerpr);
    }else{
      http.begin(url);
     }
     
    http.addHeader("Content-Type","application/json");
   httpCode=http.POST(postData);
   http.end();
   Serial.print("Server returned code: ");Serial.println(httpCode);
  if (httpCode >= 200 && httpCode < 300){
    return true;
    }else{
     return false;
     }
  }else{
    return false;
  }
}

  

int influx_send_entra_persona(){
   if(WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    char url[160];
    sprintf(url,"%s/write?db=%s&u=%s&p=%s",influx_host,influx_db,influx_user,influx_pass);
    char postData[128];
    sprintf(postData,"Personas,accion=entrar,device=%s,sensor=%s value=1",device_name,sensor_name);

    if (prefix("https://",influx_host)){ //Si es https
      http.begin(url, fingerpr);
    }else{
      http.begin(url);
     }
    
    
    http.addHeader("Content-Type","application/json");
   httpCode=http.POST(postData);
   http.end();
   Serial.print("Server returned code: ");Serial.println(httpCode);
  if (httpCode >= 200 && httpCode < 300){
    return true;
    }else{
     return false;
     }
  }else{
    return false;
  }
}



int influx_send_sale_persona(){
  if(WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    char url[160];
    sprintf(url,"%s/write?db=%s&u=%s&p=%s",influx_host,influx_db,influx_user,influx_pass);
    char postData[128];
    sprintf(postData,"Personas,accion=salir,device=%s,sensor=%s value=-1",device_name,sensor_name);

    if (prefix("https://",influx_host)){ //Si es https
      http.begin(url, fingerpr);
    }else{
      http.begin(url);
     }
     
    http.addHeader("Content-Type","application/json");
    httpCode=http.POST(postData);
   http.end();
   Serial.print("Server returned code: ");Serial.println(httpCode);
  if (httpCode >= 200 && httpCode < 300){
    return true;
    }else{
     return false;
     }
  }else{
    return false;
  }
}
  
  
ICACHE_RAM_ATTR void echo_int_fall(){
  izquierdo_duracion=micros() - tiempo;
  //izqInt=0;
  }

  
ICACHE_RAM_ATTR void echo_2_int_fall(){
  derecho_duracion=micros() - tiempo;
  //derInt=0;
  }



void loop(){
  
    digitalWrite (Trigger_izq, HIGH);
    delayMicroseconds(2);
    digitalWrite (Trigger_izq, LOW);
    tiempo=micros();
    delayMicroseconds(20);
      izquierdo_distancia = (izquierdo_duracion/2) / 29.1;
      derecho_distancia = (derecho_duracion/2) / 29.1;
    delay(10);



    
    if ((izquierdo_distancia < izquierdo_dist_disparo) || (izquierdo_distancia > 3000)) { // Se esta a menos de cierta distancia activa izquierdo.
      izquierdo = HIGH;
    }
 else {
      izquierdo = LOW;
    }  
 if ((derecho_distancia < derecho_dist_disparo) || (derecho_distancia > 3000)) { // Se esta a menos de cierta distancia activa derecho.
     derecho = HIGH;
    }
 else {
      derecho = LOW;
    } 


////////////////////////////////////////////////////////////////
if (izquierdo == HIGH && derecho == LOW && pulsadores == "")
  {
  pulsadores="entrando";
  inactivo=0;
  }
  
if (izquierdo == LOW && derecho == HIGH && pulsadores == "")
    {
    pulsadores="saliendo";
    inactivo=0;
    }

if (izquierdo == HIGH && derecho == HIGH && pulsadores == "entrando")
  {
  pulsadores="entrando_medio";
  //inactivo=0;
  }    
if (izquierdo == LOW && derecho == HIGH && pulsadores == "entrando_medio")
  {
  pulsadores="casi_entro";
  //inactivo=0;
  }

if (izquierdo == HIGH && derecho == HIGH && pulsadores == "saliendo")
  {
  pulsadores="saliendo_medio";
  //inactivo=0;
  }    
if (izquierdo == HIGH && derecho == LOW && pulsadores == "saliendo_medio")
  {
  pulsadores="casi_salio";
  //inactivo=0;
  }



if (izquierdo == LOW && derecho == LOW && pulsadores == "casi_entro")
  {
    inactivo=0;
  pulsadores="";
  cuenta=cuenta+1;
  Serial.print("Cuenta uno mas: ");
  Serial.println(cuenta); 
  //Aca tendria que haber un delay para no contar demas, pero como enviar a influx tarda al menos un segundo, eso será el delay
  influx_send_entra_persona();
  }
if (izquierdo == LOW && derecho == LOW && pulsadores == "casi_salio")
  {
   inactivo=0;
  pulsadores="";
  cuenta=cuenta-1;
  Serial.print("Cuenta uno menos: ");
  Serial.println(cuenta); 
  //Aca tendria que haber un delay para no contar demas, pero como enviar a influx tarda al menos un segundo, eso será el delay
  influx_send_sale_persona();
  }
///////////////////////////////////////////////


inactivo++;
if (inactivo > 150){
  inactivo=0;
  pulsadores="";
  }
}
