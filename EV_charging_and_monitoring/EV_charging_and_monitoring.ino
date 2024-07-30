#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid = "EV CHARGING";
const char* password = "PASSWORD";

// Create an instance of the server
ESP8266WebServer server(80);

// Pin definitions
const int relayPin = D1; // Relay control pin
const int irSensorPin = D2; // IR sensor pin
const int muxS0 = D3; // MUX S0 pin
const int muxS1 = D4; // MUX S1 pin
const int muxS2 = D5; // MUX S2 pin
const int muxS3 = D6; // MUX S3 pin

// Analog pin for the multiplexer output
const int analogPin = A0;

// Voltage, current, and other values
float voltage = 0.0;
float current = 0.0;
float power = 0.0;
float energyBill = 0.0;

// Webpage HTML
const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>EV Charging Monitor</title>
</head>
<body>
  <h1>EV Charging Monitor</h1>
  <p>Charging Status: %STATUS%</p>
  <p>Voltage: %VOLTAGE% V</p>
  <p>Current: %CURRENT% A</p>
  <p>Power: %POWER% W</p>
  <p>Energy Bill: %BILL% $</p>
  <form action="/toggle" method="get">
    <button type="submit" name="state" value="ON">Turn ON</button>
    <button type="submit" name="state" value="OFF">Turn OFF</button>
  </form>
</body>
</html>
)rawliteral";

// Function to handle webpage requests
void handleRoot() {
  String status = digitalRead(relayPin) ? "ON" : "OFF";
  String webpage = html;
  webpage.replace("%STATUS%", status);
  webpage.replace("%VOLTAGE%", String(voltage));
  webpage.replace("%CURRENT%", String(current));
  webpage.replace("%POWER%", String(power));
  webpage.replace("%BILL%", String(energyBill));
  server.send(200, "text/html", webpage);
}

// Function to handle toggle relay requests
void handleToggle() {
  if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "ON") {
      digitalWrite(relayPin, HIGH);
    } else if (state == "OFF") {
      digitalWrite(relayPin, LOW);
    }
  }
  handleRoot();
}

// Function to read from the multiplexer and update sensor values
void updateSensorValues() {
  // Set MUX channel (for example, channel 0 for voltage)
  digitalWrite(muxS0, LOW);
  digitalWrite(muxS1, LOW);
  digitalWrite(muxS2, LOW);
  digitalWrite(muxS3, LOW);

  // Read voltage
  voltage = analogRead(analogPin) * (3.3 / 1023.0) * 5.0; // Adjust as necessary for your sensor

  // Set MUX channel (for example, channel 1 for current)
  digitalWrite(muxS0, HIGH);
  digitalWrite(muxS1, LOW);
  digitalWrite(muxS2, LOW);
  digitalWrite(muxS3, LOW);

  // Read current
  current = analogRead(analogPin) * (3.3 / 1023.0) * 5.0; // Adjust as necessary for your sensor

  // Calculate power and energy bill
  power = voltage * current;
  energyBill = power * 0.1; // Example calculation
}

void setup() {
  Serial.begin(115200);

  // Initialize relay and IR sensor pins
  pinMode(relayPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  pinMode(muxS0, OUTPUT);
  pinMode(muxS1, OUTPUT);
  pinMode(muxS2, OUTPUT);
  pinMode(muxS3, OUTPUT);

  // Initialize relay to OFF
  digitalWrite(relayPin, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  // Start server
  server.begin();
}

void loop() {
  server.handleClient();
  updateSensorValues();
}
