#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "";
const char* password = "";
WiFiClient client;
// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO    true

// Set number of relays
#define NUM_RELAYS  5

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {16, 5, 4, 0, 2};

bool ledState0 = 0;
bool ledState1 = 0;
bool ledState2 = 0;
bool ledState3 = 0;

const int pinD0=16;
const int pinD1 = 5;
const int pinD2 = 4;
const int pinD3 = 0;
const int pinD4 = 2;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 16</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
     <div class="card">
      <h2>Output - GPIO 5</h2>
      <p class="state">state: <span id="state1">%STATE1%</span></p>
      <p><button id="button1" class="button">Toggle</button></p>
    </div>
     <div class="card">
      <h2>Output - GPIO 4</h2>
      <p class="state">state: <span id="state2">%STATE2%</span></p>
      <p><button id="button2" class="button">Toggle</button></p>
    </div>
     <div class="card">
      <h2>Output - GPIO 0</h2>
      <p class="state">state: <span id="state3">%STATE3%</span></p>
      <p><button id="button3" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    let state;
    let state1;
    let state2;
    let state3;
    
    if (event.data == "socket0_1"){
      state = "ON";
      document.getElementById('state').innerHTML  = state;
    }
    if(event.data == "socket0_0"){
      state = "OFF";
      document.getElementById('state').innerHTML  = state;
    }
    if (event.data == "socket1_1"){
      state1 = "ON";
       document.getElementById('state1').innerHTML = state1;
    }
    if(event.data == "socket1_0"){
      state1 = "OFF";
       document.getElementById('state1').innerHTML = state1;
    }
    if (event.data == "socket2_1"){
      state2 = "ON";
       document.getElementById('state2').innerHTML = state2;
    }
     if(event.data == "socket2_0"){
      state2 = "OFF";
       document.getElementById('state2').innerHTML = state2;
    }
    if (event.data == "socket3_1"){
      state3 = "ON";
        document.getElementById('state3').innerHTML = state3;
    }
    if(event.data == "socket3_0"){
      state3 = "OFF";
        document.getElementById('state3').innerHTML = state3;
    }
    
   
   
  
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
     document.getElementById('button1').addEventListener('click', toggle1);
      document.getElementById('button2').addEventListener('click', toggle2);
       document.getElementById('button3').addEventListener('click', toggle3);
  }
  function toggle(){
    websocket.send('toggle');
  }
   function toggle1(){
    websocket.send('socket1');
  }
   function toggle2(){
    websocket.send('socket2');
  }
   function toggle3(){
    websocket.send('socket3');
  }
</script>
</body>
</html>
)rawliteral";





void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
 
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    
    
    if (String((char*)data)=="toggle"){
      ledState0 = !ledState0;
     String sendText0 = ledState0 ? "socket0_1":"socket0_0";
     
      ws.textAll(String(sendText0));
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(pinD0, !ledState0);
      }
      else{
        Serial.print("NC ");
        digitalWrite(pinD0, ledState0);
      }
      

    }

    if(strcmp((char*)data, "socket1") == 0) {
      ledState1 = !ledState1;
     String sendText1 = ledState1 ? "socket1_1":"socket1_0";
      ws.textAll(String(sendText1));
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(pinD1, !ledState1);
      }
      else{
        Serial.print("NC ");
        digitalWrite(pinD1, ledState1);
      }

    }

    if (strcmp((char*)data, "socket2") == 0) {
      ledState2 = !ledState2;
   
    String sendText2 = ledState2 ? "socket2_1":"socket2_0";
      ws.textAll(String(sendText2));
       if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(pinD2, !ledState2);
      }
      else{
        Serial.print("NC ");
        digitalWrite(pinD2, ledState2);
      }
  
    }

    
    if (strcmp((char*)data, "socket3") == 0) {
      ledState3 = !ledState3;
      String sendText3 = ledState3 ? "socket3_1":"socket3_0";
      ws.textAll(String(sendText3));
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(pinD3, !ledState3);
      }
      else{
        Serial.print("NC ");
        digitalWrite(pinD3, ledState3);
      }
    }

  if (strcmp((char*)data, "__ping__") == 0) {
      Serial.printf("__pong__");
      ws.textAll(String("__pong__"));
    }

    
  }
  
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
          ws.textAll(String(ledState0 ? "socket0_1":"socket0_0"));
          ws.textAll(String(ledState1 ? "socket1_1":"socket1_0"));
          ws.textAll(String(ledState2 ? "socket2_1":"socket2_0"));
          ws.textAll(String(ledState3 ? "socket3_1":"socket3_0"));
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState0){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
   if(var == "STATE1"){
    if (ledState1){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
   if(var == "STATE2"){
    if (ledState2){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
   if(var == "STATE3"){
    if (ledState3){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }

  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}

void loop() {
  ws.cleanupClients();

  

}
