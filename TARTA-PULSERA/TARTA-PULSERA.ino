#include <Wire.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "MAX30105.h"
#include <BluetoothSerial.h>
#include "heartRate.h"
#include <Adafruit_MLX90614.h>

MAX30105 particleSensor;
WiFiMulti wifiMulti;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
BluetoothSerial SerialBT;

#define D2 25
#define D3 26
#define D4 27

const byte RATE_SIZE = 10; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
String cadenas[100];
int cadenaPosition = 0;
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
int programPart = 0;

float beatsPerMinute;
int beatAvg;
int avgTemp;
String user;

int red = D3;
int green = D2;
int blue = D4;

int finalTemp=0;

uint8_t address[6] = {0x8C, 0x1A, 0xBF, 0xEE, 0xCE, 0x29};
String MACadd = "8C:1A:BF:EE:CE:29";

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_NONE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_MASTER;
esp_err_t connHan;

HTTPClient http;

void setup(){
  Serial.begin(115200);
  user = WiFi.macAddress();

  pinMode(green, OUTPUT); // Verde
  pinMode(blue, OUTPUT); // Azul
  pinMode(red, OUTPUT); // Rojo
  
  wifiMulti.addAP("AndroidAPDBC0","urmk5912");
  
  if(esp_spp_start_discovery(address) == ESP_OK){
    Serial.println(F("Device found!"));
  }else{
    Serial.println(F("Device not found :("));
  }
  
  connHan = esp_spp_connect(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_MASTER, 3,address);
  delay(1000); 
}

void loop(){
  if(programPart == 0){
    mlx.begin();
    avgTemp = 0;
    for(int i=0; i<10; i++){
      avgTemp = avgTemp + mlx.readObjectTempC();
    }

    finalTemp = avgTemp/10;
  
    Serial.print(F("Temp is: "));
    Serial.print(finalTemp);
    Serial.print(F("ºC\n"));

    if(finalTemp < 37){
      Serial.print(F("Cambiado a verde\n"));
      digitalWrite(green, HIGH);
      digitalWrite(blue, LOW);
      digitalWrite(red, LOW);
    }else if (finalTemp >= 37 && finalTemp < 38){
      Serial.print(F("Cambiado a amarillo\n"));
      digitalWrite(green, HIGH);
      digitalWrite(blue, LOW);
      digitalWrite(red, HIGH);
    }else if(finalTemp >= 38){
      Serial.print(F("Cambiado a rojo\n"));
      digitalWrite(red, HIGH); 
      digitalWrite(green, LOW);
      digitalWrite(blue, LOW);

    }
    programPart++;
  }else if(programPart == 1){
    if (!particleSensor.begin()){
      Serial.println(F("MAX30105 was not found. Please check wiring/power. "));
      while (1);
    }

    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
    
    long irValue = 0;
    
    for(int i=0; i< 10; i++){
      
      irValue = particleSensor.getIR();
  
      if (checkForBeat(irValue) == true){
        long delta = millis() - lastBeat;
        lastBeat = millis();
    
        beatsPerMinute = 60 / (delta / 1000.0);
    
        if (beatsPerMinute < 255 && beatsPerMinute > 20){
          rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
      }
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }else{
        i--;
      }
    }
    Serial.print(F("Avg BPM="));
    Serial.print(beatAvg);
    
  if (irValue < 50000)
    Serial.print(F(" No finger?"));

    Serial.println();
    programPart++;
    
  }else if(programPart > 1){
    programPart = 0;
    
    if(connHan != ESP_OK){
     connHan = esp_spp_connect(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_MASTER, 3,address);
    }
    if(wifiMulti.run() == WL_CONNECTED){
      String mensaje = "";
      if(finalTemp >= 38){
        mensaje = "{\"temperature\":39, \"cardique\": 70}";
      }else if(finalTemp >= 37 && finalTemp < 38){
        mensaje = "{\"temperature\":37.5, \"cardique\": 70}";
      }else{
        mensaje = "{\"temperature\":34, \"cardique\": 70}";
      }

      Serial.print(mensaje + "\n");
      http.begin("http://192.168.43.211:80/userdata");
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(mensaje);
        Serial.println(httpCode);
        if(httpCode < 0){
          cadenas[cadenaPosition] = mensaje;
          cadenaPosition += 1;
        }else{
          for(int i = 0; i < cadenaPosition; i++){
            http.POST(cadenas[i]);
          }
        }
      
    }
    delay(5000);
  }
}
