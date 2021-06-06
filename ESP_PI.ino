#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

#include "DHT.h"

#define DHTPIN D1
#define DHTTYPE DHT11

#ifndef STASSID
#define STASSID "<YOUR SSID>"
#define STAPSK "<YOUR PASSWORD"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
// Add your MQTT Broker IP address, example:
const char *mqtt_server = "test.mosquitto.org";


DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;           
PubSubClient client(espClient); 

long lastMsg = 0;

float temperature = 0;
float humidity = 0;

//Prototypes
void setup_wifi();

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  dht.begin();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("<YOUR ID CONNECTION"))
    {
      Serial.println("connected");
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

void loop()
{
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  if (WiFi.status() != WL_CONNECTED)
  {
     Serial.println(" WIFI Disconected...");
    setup_wifi();    
  }
  
  if (!client.connected())
  {
     Serial.println("Disconected...");
     reconnect();
     return;
  }
  client.loop();
  
  long now = millis();
  
  if (now - lastMsg > 5000)
  {
    lastMsg = now;

    // Convert the value to a char array
    char tempString[8];
    dtostrf(t, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("<YOUR MQTT PORT>", tempString);
 
    // Convert the value to a char array
    char humString[8];
    dtostrf(h, 1, 2, humString);
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("<YOUR MQTT PORT>", humString);

    // Convert the value to a char array
    char hicString[8];
    dtostrf(hic, 1, 2, hicString);
    Serial.print("Compute heat index: ");
    Serial.println(hic);
    client.publish("<YOUR MQTT PORT>", hicString);
  }
}
