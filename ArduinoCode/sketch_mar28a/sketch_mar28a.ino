#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// --- WIFI SETTINGS ---
const char* ssid = "";
const char* password = "";
WebServer server(80);
const int ledPin = 2;

unsigned long ledTurnOffTime = 0;
bool ledActive = false;

void handlePost() {
  // Check if there is actually a body in the request

  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    // DEBUG: Print the raw string to the Serial Monitor
    Serial.print("Raw Data Received: ");
    Serial.println(body);
    // Use DynamicJsonDocument for safer memory handling if payloads vary
    DynamicJsonDocument doc(512); 
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.print("JSON Deserialization failed: ");
      Serial.println(error.f_str());
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    // Safety Check: Ensure the "player" key actually exists and is a string
    if (doc.containsKey("player") && doc["player"].is<const char*>()) {
      const char* playerName = doc["player"];
      Serial.printf("Player %s joined!\n", playerName);

      digitalWrite(ledPin, HIGH);
      ledActive = true;
      ledTurnOffTime = millis() + 3000; 

      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      Serial.println("JSON received but 'player' field is missing or invalid.");
      server.send(422, "text/plain", "Missing player field");
    }
  } else {
    server.send(400, "text/plain", "No body received");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Connection fix
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/trigger-led", HTTP_POST, handlePost);
  server.begin();
}

void loop() {
  server.handleClient();

  if (ledActive && millis() > ledTurnOffTime) {
    digitalWrite(ledPin, LOW);
    ledActive = false;
  }
}
