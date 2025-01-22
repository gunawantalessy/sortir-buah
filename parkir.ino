#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Pin Definitions for Infrared Sensors
#define IR_SENSOR_1 5   // GPIO5 (D1)
#define IR_SENSOR_2 4   // GPIO4 (D2)
#define IR_SENSOR_3 0   // GPIO0 (D3)
#define IR_SENSOR_4 2   // GPIO2 (D4)
#define IR_SENSOR_5 14  // GPIO14 (D5)

// WiFi Credentials (Will be used in Access Point mode)
const char* ssid = "GunawanTalessy_AP";  // Set SSID for ESP8266 WiFi
const char* password = "12345678";       // Set Password for ESP8266 WiFi

// ESP Web Server
ESP8266WebServer server(80);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27, 16 columns, 2 rows for LCD

// Parking Slot Status
bool slots[5] = {false, false, false, false, false}; // Status for 5 parking slots

// Portal state (if all slots are full, portal cannot be opened)
bool portalOpen = true;

void setup() {
  Serial.begin(115200);

  // Initialize Infrared Sensors
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(IR_SENSOR_3, INPUT);
  pinMode(IR_SENSOR_4, INPUT);
  pinMode(IR_SENSOR_5, INPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  // Set up WiFi to create an Access Point
  connectToWiFi();

  // Configure Web Server Routes
  server.on("/", handleRoot);         // Root page
  server.on("/status", handleStatus); // Page to get parking status in JSON format
  server.on("/portal", handlePortal); // Handle the portal (open/close)
  server.begin();                     // Start the server
  Serial.println("Web Server Started!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void loop() {
  // Handle web server requests
  server.handleClient();

  // Update slot statuses based on infrared sensors
  slots[0] = digitalRead(IR_SENSOR_1) == LOW;
  slots[1] = digitalRead(IR_SENSOR_2) == LOW;
  slots[2] = digitalRead(IR_SENSOR_3) == LOW;
  slots[3] = digitalRead(IR_SENSOR_4) == LOW;
  slots[4] = digitalRead(IR_SENSOR_5) == LOW;

  // Check if all slots are full and update portal state
  portalOpen = !areAllSlotsFull();

  // Update LCD with parking slot status
  lcd.setCursor(0, 0);
  lcd.print("Slot: ");
  lcd.print(slots[0] ? "1 " : "- ");
  lcd.print(slots[1] ? "2 " : "- ");
  lcd.print(slots[2] ? "3 " : "- ");
  lcd.setCursor(0, 1);
  lcd.print(slots[3] ? "4 " : "- ");
  lcd.print(slots[4] ? "5 " : "- ");
}

void connectToWiFi() {
  // Set ESP8266 as Access Point
  WiFi.mode(WIFI_AP);  
  WiFi.softAP(ssid, password);  // Create WiFi network

  // Print IP Address to Serial Monitor
  Serial.println("Access Point Created!");
  Serial.println("SSID: " + String(ssid));
  Serial.println("IP Address: " + WiFi.softAPIP().toString());
}

void handleRoot() {
  String html = "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<title>Parking System</title>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<style>"
                "body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; padding: 20px; }"
                "h1 { color: #4CAF50; }"
                "p { font-size: 18px; }"
                ".slot { font-size: 20px; margin-top: 10px; }"
                ".occupied { color: red; font-weight: bold; }"
                ".available { color: green; font-weight: bold; }"
                ".button { background-color: #4CAF50; color: white; padding: 15px 32px; text-align: center; display: inline-block; font-size: 16px; border: none; cursor: pointer; margin-top: 20px; }"
                ".button:disabled { background-color: #c3c3c3; cursor: not-allowed; }"
                "</style>"
                "</head>"
                "<body>"
                "<h1>Parking System</h1>"
                "<div id='slots'></div>"
                "<button class='button' id='portalButton' onclick='togglePortal()'>"
                + String(portalOpen ? "Open Portal" : "Portal Closed") +
                "</button>"
                "<script>"
                "function togglePortal() {"
                "  fetch('/portal');"
                "  setTimeout(() => location.reload(), 1000);"
                "}"
                "setInterval(() => {"
                "  fetch('/status').then(res => res.json()).then(data => {"
                "    const slots = data.slots;"
                "    let html = '';"
                "    slots.forEach((slot, index) => {"
                "      html += `<p class='slot'>Slot ${index + 1}: <span class='${slot ? 'occupied' : 'available'}'>${slot ? 'Occupied' : 'Available'}</span></p>`;"
                "    });"
                "    document.getElementById('slots').innerHTML = html;"
                "  });"
                "}, 1000);"
                "</script>"
                "</body>"
                "</html>";
  server.send(200, "text/html", html); // Serve the HTML page to the browser
}

void handleStatus() {
  String json = "{ \"slots\": [";
  for (int i = 0; i < 5; i++) {
    json += slots[i] ? "true" : "false";
    if (i < 4) json += ", ";
  }
  json += "] }";
  server.send(200, "application/json", json); // Send parking slot status in JSON format
}

void handlePortal() {
  // Toggle portal open/closed status
  portalOpen = !portalOpen;

  // Respond with the current portal status
  String response = portalOpen ? "Portal Opened!" : "Portal Closed!";
  server.send(200, "text/plain", response);
}

bool areAllSlotsFull() {
  // Check if all parking slots are full
  for (int i = 0; i < 5; i++) {
    if (!slots[i]) {
      return false; // If there's an empty slot, return false
    }
  }
  return true; // All slots are occupied
}
