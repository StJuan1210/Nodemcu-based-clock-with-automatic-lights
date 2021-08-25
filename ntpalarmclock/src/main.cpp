#include <Arduino.h>
#include "FastLED.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include <FS.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define NUM_LEDS1 10
#define LED_TYPE    WS2813
#define COLOR_ORDER GRB
#define PIN1 D4
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int data=255;
int r,g,b;
void static1(int, int, int,int);
AsyncWebServer server(80);
CRGB leds1[NUM_LEDS1];
void alarmcl();
int hr1, minut1;
String outputStateValue = "null";
const char *ssid     = "Mattackal 2.4";
const char *password = "burka123";
const char* PARAM_INPUT_1 = "Hour";
const char* PARAM_INPUT_2 = "Minute";
const char* PARAM_INPUT_3 = "state";

const long utcOffsetInSeconds = 19800;
int lon = 0;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Alarm Clock</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
   <style>
    @import url('https://fonts.googleapis.com/css2?family=Poppins:wght@700&display=swap');
    html {font-family: 'Poppins',sans-serif; display: inline-block; text-align: center; background-color:#D6AEAE;}
    h2 {font-family: 'Poppins',sans-serif;font-size: 3.0rem;color: #241332;}
    p {font-family: 'Poppins',sans-serif;font-size: 3.0rem;color: #ffffff;}
    input[type = submit] {
            background-color: rgb(0, 0, 0);
            border: none;
            text-decoration: none;
            color: #ffffff;
            padding: 8px 8px;
            margin: 8px 8px;
            cursor: pointer;
            font-family: 'Poppins',sans-serif;
         }
    input[type=number]::-webkit-inner-spin-button {
                            color: #ffffff;
                            opacity: 1;
    }
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
  </style>
  
  <script>
    function submitMessage() {
      alert("Saved value to memory);
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>
  <h2>Set The Alarm</h2>
  <form action="/get" target="hidden-form">
    Hour: <input type="number" name="Hour"style="background-color:white; 
              border: solid 1px #6E6E6E;
              height: 20px; 
              font-size:18px; 
              color:#bbb" 
              placeholder="Hour" >
    <input type="submit"  value="Submit" onclick="submitMessage()">
  </form><br>
  <form action="/get" target="hidden-form">
    Minute: <input type="number" name="Minute"style="background-color:white; 
              border: solid 1px #6E6E6E;
              height: 20px; 
              font-size:18px; 
              color:#bbb" 
              placeholder="Minute">
    <input type="submit"   value="Submit" onclick="submitMessage()">
  </form><br>
  <h2>Light Control</h2>
  <form action="/get" target="hidden-form">
    State: <input type="number"  name="state"style="background-color:white; 
              border: solid 1px #6E6E6E;
              height: 20px; 
              font-size:18px; 
              color:#bbb" 
              placeholder="1 = on,0 = off">
    <input type="submit"  value="Submit" onclick="submitMessage()">
  </form><br>
  <iframe style="display:none" name="hidden-form"></iframe>
</body></html>)rawliteral";

String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }

  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
// Replaces placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputString"){
    return readFile(SPIFFS, "/Hour.txt");
  }
  else if(var == "inputInt"){
    return readFile(SPIFFS, "/Minute.txt");
  }
  else if(var == "state"){
    return readFile(SPIFFS, "/state.txt");
  }
  return String();
}
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);}
  display.clearDisplay();
  timeClient.begin();
  FastLED.addLeds<LED_TYPE, PIN1, COLOR_ORDER>(leds1, NUM_LEDS1).setCorrection( TypicalLEDStrip );
  if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;}
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);  });
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;  
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      writeFile(SPIFFS, "/Hour.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      writeFile(SPIFFS, "/Minute.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      lon = 1;
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {  
  timeClient.update();
  int i = 0;
  int Hour1 = readFile(SPIFFS, "/Hour.txt").toInt();
  int Minute1 = readFile(SPIFFS, "/Minute.txt").toInt();
  if(timeClient.getHours()==Hour1 && timeClient.getMinutes()==Minute1){
  alarmcl();}
  else{
  static1(0, 0, 0 ,0);
  FastLED.show();
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("Time>");
  display.print(timeClient.getHours());
  display.print(":");
  display.print(timeClient.getMinutes());
  display.display(); 
  delay(2000);
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("ALM>");
  display.print(Hour1);
  display.print(":");
  display.print(Minute1);
  display.display(); 
  delay(2000);
  display.clearDisplay();
  if(lon == 1){
    static1(150, 150, 150 ,150);
    FastLED.show();
    delay(10000);
    lon=0;

  }
Serial.println(outputStateValue);
}
void static1(int r, int g, int b,int brightness)
{
  FastLED.setBrightness(brightness);
  for (int i = 0; i < NUM_LEDS1; i++ )
  {
    leds1[i] = CRGB(r, g, b);
  }
  FastLED.show();
}
void alarmcl(){
  timeClient.update();
  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  //Serial.println(timeClient.getFormattedTime());
  static1(150, 150, 150 ,150);
  FastLED.show();

}