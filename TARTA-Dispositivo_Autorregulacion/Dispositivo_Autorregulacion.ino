#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#include <ArduinoJson.h>

#define ROOMSIZE 4
String roomsIds[ROOMSIZE] = {"123", "234", "345", "456"}; // Se crea una cadena de habitaciones


WiFiMulti wifiMulti;  // Variable para detectar la wifi

const uint16_t kIrLed = 25;  // El pin ESP GPIO que se usará para controlar el emisor
IRac ac(kIrLed);  // Crear un objeto A/C usando GPIO para enviar los mensajes

void setup() {
  Serial.begin(115200);
  delay(200);



  ac.next.protocol = decode_type_t::COOLIX;  // Enviar el protocolo COOLIX
  ac.next.model = 1;  // Probar el primer modelo
  ac.next.mode = stdAc::opmode_t::kCool;  // Utilizar el modo Cool
  ac.next.celsius = true;  // Usamos celsius para las unidades de la temperatura. False = Fahrenheit
  ac.next.degrees = 25;  // 25 grados por defecto.
  ac.next.fanspeed = stdAc::fanspeed_t::kMedium;  // Enciende el A/C a temperatura media.
  ac.next.swingv = stdAc::swingv_t::kOff;  // No mueva el ventilador hacia arriba o hacia abajo.
  ac.next.swingh = stdAc::swingh_t::kOff;  //No mueva el ventilador hacia la izquierda o hacia la derecha.
  ac.next.light = false;  // Apagar cualquier LED / Luces / Pantalla que podamos.
  ac.next.beep = false;  // Apagar cualquier pitido del A/C si podemos.
  ac.next.econo = false;  // Apagar cualquier modo económico si podemos.
  ac.next.filter = false;  // Apagar los filtros de iones / moho / salud si podemos.
  ac.next.turbo = false;  // No usar ningún modo turbo / potente / etc.
  ac.next.quiet = false;  // No usar ningún modo silencioso / silencioso / etc.
  ac.next.sleep = -1;  // No establecer ningún modo o tiempo de suspensión.
  ac.next.clean = false;  // Apaguar las opciones de limpieza si podemos.
  ac.next.clock = -1;  // No establecer ninguna hora actual si podemos evitarla.
  ac.next.power = false;  // Comenzar inicialmente con la unidad apagada.


  wifiMulti.addAP("AndroidAP2693", "ychl2034"); //Utilizar los datos de la wifi: (red , contraseña)
  //wifiMulti.addAP("AndroidAPDBC0", "urmk5912");
}

void loop() {



  if ((wifiMulti.run() == WL_CONNECTED)) {    // Comprobar si se está conectado a la wifi

    Serial.println("holi 123");
    String message = "";
    HTTPClient http;  // Crear una variable http

    http.begin("http://192.168.43.211:80/changeTemp?roomId=123"); // Obtener una respuesta del servidor para conocer si cambiamos la temperatura o no


    int httpCode = http.GET();  // Obtener el código de respuesta.


    if (httpCode > 0) {

      if (httpCode == HTTP_CODE_OK) {   // Si el código es 200, obtener la respuesta

        message = "yes";
        Serial.println(message);

      }
    }

    http.end();   //Finalizamos la variable http.



    if (message == "yes") {

      HTTPClient http2; // Crear una variable http

      http2.begin("http://192.168.43.211:80/getNewTemp?roomId=123"); // Obtener una respuesta del servidor para conocer la nueva temperatura a cambiar

      httpCode = http2.GET(); // Obtener el código de respuesta.

      if (httpCode > 0) {

        if (httpCode == HTTP_CODE_OK) { // Si el código es 200, obtener la respuesta

          float value = 23;
          Serial.println(value);

          ac.next.power = true;  // Encender la unidad A/C
          Serial.println("Sending a message to turn ON the A/C unit.");
          ac.next.degrees = value; // Utilizamos la temperatura que nos manda el servidor.
          ac.sendAc();  // Creamos y enviamos el mensaje a la unidad A/C.
          delay(10000);  // Esperar 10 segundos.

          ac.next.power = false;  // Apagar la unidad A/C
          Serial.println("Send a message to turn OFF the A/C unit.");
          ac.sendAc();  // Creamos y enviamos el mensaje a la unidad A/C.
          Serial.println("enviada señal");

        }
      }
      http2.end();//Finalizamos la variable http.
    }

  } else {
    Serial.println("no entra");
  }

  delay(60000); // Esperar 1 minuto


}
