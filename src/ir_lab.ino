#include <WiFi.h>

#include <WebServer.h>

#include <IRremote.h>

#include <FS.h>

#include <SPIFFS.h>

#include <ArduinoJson.h>



const char* ssid = "your wifi";

const char* password = "your wifi";

const int IR_LED_PIN = 4;

const int IR_RECV_PIN = 14;





WebServer server(80);

IRsend irsend(IR_LED_PIN);

IRrecv irrecv(IR_RECV_PIN);

decode_results results;




DynamicJsonDocument irCodes(4096);




void loadCodes() {

if (!SPIFFS.begin(true)) return;

File file = SPIFFS.open("/ircodes.json", "r");

if (!file) return;

deserializeJson(irCodes, file);

file.close();

}



void saveCodes() {

File file = SPIFFS.open("/ircodes.json", "w");

if (!file) return;

serializeJson(irCodes, file);

file.close();

}



void handleRoot() {

String html = "<html><body><h1>IR Lab</h1>";



html += "<h2>1. Learn Code</h2>";

html += "<form action='/learn' method='POST'>";

html += "Device: <input name='device'><br>";

html += "Button: <input name='button'><br>";

html += "<input type='submit' value='Learn'></form>";



html += "<h2>2. Send Code</h2>";

html += "<form action='/send' method='POST'>";

html += "Device: <input name='device'><br>";

html += "Button: <input name='button'><br>";

html += "<input type='submit' value='Send'></form>";



html += "<h2>3. Saved Codes</h2><pre>";

String output;

serializeJsonPretty(irCodes, output);

html += output;

html += "</pre></body></html>";

server.send(200, "text/html", html);

}



void handleLearn() {

if (server.hasArg("device") && server.hasArg("button")) {

String device = server.arg("device");

String button = server.arg("button");



Serial.println("Press button on remote...");

while (!irrecv.decode(&results)) { delay(10); }



irCodes[device][button] = String(results.value, HEX);

saveCodes();

irrecv.resume();

server.send(200, "text/plain", "Code learned!");

} else {

server.send(400, "text/plain", "Missing device or button");

}

}




void handleSend() {

if (server.hasArg("device") && server.hasArg("button")) {

String device = server.arg("device");

String button = server.arg("button");

String codeStr = irCodes[device][button];


if (codeStr.isEmpty()) {

server.send(404, "text/plain", "Code not found");

return;

}


unsigned long code = strtoul(codeStr.c_str(), NULL, 16);

irsend.sendNEC(code, 32); 

server.send(200, "text/plain", "Code sent!");

} else {

server.send(400, "text/plain", "Missing device or button");

}

}



void setup() {

Serial.begin(115200);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {

delay(500);

Serial.print(".");

}

Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());



irrecv.enableIRIn();

loadCodes();



server.on("/", handleRoot);

server.on("/learn", HTTP_POST, handleLearn);

server.on("/send", HTTP_POST, handleSend);

server.begin();

}



void loop() {

server.handleClient();

}
