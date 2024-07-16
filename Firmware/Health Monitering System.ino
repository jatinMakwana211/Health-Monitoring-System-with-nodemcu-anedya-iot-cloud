#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Replace with your own Anedya and WiFi credentials
String regionCode = "ap-in-1";
const char *deviceID = "25b8c097-590f-4d84-a734-776ac7c0fb1b"; 
const char *connectionkey = "45c168dc4b8fc2d085de0c3a547b557b"; 
const char *ssid = "Jatin"; 
const char *pass = "jatin123"; 

// MQTT connection settings
String str_broker = "mqtt." + String(regionCode) + ".anedya.io";
const char *mqtt_broker = str_broker.c_str(); 
const char *mqtt_username = deviceID; 
const char *mqtt_password = connectionkey; 
const int mqtt_port = 8883; 
String responseTopic = "$anedya/device/" + String(deviceID) + "/response"; 
String errorTopic = "$anedya/device/" + String(deviceID) + "/errors"; 

long long submitTimer;     
String timeRes, submitRes; 

// OneWire setup for DS18B20 sensor
#define ONE_WIRE_BUS D2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// MAX30100 sensor setup
#define REPORTING_PERIOD_MS 1000
PulseOximeter pox;
uint32_t tsLastReport = 0;

// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);


void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.begin(ssid, pass);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  submitTimer = millis();

  esp_client.setInsecure();
  mqtt_client.setServer(mqtt_broker, mqtt_port); 
  mqtt_client.setKeepAlive(60);                  
  mqtt_client.setCallback(mqttCallback);         
  connectToMQTT();                               
  mqtt_client.subscribe(responseTopic.c_str());  
  mqtt_client.subscribe(errorTopic.c_str());     

  setDevice_time(); 

  sensors.begin();

  // Initialize the MAX30100 sensor
  if (!pox.begin()) {
    Serial.println("Failed to initialize MAX30100 sensor!");
    while (1);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void loop() {
  if (!mqtt_client.connected()) {
    connectToMQTT();
  }
  mqtt_client.loop();

  sensors.requestTemperatures(); 
  float temperature = sensors.getTempCByIndex(0); 

  Serial.print("Temperature is: ");
  Serial.print(temperature);
  Serial.println(" °C");

  anedya_submitData("temperature", temperature); 

  // Read heart rate and SpO2
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    float heartRate = pox.getHeartRate();
    float spO2 = pox.getSpO2();
    Serial.print("Heart rate:");
    Serial.print(heartRate);
    Serial.print(" bpm / SpO2:");
    Serial.print(spO2);
    Serial.println(" %");

    anedya_submitData("heartrate", heartRate);
    anedya_submitData("spO2", spO2);

    tsLastReport = millis();
  }

  delay(5000); 
}

void connectToMQTT() {
  while (!mqtt_client.connected()) {
    const char *client_id = deviceID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) { 
      Serial.println("Connected to Anedya broker");
    } else {
      Serial.print("Failed to connect to Anedya broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Retrying in 5 seconds.");
      delay(5000);
    }
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  char res[150] = "";

  for (unsigned int i = 0; i < length; i++) {
    res[i] = payload[i];
  }
  String str_res(res);
  if (str_res.indexOf("deviceSendTime") != -1) {
    timeRes = str_res;
  } else {
    submitRes = str_res;
    Serial.println(str_res);
  }
}

void setDevice_time() {
  String timeTopic = "$anedya/device/" + String(deviceID) + "/time/json"; 
  const char *mqtt_topic = timeTopic.c_str();
  if (mqtt_client.connected()) {
    Serial.print("Time synchronizing......");

    boolean timeCheck = true; 
    long long deviceSendTime;
    long long timeTimer = millis();
    while (timeCheck) {
      mqtt_client.loop();

      unsigned int iterate = 2000;
      if (millis() - timeTimer >= iterate) { 
        Serial.print(".");
        timeTimer = millis();
        deviceSendTime = millis();

        StaticJsonDocument<200> requestPayload;            
        requestPayload["deviceSendTime"] = deviceSendTime; 
        String jsonPayload;                                
        serializeJson(requestPayload, jsonPayload);        
        const char *jsonPayloadLiteral = jsonPayload.c_str();
        mqtt_client.publish(mqtt_topic, jsonPayloadLiteral);

      } 

      if (timeRes != "") { 
        String strResTime(timeRes);

        DynamicJsonDocument jsonResponse(100);     
        deserializeJson(jsonResponse, strResTime); 

        long long serverReceiveTime = jsonResponse["serverReceiveTime"]; 
        long long serverSendTime = jsonResponse["serverSendTime"];       

        long long deviceRecTime = millis();                                                                
        long long currentTime = (serverReceiveTime + serverSendTime + deviceRecTime - deviceSendTime) / 2; 
        long long currentTimeSeconds = currentTime / 1000;                                                 

        setTime(currentTimeSeconds); 
        Serial.println("\n synchronized!");
        timeCheck = false;
      } 
    } 
  } else {
    connectToMQTT();
  } 
} 

void anedya_submitData(String datapoint, float sensor_data) {
  boolean check = true;

  String strSubmitTopic = "$anedya/device/" + String(deviceID) + "/submitdata/json";
  const char *submitTopic = strSubmitTopic.c_str();
  while (check) {
    if (mqtt_client.connected()) {

      if (millis() - submitTimer >= 2000) {

        submitTimer = millis();
        long long current_time = now();                     
        long long current_time_milli = current_time * 1000; 

        String jsonStr = "{\"data\":[{\"variable\": \"" + datapoint + "\",\"value\":" + String(sensor_data) + ",\"timestamp\":" + String(current_time_milli) + "}]}";
        const char *submitJsonPayload = jsonStr.c_str();
        mqtt_client.publish(submitTopic, submitJsonPayload);
      }
      mqtt_client.loop();
      if (submitRes != "") {
        DynamicJsonDocument jsonResponse(100);    
        deserializeJson(jsonResponse, submitRes); 

        int errorCode = jsonResponse["errCode"]; 
        if (errorCode == 0) {
          Serial.println("Data pushed to Anedya!!");
        } else if (errorCode == 4040) {
          Serial.println("Failed to push data!!");
          Serial.println("unknown variable Identifier");
          Serial.println(submitRes);
        } else {
          Serial.println("Failed to push data!!");
          Serial.println(submitRes);
        }
        check = false;
        submitTimer = 5000;
      }
    } else {
      connectToMQTT();
    } 
  }
}
