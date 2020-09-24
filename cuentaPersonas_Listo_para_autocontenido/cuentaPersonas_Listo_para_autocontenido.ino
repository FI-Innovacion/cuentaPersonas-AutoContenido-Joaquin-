#include <DNSServer.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <string.h>

#define INTDELAY 30
#define RDELAY 100

#define APNAME "Configurar_Sensores"
#define APPASS "ESP2866"

const char* fingerpr = "ED:BB:DB:12:54:FD:E1:A4:10:79:BC:CF:FE:06:42:63:80:2E:17:CE";
int httpCode         = 0;

char influx_host[128];
char influx_db[32];
char influx_user[32];
char influx_pass[32];
char device_name[32];
char sensor_name[32];

char request_url[160];
bool newData = false;

int loopCount   = 0;
int Trigger_der = 12;
int Echo_der    = D3;
int Trigger_izq = 5;
int Echo_izq    = 4;
int detectado   = 1;

int derecho              = 0;
int derecho_max          = 0;
int derecho_dist_disparo = 0;

int izquierdo              = 0;
int izquierdo_max          = 0;
int izquierdo_dist_disparo = 0;

int tiempo = 0;
int izquierdo_duracion, izquierdo_distancia;
int derecho_duracion, derecho_distancia;
int inactivo = 0;
int cuenta   = 0;

enum EstadoPulsador {
  NONE,
  ENTRANDO,
  MEDIO_ENTRANDO,
  SALIENDO,
  MEDIO_SALIENDO,
  CASI_ENTRANDO,
  CASI_SALIENDO,
} pulsadores;

void SaveDataCallback() {
  newData = true;
}

bool prefix(const char* pre, const char* str) {
  return strncmp(pre, str, strlen(pre)) == 0;
}

int influx_send_info(int info) {
  if (WiFi.status() != WL_CONNECTED) return 0;

  char postData[128];
  switch (info) {
    case 0:
      sprintf(postData, "Personas,accion=inicio_dispositivo,device=%s,sensor=%s value=0", device_name, sensor_name);
      break;
    case 1:
      sprintf(postData, "Personas,accion=entrar,device=%s,sensor=%s value=1", device_name, sensor_name);
      break;
    case -1:
      sprintf(postData, "Personas,accion=salir,device=%s,sensor=%s value=-1", device_name, sensor_name);
      break;
  }

  HTTPClient http;
  if (prefix("https://", influx_host)) {  //Si es https
    http.begin(request_url, fingerpr);
  } else {
    http.begin(request_url);
  }

  http.addHeader("Content-Type", "application/json");
  httpCode = http.POST(postData);
  http.end();
  Serial.print("Server returned code: ");
  Serial.println(httpCode);
  return (httpCode >= 200 && httpCode < 300);
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

  if (newData) {
    strcpy(influx_host, param_host.getValue());
    strcpy(influx_db, param_db.getValue());
    strcpy(influx_user, param_user.getValue());
    strcpy(influx_pass, param_pass.getValue());
    strcpy(device_name, param_device_name.getValue());
    strcpy(sensor_name, param_sensor_name.getValue());
    savestr(128, influx_host, 128);
    savestr(160, influx_db, 32);
    savestr(192, influx_user, 32);
    savestr(224, influx_pass, 32);
    savestr(256, device_name, 32);
    savestr(288, sensor_name, 32);
  } else {
    loadstr(128, influx_host, 128);
    loadstr(160, influx_db, 32);
    loadstr(192, influx_user, 32);
    loadstr(224, influx_pass, 32);
    loadstr(256, device_name, 32);
    loadstr(288, sensor_name, 32);
  }

  sprintf(request_url, "%s/write?db=%s&u=%s&p=%s", influx_host, influx_db, influx_user, influx_pass);

  for (int i = 0; i <= 50; i++) {
    Serial.print("Actual:");

    digitalWrite(Trigger_izq, LOW);
    delayMicroseconds(2);
    digitalWrite(Trigger_izq, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger_izq, LOW);
    izquierdo_duracion  = pulseIn(Echo_izq, HIGH);
    izquierdo_distancia = (izquierdo_duracion / 2) / 29.1;

    if (izquierdo_distancia > izquierdo_max || izquierdo_distancia < 3000) {
      izquierdo_max = izquierdo_distancia;
    }

    Serial.print(izquierdo_distancia);
    Serial.print(",");

    digitalWrite(Trigger_izq, LOW);
    delayMicroseconds(2);
    digitalWrite(Trigger_izq, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trigger_izq, LOW);
    derecho_duracion  = pulseIn(Echo_der, HIGH);
    derecho_distancia = (derecho_duracion / 2) / 29.1;

    if (derecho_distancia > derecho_max || derecho_distancia < 3000) {
      derecho_max = derecho_distancia;
    }

    Serial.print(derecho_distancia);
    Serial.println(".");

    delay(100);
  }

  derecho_dist_disparo   = derecho_max * 0.8;
  izquierdo_dist_disparo = izquierdo_max * 0.8;

  attachInterrupt(digitalPinToInterrupt(Echo_izq), echo_int_fall, FALLING);  //Se vincula la funcion a la subida de señal del pin de interrupcion.
  attachInterrupt(digitalPinToInterrupt(Echo_der), echo_2_int_fall, FALLING);  //Se vincula la funcion a la subida de señal del pin de interrupcion.

  Serial.print("Fin de calibracion. Distancia de disparo: ");
  Serial.print(izquierdo_dist_disparo);
  Serial.print(" y ");
  Serial.println(derecho_dist_disparo);
  Serial.println();
  Serial.println("Configurando conexion a base de datos...");
  Serial.print("Datos ingresados: ");
  Serial.print("Host: ");
  Serial.println(influx_host);
  Serial.print("DB: ");
  Serial.println(influx_db);
  Serial.print("User: ");
  Serial.println(influx_user);
  Serial.print("Password: ");
  Serial.println(influx_pass);

  if (!influx_send_info(0)) {  //Si no recibo un codigo success del server, limpiar datos de conexion y reconfigurar en el siguiente reinicio.
    Serial.println("Error de configuracion, limpiado configuracion y reiniciando..");
    Serial.print("Server returned code: ");
    Serial.println(httpCode);
    WiFi.disconnect(true);
    delay(2000);
    Serial.println("Configuracion actualizada, reiniciando...");
    ESP.reset();
  } else {
    Serial.println("Configuracion y conexion correctas, el sensor comenzará a enviar datos a la base de datos.");
  }
}

ICACHE_RAM_ATTR void echo_int_fall() {
  izquierdo_duracion = micros() - tiempo;
  //izqInt=0;
}

ICACHE_RAM_ATTR void echo_2_int_fall() {
  derecho_duracion = micros() - tiempo;
  //derInt=0;
}

void contar(int info) {
  inactivo   = 0;
  pulsadores = NONE;
  cuenta += info;
  Serial.print("Cuenta: ");
  Serial.println(cuenta);
  //Aca tendria que haber un delay para no contar demas, pero como enviar a
  //influx tarda al menos un segundo, eso será el delay
  influx_send_info(info);
}

void loop() {
  digitalWrite(Trigger_izq, HIGH);
  delayMicroseconds(2);
  digitalWrite(Trigger_izq, LOW);
  tiempo = micros();
  delayMicroseconds(20);
  izquierdo_distancia = (izquierdo_duracion / 2) / 29.1;
  derecho_distancia   = (derecho_duracion / 2) / 29.1;
  delay(10);

  if ((izquierdo_distancia < izquierdo_dist_disparo) || (izquierdo_distancia > 3000)) {  // Se esta a menos de cierta distancia activa izquierdo.
    izquierdo = HIGH;
  } else {
    izquierdo = LOW;
  }
  if ((derecho_distancia < derecho_dist_disparo) || (derecho_distancia > 3000)) {  // Se esta a menos de cierta distancia activa derecho.
    derecho = HIGH;
  } else {
    derecho = LOW;
  }

  switch (pulsadores) {
    case NONE:
      if (izquierdo == HIGH && derecho == LOW) {
        pulsadores = ENTRANDO;
        inactivo   = 0;
      } else if (izquierdo == LOW && derecho == HIGH) {
        pulsadores = SALIENDO;
        inactivo   = 0;
      }
      break;
    case ENTRANDO:
      if (izquierdo == HIGH && derecho == HIGH) pulsadores = MEDIO_ENTRANDO;
      break;
    case MEDIO_ENTRANDO:
      if (izquierdo == LOW && derecho == HIGH) pulsadores = CASI_ENTRANDO;
      break;
    case CASI_ENTRANDO:
      if (izquierdo == LOW && derecho == LOW) contar(1);
      break;
    case SALIENDO:
      if (izquierdo == HIGH && derecho == HIGH) pulsadores = MEDIO_SALIENDO;
      break;
    case MEDIO_SALIENDO:
      if (izquierdo == HIGH && derecho == LOW) pulsadores = CASI_SALIENDO;
      break;
    case CASI_SALIENDO:
      if (izquierdo == LOW && derecho == LOW) contar(-1);
      break;
  }

  ++inactivo;
  if (inactivo > 150) {  //probablemente haya que subir de 150 a 200
    inactivo   = 0;
    pulsadores = NONE;
  }
}
