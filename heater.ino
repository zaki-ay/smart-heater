#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <config.h>

#define DHTPIN D6
#define DHTTYPE DHT11

/*******************
 * USER PARAMETERS
 *******************/

float wanted_temp = 25.0;
const int local_ip_address = 189;
const int room_number = 3;

const unsigned long servoMoveDelay = 5000;
unsigned int timeMoveServo = 750;
unsigned int moveUpServo = 100;
unsigned int moveDownServo = 80;
const bool full_servo = true;

//These values come from config.h, which is part of .gitignore for the purpose of security
//const char *ssid = "<network-name>";
//const char *password = "<network-password>";
/*******************
 * END USER PARAMETERS
 *******************/

bool auto_set = false;

DHT_Unified dht(DHTPIN, DHTTYPE);
Servo myservo;
ESP8266WebServer server(80);
uint32_t delayMS;

IPAddress staticIP(192, 168, 0, local_ip_address);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
  Serial.begin(9600);
  myservo.attach(D4);

  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  server.begin();
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;

  // Enable CORS for all routes
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Max-Age", "10000");
      server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "*");
      server.send(204);
    } else {
      server.send(404, "text/plain", "Not Found");
    }
  });

  server.on("/", HTTP_GET, handleRoot);
  server.on("/moveUp", HTTP_GET, handleMoveUp);
  server.on("/moveDown", HTTP_GET, handleMoveDown);
  server.on("/setWantedTemp", HTTP_POST, handleSetWantedTemp);
  server.on("/setAuto", HTTP_POST, handleSetAuto);
  server.on("/currentTemp", HTTP_GET, handleCurrentTemp);
  server.on("/currentMode", HTTP_GET, handleCurrentMode);
}

void handleRoot() {
  float current_temp = readTemperature();
  float current_humidity = readHumidity();

  String response = "<html>";
  response += "<head><meta charset='UTF-8'></head>";
  response += "<body>";
  response += "<h1>Room " + String(room_number) + "</h1>";
  response += "<p>Temperature: " + String(current_temp) + " °C</p>";
  response += "<p>Humidity: " + String(current_humidity) + " %</p>";
  response += "<p><a href='/moveUp'>Move Servo Up</a></p>";
  response += "<p><a href='/moveDown'>Move Servo Down</a></p>";
  response += "<form action='/setWantedTemp' method='post'>";
  response += "<label for='wanted_temp'>Set Temperature:</label>";
  response += "<input type='number' name='wanted_temp' step='0.5' value='" + String(wanted_temp) + "'/>";
  response += "<input type='submit' value='Set'/>";
  response += "<p>Auto mode: " + String(auto_set) + "</p>";
  response += "</form>";
  response += "</body></html>";

  // Enable CORS for this route
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", response);
}

void handleMoveUp() {
  moveUp();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Moving servo up");
}

void handleMoveDown() {
  moveDown();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Moving servo down");
}

void handleSetWantedTemp() {
  if (server.hasArg("wanted_temp")) {
    wanted_temp = server.arg("wanted_temp").toFloat();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Set to " + String(wanted_temp));
  } else {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Bad Request");
  }
}

void handleCurrentTemp() {
  float current_temp = readTemperature();

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");

  server.send(200, "text/plain", String(current_temp));
}

void handleCurrentMode() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");

  server.send(200, "text/plain", String(auto_set));
}

float readTemperature() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
    return NAN;
  } else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    return event.temperature;
  }
}

float readHumidity() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
    return NAN;
  } else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    return event.relative_humidity;
  }
}

void moveUp() {
  if (full_servo) {
    myservo.write(moveUpServo);
    delay(timeMoveServo);
    myservo.write(90);
  } else {
    myservo.write(moveUpServo*10);
  }
}

void moveDown() {
  if (full_servo) {
    myservo.write(moveDownServo);
    delay(timeMoveServo);
    myservo.write(90);
  } else {
    myservo.write(moveDownServo*10);
  }
}

unsigned long servoMoveTimer = 0;

void handleSetAuto() {
  if (server.hasArg("auto_set")) {
    auto_set = server.arg("auto_set").equals("1");  // Convert string to boolean
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Auto ON");
  } else {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "text/plain", "Bad Request");
  }
}

void loop() {
  server.handleClient();
  Serial.println(auto_set);
  if (auto_set == true) {  // Compare with true instead of 1
    unsigned long currentMillis = millis();
  
    // Check if it's time to move the servo
    if (currentMillis - servoMoveTimer >= servoMoveDelay) {
      float current_temp = readTemperature();
  
      if (!isnan(current_temp) && abs(current_temp - wanted_temp) > 0.5) {
        if (current_temp < wanted_temp) {
          moveUp();
        } else {
          moveDown();
        }
  
        servoMoveTimer = currentMillis;
      }
    }
  }
}
