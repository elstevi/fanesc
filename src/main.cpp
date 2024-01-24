/*
 Steven Douglas
 https://github.com/elstevi/fanesc
 */
/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-web-server-slider-pwm/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "DShotESC.h"

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

const int output = 2;

String sliderValue = "0";
DShotESC esc0;

const char* PARAM_INPUT = "value";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>snow fan speed</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
  </style>
</head>
<body>
  <h2>snow fan speed</h2>
  <p><span id="textSliderValue">%SLIDERVALUE%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this)" id="pwmSlider" min="0" max="1999" value="%SLIDERVALUE%" step="1" class="slider"></p>
  <p><a href=reboot>reboot<a/>
  <p><a onclick="updateSliderPWM2(0)">off<a/>
  <p><a onclick="updateSliderPWM2(100)">100<a/>
  <p><a onclick="updateSliderPWM2(500)">500<a/>
  <p><a onclick="updateSliderPWM2(1000)">1000<a/>
  <p><a onclick="updateSliderPWM2(1200)">1200<a/>
  <p><a onclick="updateSliderPWM2(1500)">1500<a/>
  <p><a onclick="updateSliderPWM2(1700)">1700<a/>
  <p><a onclick="updateSliderPWM2(1999)">2000<a/>
<script>
function updateSliderPWM(element) {
  var sliderValue = document.getElementById("pwmSlider").value;
  document.getElementById("textSliderValue").innerHTML = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue, true);
  xhr.send();
}
function updateSliderPWM2(element) {
  var sliderValue = element;
  document.getElementById("textSliderValue").innerHTML = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if (var == "SLIDERVALUE"){
    return sliderValue;
  }
  return String();
}

void(* resetFunc) (void) = 0;

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  Serial.setDebugOutput(true);
  esp_log_level_set("*", ESP_LOG_VERBOSE);
  Serial.println("snowesp starting");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  delay(3000);
  Serial.println(WiFi.localIP());

  esc0.install(GPIO_NUM_14, RMT_CHANNEL_0);
  esc0.init();
  esc0.setReversed(false);
  esc0.set3DMode(false);
  esc0.throttleArm(6000);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    resetFunc();		  
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      // esc0.sendThrottle(100);
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
  // Start server
  server.begin();
}
  
void loop() {
  // you must send the throttle signal a few times per second
  esc0.sendThrottle(sliderValue.toInt());
  delay(1);
}
