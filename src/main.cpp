#include <TinyGPS.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ClosedCube_HDC1080.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

#include <iostream>
#include <string>

#include <sstream>

TinyGPS gps;
ClosedCube_HDC1080 sensor;
SoftwareSerial ss(2,0);

//para conectar al servidor
const char* ssid = "dlink";//nombre de la red
const char* password = "";//contraseña
const char* server = "54.225.72.244";
WiFiClient client;
String respuesta;

static void smartdelay(unsigned long ms);
static float leer_temperatura(int num_lecturas);
static float leer_humedad(int num_lecturas);
void enviar_datos_h_t();
void enviar_datos_gps();
void metodo_post_h_t();
void metodo_post_gps();
unsigned long age;
float temperatura;
float humedad;
float media_temperatura;
float media_humedad;
float latitud;
float longitud;
String datos_h_t;
String datos_gps;

int j;

String estado;

void setup()
{
  Serial.begin(115200);
  sensor.begin(0x40);
  ss.begin(9600);

  delay(10);
  pinMode(A0, INPUT);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  j=0;
}

void loop()
{
  smartdelay(100);//100

  //Máquina de estados:
  estado="A";
  if(estado=="A")
  {
    leer_temperatura(5);//600 milisegundos
    smartdelay(100);//700 miisegundos
     estado = "B";
  }

  if(estado=="B")
  {
     leer_humedad(5);//1200 milisegundos
     smartdelay(100);//1300 miisegundos
     estado = "C";
  }

  if (estado == "C")
  {
     gps.f_get_position(&latitud,&longitud,&age);
     estado = "D";
  }
  
  if (estado == "D")//mandar humedad y temperatura
  {

     for(int i=0;i<87;i=i+1)//10000ms
     {
         smartdelay(100);
     }
    enviar_datos_gps();
    
    if(j%3==0)
    {
      enviar_datos_h_t();
      j=0;
    }
    j=j+1;
    estado = "E";
  }

  if (estado == "E")//mandar gps
  {
    
    smartdelay(100);
    estado="A";
  }



}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


static float leer_temperatura(int num_lecturas)
{

    temperatura=0;
    for(int i = 0; i < num_lecturas; i = i + 1)
    {
      temperatura += sensor.readTemperature(); 
      smartdelay(100);
    }
    media_temperatura= temperatura/num_lecturas;

    return media_temperatura;
}

static float leer_humedad(int num_lecturas)
{
    humedad=0;
    for(int i = 0; i < num_lecturas; i = i + 1)
    {
      humedad += sensor.readHumidity(); 
      smartdelay(100);
    }
    media_humedad= humedad/num_lecturas;

    return media_humedad;
}

void enviar_datos_h_t()
{
  if(client.connect(server,80))//si se conecta al servidor
  {
    Serial.print("Conexión exitosa_ht");
    metodo_post_h_t();
  }
}

void enviar_datos_gps()
{
  if(client.connect(server,80))//si se conecta al servidor
  {
    Serial.print("Conexión exitosa_gps");
    metodo_post_gps();
  }
}

void metodo_post_h_t()
{
  //JSON de humedad y temperatura
  datos_h_t  = "{\"id\":\"440004\",\"temperatura\":\""  + String(media_temperatura,6)+
  "\""+","+"\"humedad\":"+"\"" + String(media_humedad,6)+"\"}";
  client.print("POST /datos HTTP/1.1\n");
  client.print("Host: 54.225.72.244 \n");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(datos_h_t.length());
  client.println();
  client.println(datos_h_t);
}


void metodo_post_gps()
{
  //JSON de GPS
  datos_gps = "{\"id\":\"440004\",\"latitud\":\"" + String(latitud,6)+"\""+","+
  "\"longitud\":" + "\"" + String(longitud,6)+"\""+"}";
  //GPS
  client.print("POST /datos HTTP/1.1\n");
  client.print("Host: 54.225.72.244 \n");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(datos_gps.length());
  client.println();
  client.println(datos_gps);
 
}