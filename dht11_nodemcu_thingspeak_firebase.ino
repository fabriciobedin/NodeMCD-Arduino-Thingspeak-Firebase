#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "DHT.h"
#include <FirebaseArduino.h>
 
//Define do pino a ser utilizado no ESP para o sensor = GPIO4
#define DHT_DATA_PIN 4
#define LDR_DATA_PIN A0
#define DHTTYPE DHT11
#define FIREBASE_HOST "testeesp8266.firebaseio.com"
#define FIREBASE_AUTH "TR5eDIZZSyNDciF9JD9epICT2xJdQJPl4MOJCVZo"
#define WIFI_SSID "wlanfabricio"
#define WIFI_PASSWORD "xxxxxxx"
 
//Definir o SSID da rede WiFi
//const char* WIFI_SSID = "wlanfabricio";
//Definir a senha da rede WiFi
//const char* WIFI_PASSWORD = "xxxxxxx";
 
//Colocar a API Key para escrita neste campo
//Ela é fornecida no canal que foi criado na aba API Keys
String apiKey = "MC9KBKNIXEIB9M5T";
const char* server = "api.thingspeak.com";
 
DHT dht(DHT_DATA_PIN, DHTTYPE);
WiFiClient client;
 
void setup() {
  //Configuração da UART
  Serial.begin(9600);
  
//  pinMode(REFERENCIA, OUTPUT);
//  pinMode(NIVEL1, INPUT);
//  pinMode(NIVEL2, INPUT);
//  pinMode(NIVEL3, INPUT);

 //Inicia o WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Espera a conexão no router
  Serial.print("conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  dht.begin();

  //Logs na porta serial
  Serial.println("");
  Serial.print("Conectado na rede ");
  Serial.println(WIFI_SSID);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
 
void loop() {
 
  //Espera 5 segundos para fazer a leitura
  delay(1000);
  //Leitura de umidade
  float umidade = dht.readHumidity();
  //Leitura de temperatura
  float temperatura = dht.readTemperature();

  float luminosidade = analogRead(LDR_DATA_PIN);
 
  //Se não for um numero retorna erro de leitura
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Erro ao ler o sensor DHT11!");
    return;
  }

  if (isnan(luminosidade)) {
    Serial.println("Erro ao ler o sensor LDR!");
    return;
  }

  Firebase.setFloat("temperatura", temperatura);
  Firebase.setFloat("umidade", umidade);
  Firebase.setFloat("luminosidade", luminosidade);
  
  if (Firebase.failed()) {
      Serial.print("Erro, não conectado ao Firebase");
      Serial.println(Firebase.error());  
      return;
  }
 
  //Inicia um client TCP para o envio dos dados
  if (client.connect(server,80)) {
    String postStr = apiKey;
           postStr +="&amp;field1=";
           postStr += String(temperatura);
           postStr +="&amp;field2=";
           postStr += String(umidade);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
 
     //Logs na porta serial
     Serial.print("Temperatura: ");
     Serial.print(temperatura);
     Serial.print(" Umidade: ");
     Serial.print(umidade);
     Serial.print(" Luminosidade: ");
     Serial.println(luminosidade);
  }
  client.stop();
}
