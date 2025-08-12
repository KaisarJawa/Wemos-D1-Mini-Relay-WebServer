#include <Arduino.h>

// Conditional compilation for ESP8266 and ESP32
// This ensures the correct libraries are included for your board.
#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#else // Assume ESP32
#include <WiFi.h>
#include <WebServer.h>
#endif

// Define the web server's port
#define HTTP_PORT 80

// --- WiFi Configuration ---
// Change these to your WiFi network credentials.
const char *ssid = "YourSSID";
const char *password = "YourPass";

// Define a static IP for consistent network access.
// IMPORTANT: Update these values to match your local network settings.
IPAddress staticIP(192, 168, 1, 100); // Example: Your desired static IP
IPAddress gateway(192, 168, 1, 1);    // Example: Your router's IP address
IPAddress subnet(255, 255, 255, 0);   // Example: Your network's subnet mask
IPAddress dns1(8, 8, 8, 8);           // Example: A public DNS server (Google)

// --- Relay Pin Definitions ---
// Change these to the GPIO pins connected to your relay module.
// Note: Many relay modules use inverted logic (LOW = ON, HIGH = OFF).
const int RELAY1_PIN = 16;
const int RELAY2_PIN = 17;

// --- Web Server Initialization ---
#ifdef ESP8266
ESP8266WebServer webServer(HTTP_PORT);
#else
WebServer webServer(HTTP_PORT);
#endif

// --- Function Prototypes ---
void setRelayState(int relayPin, int state, const char *name);
void handleRelay1On();
void handleRelay1Off();
void handleRelay2On();
void handleRelay2Off();
void handleAllRelaysOn();
void handleAllRelaysOff();
void handleRoot();
void handleNotFound();

// --- Functions to Handle Relay States ---
// Sets the state of a specified relay pin and prints the status to the Serial monitor.
void setRelayState(int relayPin, int state, const char *name)
{
    digitalWrite(relayPin, state);
    Serial.print(name);
    Serial.print(" set to: ");
    // Using a ternary operator for a cleaner ON/OFF output.
    Serial.println(state == LOW ? "ON" : "OFF");
}

// --- Handler Functions for Web Server Endpoints ---
// Each function corresponds to a specific URL and controls the relays.
void handleRelay1On()
{
    setRelayState(RELAY1_PIN, LOW, "Relay1");
    webServer.send(200, "text/plain", "Relay 1 ON");
}

void handleRelay1Off()
{
    setRelayState(RELAY1_PIN, HIGH, "Relay1");
    webServer.send(200, "text/plain", "Relay 1 OFF");
}

void handleRelay2On()
{
    setRelayState(RELAY2_PIN, LOW, "Relay2");
    webServer.send(200, "text/plain", "Relay 2 ON");
}

void handleRelay2Off()
{
    setRelayState(RELAY2_PIN, HIGH, "Relay2");
    webServer.send(200, "text/plain", "Relay 2 OFF");
}

void handleAllRelaysOn()
{
    setRelayState(RELAY1_PIN, LOW, "Relay1");
    setRelayState(RELAY2_PIN, LOW, "Relay2");
    webServer.send(200, "text/plain", "All Relays ON");
}

void handleAllRelaysOff()
{
    setRelayState(RELAY1_PIN, HIGH, "Relay1");
    setRelayState(RELAY2_PIN, HIGH, "Relay2");
    webServer.send(200, "text/plain", "All Relays OFF");
}

// Handler for the root URL ("/"). It displays a list of available commands.
void handleRoot()
{
    String text = "--- Web Relay Control ---\r\n\r\n"
                "Available Commands:\r\n"
                "  Relay1 ON:  /r1on\r\n"
                "  Relay1 OFF: /r1off\r\n"
                "  Relay2 ON:  /r2on\r\n"
                "  Relay2 OFF: /r2off\r\n"
                "  All ON:     /rallon\r\n"
                "  All OFF:    /ralloff\r\n";
    webServer.send(200, "text/plain", text);
}

// Handler for URLs that are not defined.
void handleNotFound()
{
    String message = "404: File Not Found\r\n\r\n";
    message += "URI: ";
    message += webServer.uri();
    webServer.send(404, "text/plain", message);
}

// --- Setup Function: Runs once on boot ---
void setup()
{
    // Initialize Serial communication for debugging
    Serial.begin(115200);

    // Set relay pins as outputs and ensure they are initially OFF (HIGH)
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);

    // Configure and connect to Wi-Fi
    WiFi.setHostname("esp8266-webserver");
    WiFi.config(staticIP, gateway, subnet, dns1);
    WiFi.begin(ssid, password);

    Serial.print("\nConnecting to WiFi: ");
    Serial.println(ssid);
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Wait for a Wi-Fi connection with a 30-second timeout
    unsigned long connectTimeout = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectTimeout < 30000)
    {
        delay(500);
        Serial.print(".");
    }

    // Check if the connection was successful
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());

        // Register the URL handlers
        webServer.onNotFound(handleNotFound); // Catch-all for undefined URLs
        webServer.on("/", handleRoot);
        webServer.on("/r1on", handleRelay1On);
        webServer.on("/r1off", handleRelay1Off);
        webServer.on("/r2on", handleRelay2On);
        webServer.on("/r2off", handleRelay2Off);
        webServer.on("/rallon", handleAllRelaysOn);
        webServer.on("/ralloff", handleAllRelaysOff);

        // Start the web server
        webServer.begin();
        Serial.println("HTTP server started");
    }
    else
    {
        Serial.println("\nFailed to connect to WiFi.");
        Serial.print("Final WiFi Status: ");
        Serial.println(WiFi.status());
    }
}

// --- Loop Function: Runs continuously ---
void loop()
{
    // Only handle web client requests if the device is connected to WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
        webServer.handleClient();
    }
}
