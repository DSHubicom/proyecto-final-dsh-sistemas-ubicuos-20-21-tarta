/*
  ESP32 Web Server - TARTA
  by Descripcion de Sistemas Hardware
  Universida de Extremadura
*/

#include <WiFi.h>
#include <WebServer.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include <ArduinoJson.h>

HardwareSerial myHardwareSerial(1); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

// SSID & Password
const char* ssid = "AndroidAPDBC0"; // "AndroidAP"; // "AndroidAP"; // "MOVISTAR_D397"; // Enter your SSID here
const char* password = "urmk5912"; // "1234abcd"; // "1234abcd"; // "E8E4953EBBCD2739A670";  // Enter your Password here

WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)

DynamicJsonDocument doc(1024);

void setup() {
  // Initialize comun
  Serial.begin(115200);

  Serial.println("Initialize seed");
  randomSeed(39);

  // Initialize audio
  myHardwareSerial.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Tarta Server"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(myHardwareSerial, false)) {  // Use softwareSerial to communicate with mp3
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.play(1);  //Play the first mp3

  // Initialize server
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  // Connect to your wi-fi modem
  WiFi.begin(ssid, password);

  // Check wi-fi is connected to wi-fi network
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected successfully");

  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());


  // Handle endpoint
  server.on("/", handleRoot);

  // Userdata endpoints
  server.on("/userdata", HTTP_GET, getUserdata); // Example: /userdata?userdataId=12345
  server.on("/userdata", HTTP_POST, postUserdata); // Example: /userdata
  server.on("/userdata", HTTP_PUT, putUserdata); // Example: /userdata?userdataId=12345
  server.on("/userdata", HTTP_DELETE, deleteUserdata); // Example: /userdata?userdataId=12345

  // User endpoints
  server.on("/user", HTTP_GET, getUser); // Example: /user?userId=12345
  server.on("/user", HTTP_POST, postUser); // Example: /user
  server.on("/user", HTTP_PUT, putUser); // Example: /user?userId=12345
  server.on("/user", HTTP_DELETE, deleteUser); // Example: /user?userId=12345

  // Rooms ednpoints
  server.on("/changeTemp", HTTP_GET, changeTemp); // Example: /changeTemp?roomId=12345
  server.on("/getNewTemp", HTTP_GET, getNewTemp); // Example: /getNewTemp?roomId=12345
  server.on("/getNewTempByGroup", HTTP_GET, getNewTempByGroup); // Example: /getNewTempByGroup?roomId=12345

  // Login endpoint
  server.on("/login", HTTP_GET, login); // Example: /login?email=pepito@hotmail.com&password=1234abcd

  // Error endpoints
  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("HTTP server started");

  delay(100);
}

void loop() {
  server.handleClient();
}

// HTML & CSS contents which display on web server
String HTML = "<!DOCTYPE html>\<html>\<body>\<h1>TARTA Server with ESP32</h1>\</body>\</html>";

// Get userdata endpoint
void getUserdata() {
  Serial.println("Request getUserdata");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String userdataId = server.pathArg(0);

  // mock data
  String result = "{\"status\":\"OK\", \"data\": {\"id\": 12345, \"temperature\": 37, \"cardiaco\": 60, \"date\": \"Fri Dec 11 2020 19:04:18 GMT+0100 (hora estándar de Europa central) {}\"}}";

  server.send(200, "application/json", result);
}

// Post userdata endpoint
void postUserdata() {
  Serial.println("Request postUserdata");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String bodyString = server.arg(0);

  deserializeJson(doc, bodyString);
  JsonObject bodyJSON = doc.as<JsonObject>();

  long temperature = bodyJSON[String("temperature")];

  Serial.println("Temperature of user: " + String(temperature) + "ºC");

  // Trigger temperature alert
  if (temperature) {
    if (temperature > 37.2) { // febricula
      myDFPlayer.play(2);  //Play the first alert mp3

      if (myDFPlayer.available()) {
        printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
      }
    } else if (temperature < 36.1) { // hipoternmia
      myDFPlayer.play(1);  //Play the second alert mp3

      if (myDFPlayer.available()) {
        printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
      }
    }

  }

  String result = "{\"status\": \"OK\", \"data\": " + bodyString + "}";

  server.send(200, "application/json", result);
}

// Put userdata endpoint
void putUserdata() {
  Serial.println("Request putUserdata");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String userdataId = server.pathArg(0);

  // mock data
  String result = "{\"status\": \"OK\", \"data\": {\"id\": 12345, \"temperature\": 37, \"cardiaco\": 60, \"date\": \"Fri Dec 11 2020 19:04:18 GMT+0100 (hora estándar de Europa central) {}\"}}";

  server.send(200, "application/json", result);
}

// Delete userdata endpoint
void deleteUserdata() {
  Serial.println("Request deleteUserdata");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String userdataId = server.pathArg(0);

  // mock data
  String result = "{\"status\": \"OK\", \"data\": {}}";

  server.send(200, "application/json", HTML);
}

// Get user endpoint
void getUser() {
  Serial.println("Request getUser");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String userId = server.pathArg(0);

  // mock data
  String result = "{\"status\": \"OK\", \"data\": {\"id\": 12345, \"name\": \"Pepito\", \"surname\": \"Garcia Fernandez\", \"age\": 60, \"created\": \"Fri Dec 11 2020 19:04:18 GMT+0100 (hora estándar de Europa central) {}\"}}";

  server.send(200, "application/json", result);
}

// Post user endpoint
void postUser() {
  Serial.println("Request postUser");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  // mock data
  String result = "{\"status\": \"OK\", \"data\": {\"id\": 12345, \"name\": \"Pepito\", \"surname\": \"Garcia Fernandez\", \"age\": 60, \"created\": \"Fri Dec 11 2020 19:04:18 GMT+0100 (hora estándar de Europa central) {}\"}}";

  server.send(200, "application/json", result);
}

// Put user endpoint
void putUser() {
  Serial.println("Request putUser");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String userId = server.pathArg(0);

  // mock data
  String result = "{\"status\": \"OK\", \"data\": {\"id\": 12345, \"name\": \"Pepito\", \"surname\": \"Garcia Fernandez\", \"age\": 60, \"created\": \"Fri Dec 11 2020 19:04:18 GMT+0100 (hora estándar de Europa central) {}\"}}";


  server.send(200, "application/json", HTML);
}

// Delete user endpoint
void deleteUser() {
  Serial.println("Request deleteUser");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String userId = server.pathArg(0);

  // mock data
  String result = "{\"status\": \"OK\", \"data\": {}}";

  server.send(200, "application/json", result);
}

// Login endpoint
void changeTemp() {
  Serial.println("Request changeTemp");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  // mock data
  String result = "{\"status\": \"OK\", \"data\": \"YES\"";

  server.send(200, "application/json", result);
}

// Login endpoint
void getNewTemp() {
  Serial.println("Request getNewTemp");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  // mock data
  int randomTemperatureInt = random(19, 24);
  int randomTemperatureDecimal = random(0, 99);
  String randomTemperature = (String)randomTemperatureInt + "." + randomTemperatureDecimal;
  
  String result = "{\"status\": \"OK\", \"temperature\": " + randomTemperature  + " }";

  server.send(200, "application/json", result);
}

// Login endpoint
void getNewTempByGroup() {
  Serial.println("Request getNewTempByGroup");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  // mock data
  int randomTemperatureInt = random(19, 24);
  int randomTemperatureDecimal = random(0, 99);
  String randomTemperature = (String)randomTemperatureInt + "." + randomTemperatureDecimal;
  
  String result = "{\"status\": \"OK\", \"temperature\": " + randomTemperature + " }";

  server.send(200, "application/json", result);
}

// Login endpoint
void login() {
  Serial.println("Request login");
  if (server.args() == 0) {
    server.send(500, "text/plain", "BAD ARGS\r\n");
  }

  String email = server.pathArg(0);
  String password = server.pathArg(1);

  // mock data JWT
  String result = "{\"status\"= \"OK\", \"data\": {\"jwt\": \"yRQYnWzskCZUxPwaQupWkiUzKELZ49eM7oWxAQK_ZXw\"}}";

  server.send(200, "application/json", result);
}

// Handle root endpoint
void handleRoot() {
  Serial.println("Request handleRoot");
  server.send(200, "text/html", HTML);
}

// Handle not found endpoint
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
