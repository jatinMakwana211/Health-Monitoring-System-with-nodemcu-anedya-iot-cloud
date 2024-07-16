<h1>Health Monitoring with Cloud itegration using  NodeMCU, Anedya IoT Cloud & Streamlit</h1>
<br>
<br>

<h3> Overview </h3>
This repository contains the code and documentation for a comprehensive Health Monitoring and Reporting System. The system uses a NodeMCU (ESP8266) to collect Hartbeat,SPO2 and body temperature data from various sensors, stores the data on Anedya IoT Cloud, and visualizes it in real-time using a Streamlit dashboard.
<br>


<b> Features </b>
<b>Real-time Monitoring:</b> Measures Hartbeat,SPO2 and body temperature  <br>
<b>Cloud Storage:</b> Stores data on Anedya IoT Cloud for remote access and long-term storage. <br>
<b>Interactive Dashboard:</b> Visualizes data using a Streamlit-based web application. <br>
<br>
<br>
<br>
<b>Components </b> <br>
NodeMCU (ESP8266) <br>
MAX30100 Sensor (Hartbeat,SPO2) <br>
DS18B20 Sensor(Body temperature) <br>
<br>
<br>

<b> Setup Instructions </b>
<br>
Hardware Connections <br> <br>
<b>Max30100 sensor </b>  <br>

VCC to +5V <br>
GND to GND <br>
SCL pin  D1 <br>
SDA pin D2
<br>

 <b> DS18B20 Sensor: </b> <br>
VCC to +5V  <br>
GND to GND <br>
Data pin  D3 <br>
<br>
<br>
<br>


<b> Install Required Libraries </b> <br>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

<br>
<br>
<b> Upload Code to NodeMCU </b> <br>

<p>Open the <i><strong> Health Monitering System.ino </strong> </i>file in the Arduino IDE. Configure your WiFi credentials and Anedya IoT Cloud API endpoint in the code. Then upload the code to your NodeMCU. This file is available in Firmware folder.</p> <br>
<br>
 
<b> Set Up Anedya IoT Cloud </b> <br>
Create an account on Anedya IoT Cloud.<br>
Set up a new project and configure the endpoints for data reception.<br>

<br>
<b>Install Streamlit and Run Dashboard </b> <br>
<p>
 Ensure you have Python and pip installed. Then install Streamlit and run the dashboard.
</p> <br>

 <i> pip install streamlit <br>

streamlit run dashboard.py </i>

The <strong>dashboard.py </strong>script should be located in the root directory of the cloned repository. <br>

<br>
<br>

<b>Contributing </b> <br>

Contributions are welcome! Please fork the repository and submit a pull request.
 
