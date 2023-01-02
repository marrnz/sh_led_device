#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>
#include <string>

// LED
#define NUM_LEDS 120
#define DATA_PIN D4
CRGB leds[NUM_LEDS];
CRGB previousColor = CRGB::Red;
bool writeOngoing = false;
bool showLeds = false;

// WIFI
const char *ssid = "";
const char *password = "";
const char *mqtt_server = "192.168.0.192";
WiFiClient espClient;

// MQTT
#define MSG_BUFFER_SIZE (50)
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void changePower(bool on)
{
  // Serial.println("Changing Power");
  CRGB color = CRGB::Red;
  if (!on)
  {
    // Serial.println("Power off!");
    color = CRGB::Black;
  }
  // Serial.print("Color: ");
  // Serial.println(color);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

void changeLedColor(CRGB color)
{
  previousColor = color;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.println("] ");

  // unsigned long time = micros();
  std::string message = "";
  for (int i = 0; i < length; i++)
  {
    message = message + (char)payload[i];
  }
  Serial.println();
  if (message.find("power") != std::string::npos)
  {
    writeOngoing = true;
    if (message.find("on") != std::string::npos)
    {
      changePower(true);
    }
    if (message.find("off") != std::string::npos)
    {
      changePower(false);
    }
  }
  if (message.find("color") != std::string::npos)
  {
    writeOngoing = true;
    std::string colorString = message.substr(5);
    Serial.print("Color string: ");
    Serial.println(colorString.c_str());
    unsigned int hexAsInt = std::stoul(colorString, nullptr, 16);
    Serial.print("Unsinged int: ");
    Serial.println(hexAsInt);
    changeLedColor(CRGB(hexAsInt));
  }
  // Serial.print("Time taken: ");
  // Serial.println(micros() - time);
  writeOngoing = false;
  showLeds = true;
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("smart_home/rooms/schlafzimmer/led/1");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS); // GRB ordering is typical
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Yellow;
  }
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  FastLED.show();
  delay(500);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  if (showLeds) {
    FastLED.show();
  }
  if (!writeOngoing)
  {
    client.loop();
  }
}