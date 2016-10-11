/*
   weather iot  :D
   this is a weather client device
   connected to mqtt broker that
   send a json data using node-red and a node js server
   connected to the same server and a neopixel ring
   to show the temperature

   Yeffri J. Salazar, najt labs Guatemala 2016


*/
//libraries required

#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//-------- temperature setup --------//
const int8_t minTemp = 0;
const int8_t maxTemp = 30;//-------- mqtt setup --------//
const char* mqtt_server =  ";
const int   mqtt_port =  ;
const char* mqtt_user = ";
const char* mqtt_password = "";
const char* mqtt_topicpub = "Pubtopic";
const char* mqtt_topicsub = "SubTopic";
const char* mqtt_alivetopic = "alive";
WiFiClient espClient;
PubSubClient client(espClient);

//-------- neopixel setup --------//

const int PIN = 0;
const int Numpixels = 12;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(Numpixels, PIN, NEO_GRB + NEO_KHZ800);

//-------- wifi setup --------//
const char* ssid  = "";
const char* password = "";

//-------- global variables --------//
long lastMsg = 0;
char msg[50];
int value = 0;

DynamicJsonBuffer  jsonBuffer;
void setup() {
  strip.setBrightness(10);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(115200);//begin the Serial comunnication
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  setup_wifi();// connect to wifi
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

/*
   setup wifi connect to wifi with the constants
   defined up
   while does not connect print a "."
   if connect then print the local ip
   over

*/

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/*
   callback
   this function receive the info from mqtt suscription
   and print the infro that comes through
*/

void callback(char* topic, uint8_t* payload, unsigned int length) {
  String jsonin;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    jsonin += (char)payload[i];
  }
  Serial.println(jsonin);
  parseJson(jsonin);
}

//-------- json in--------//

void parseJson(String json) {
 // StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  int temp = root["main"]["temp"];
  temp = temp -273.15;
  Serial.println((int8_t)temp);
  showTemp(temp);
}

void showTemp(int8_t temp) {
  Serial.println("showTemp");
  Serial.println(temp);
  
  uint8_t tempPos = map(temp, minTemp, maxTemp, 0, 11);
  Serial.println(tempPos);
  for (uint8_t i = 0; i < Numpixels; i++) {
    strip.setPixelColor(i, 0);
    Serial.println(i);

    strip.show();
    delay(100);
  }
  for (uint8_t i = 0; i < tempPos; i++) { 
    uint8_t  tempColor = map(i, 0, 11, 170, 255);
 
    strip.setPixelColor(i, Wheel(tempColor));
    Serial.println(i);
    strip.show();
  }
}

/*
   when the mqtt client is disconnected,
   then try to reconnect to mqtt broker.
   and suscribe the mqtt suscription topic
   else try to reconnect each 5 seconds
*/

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_alivetopic, "alive");
      // ... and resubscribe
      client.subscribe(mqtt_topicsub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
