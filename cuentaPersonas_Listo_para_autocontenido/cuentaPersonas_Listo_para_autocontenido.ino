#include <ESP8266WiFi.h>
#include <InfluxDb.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define INTDELAY 30
#define RDELAY 100

#define APNAME "Configurar_Sensores"
#define APPASS "ESP2866"


 #define DISPOSITIVO "d1" //Nombre del dispositivo
 #define SENSOR_OBSTRUCCION "reflectivo"

char influx_host [128];
const unsigned short influx_port = 8086;

char influx_db [32];
char influx_user [32];
char influx_pass [32];

const byte pinExt = D5;
const byte pinInt = D7;
volatile bool intExt = false;
volatile bool intInt = false;
bool exterior = 0;
bool interior = 0;
int loopCount = 0;
int dtimeExt = 0;
int dtimeInt = 0;

int Trigger_der = 0;
int Echo_der = 2;
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

Influxdb* influx;

void SaveDataCallback () {
  newData = true;
}

void pinIntExt() {
  intExt = true;
}

void pinIntInt() {
  intInt = true;
}

void ResetState () {
  Serial.print("Reset");
  intExt = false;
  intInt = false;
  exterior = false;
  interior = false;
  loopCount = 0;
}

void SendInflux (int c) {
  Serial.print("Mandando... ");
  InfluxData row("ingreso");
  row.addValue("valor", c);
  influx->write(row);
  Serial.print("Enviado: ");
  Serial.println(c);
}

inline void SendReset (int c) {
  SendInflux(c);
  ResetState();
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
  wifiManager.addParameter(&param_host);
  wifiManager.addParameter(&param_db);
  wifiManager.addParameter(&param_user);
  wifiManager.addParameter(&param_pass);

  wifiManager.setSaveConfigCallback(SaveDataCallback);
  
  wifiManager.autoConnect(APNAME, APPASS);

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  if (newData){
    
    strcpy (influx_host, param_host.getValue());
    strcpy (influx_db, param_db.getValue());
    strcpy (influx_user, param_user.getValue());
    strcpy (influx_pass, param_pass.getValue());
    
    savestr (128, influx_host, 128);
    savestr (256, influx_db, 32);
    savestr (288, influx_user, 32);
    savestr (320, influx_pass, 32);
      
  } else {
    
    char* temp;

    temp = loadstr(128, 128);
    strcpy (influx_host, temp);
    free(temp);

    temp = loadstr(256, 32);
    strcpy (influx_db, temp);
    free(temp);

    temp = loadstr(288, 32);
    strcpy (influx_user, temp);
    free(temp);

    temp = loadstr(320, 32);
    strcpy (influx_pass, temp);
    free(temp);
    
  }

  influx = new Influxdb (influx_host, influx_port);
  influx->setDbAuth(influx_db, influx_user, influx_pass);

  pinMode(pinExt, INPUT_PULLUP);
  pinMode(pinInt, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(pinExt), pinIntExt, RISING);
  attachInterrupt(digitalPinToInterrupt(pinInt), pinIntInt, RISING);

  

  for (int i=0;i<=50;i++) {
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
    Serial.print("Actual:");Serial.print(izquierdo_distancia);Serial.print(",");Serial.print(derecho_distancia);Serial.println(".");
    
    delay(100);
    }

    derecho_dist_disparo = derecho_max*0.8;
    izquierdo_dist_disparo = izquierdo_max*0.8;

    
  attachInterrupt(digitalPinToInterrupt(Echo_izq), echo_int_fall, FALLING); //Se vincula la funcion a la subida de señal del pin de interrupcion.
  
  attachInterrupt(digitalPinToInterrupt(Echo_der), echo_2_int_fall, FALLING); //Se vincula la funcion a la subida de señal del pin de interrupcion.

  Serial.print("Fin de configuracion. Distancia de disparo: "); Serial.print(izquierdo_dist_disparo); Serial.print(" y "); Serial.println(derecho_dist_disparo);
  Serial.println(); 
  Serial.println("Configurando conexion a base de datos...");
  


}

void influx_send_entra_persona(){ //Se conto una persona.
  InfluxData measurement ("Personas");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addTag("accion", "entrar");
  measurement.addValue("value", 1);

  // write it into db
  influx->write(measurement);
  
  }

  
void influx_send_sale_persona(){ //Se conto una persona.
  InfluxData measurement ("Personas");
  measurement.addTag("device", DISPOSITIVO);
  measurement.addTag("sensor", SENSOR_OBSTRUCCION);
  measurement.addTag("accion", "salir");
  measurement.addValue("value", -1);

  // write it into db
  influx->write(measurement);
  
  }
  
void echo_int_fall(){
  izquierdo_duracion=micros() - tiempo;
  //izqInt=0;
  }

  
void echo_2_int_fall(){
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
