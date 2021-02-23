#include "ESP8266WiFi.h"  // For WIFI
#include <PubSubClient.h> // For MQTT
#include <NTPClient.h>    // For date
#include <WiFiUdp.h>      // For date
#include "DHT.h"          // For sensors
#include <ArduinoJson.h>  // For json 

// WIFI Settings
const char* ssid = "";
const char* password = "";

// MQTT Settings
const char* mqtt_server = "";
const int mqtt_port = ;
const char* mqtt_user = "";
const char* mqtt_pass = "";

// Sensors Settings
const int water_sensor = A0;
const int dht_sensor = D4;
const int pause = 1000;

// Other Settings
const char* topic_temp_out = "";
const char* topic_soil_humidity = "";
const char* topic_humidity = "";

WiFiClient espClient;
PubSubClient client(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

DHT dht(dht_sensor, DHT11); 

// SETUP START

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi connected");
}

void setup_mqtt() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_date() {
  timeClient.begin();
  timeClient.setTimeOffset(0);
}

void setup_sensors() {
  dht.begin();
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  setup_mqtt();
  setup_date();
  setup_sensors();
}

// SETUP END

String get_current_date() {
  // Expected format: 2021-02-13T14:14:12Z
 
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  return timeClient.getFormattedDate();
}

String get_response(const float& value) {
      DynamicJsonDocument doc(1024);

      String current_date = get_current_date();
      doc["timestamp"] = current_date;
      doc["value"]   = value;

      String myOutput = "";
      serializeJson(doc, myOutput);
      return myOutput;  
}


// MQTT START

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  
}

void reconnect() {
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-"; // Create a random client ID
    clientId += String(random(0xffff), HEX);
  
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// MQTT END

// SENSORS START

String read_dht_temperature(){
  delay(pause);
  float t = dht.readTemperature();
  
  if (isnan(t)) {
    Serial.println("Error");
    return "";
  }

  Serial.print(" Temp, C: ");
  Serial.println(t);

  return get_response(t);
}

String read_dht_humidity(){
  delay(pause);
  float h = dht.readHumidity();
  
  if (isnan(h)) {
    Serial.println("Error");
    return "";
  }

  Serial.print(" Hum, %: ");
  Serial.println(h);

  return get_response(h);
}

String read_moisture(){
  delay(pause);
  float raw = analogRead(water_sensor);
  float percentage = (raw / 1024) * 100;

  Serial.print(" Moisture Sensor, %: ");
  Serial.println(percentage);
  
  return get_response(percentage);
}

// SENSORS END

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }

  String response = "";
  
  delay(10000);
    
  response = read_moisture();
  if (response != "") {
    client.publish(topic_soil_humidity, response.c_str());
  }

  response = read_dht_temperature();
  if (response != "") {
    client.publish(topic_temp_out, response.c_str());
  }

  response = read_dht_humidity();
  if (response != "") {
    client.publish(topic_humidity, response.c_str());
  }
      
  //client.subscribe(topic_temp_out); // not necessarynot necessary 
  
  client.loop();

}
