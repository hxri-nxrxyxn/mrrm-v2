#include <WiFi.h> // <-- Changed from ESP8266WiFi.h
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Gianna's S24+";
const char* password = "g2024271";

// HiveMQ broker details
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// HiveMQ credentials
const char* mqtt_username = "hivemq.webclient.1760861168522";
const char* mqtt_password = "q0c7JF8Kfv@%e9LQ!#Bn";

// Topic
const char* topic = "/hari";

// --- Sensor Definitions (Comments updated for ESP32) ---
#define TRIG_PIN_1 4    // GPIO 4
#define TRIG_PIN_2 5    // GPIO 5
#define ECHO_PIN 16     // GPIO 16 (RX2)
#define NUM_SAMPLES 5   // Number of samples to average for each sensor

WiFiClient espClient;
PubSubClient client(espClient);

// --- WiFi Setup (Unchanged logic) ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- Get Single Sensor Reading (Calculation MODIFIED) ---
float getSensorReading(int trigPin, int echoPin) {
  // Ensure trigger pin is low
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10-microsecond pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo pin (timeout of 30000 us)
  long duration = pulseIn(echoPin, HIGH, 30000);

  // --- Check for timeout (duration == 0) ---
  if (duration == 0) {
    return -1.0; // Indicate a bad reading
  }
  
  // --- Check for out-of-range (sensor max is ~400 cm) ---
  // We do a quick check in CM first just to filter bad sensor data
  float distance_cm_check = (duration * 0.0343) / 2.0;
  if (distance_cm_check > 400) {
    return -1.0; // Indicate a bad reading (too far)
  }

  // --- START: User's new calculation ---
  float distance_feet;
  distance_feet = duration * 0.034 / 2 / 30.48;
  distance_feet = 7.1 - distance_feet + 0.977;
  distance_feet = distance_feet - 0.5;
  // --- END: User's new calculation ---

  return distance_feet;
}

// --- Get Averaged Sensor Reading (Unchanged) ---
float getAverageReading(int trigPin, int echoPin, int numSamples) {
  float totalDistance = 0;
  int validReadings = 0;

  for (int i = 0; i < numSamples; i++) {
    float dist = getSensorReading(trigPin, echoPin);
    if (dist != -1.0) {
      totalDistance += dist;
      validReadings++;
    }
    delay(50); // Small delay between samples
  }

  if (validReadings > 0) {
    return totalDistance / validReadings;
  } else {
    return -1.0;
  }
}

// --- MQTT Reconnect (Client ID generation changed) ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    
    // --- Changed for ESP32 ---
    // Create a unique client ID using the ESP32's MAC address
    String clientId = "ESP32-";
    clientId += WiFi.macAddress();
    // --- End Change ---
    
    // Connect with username & password
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected ✅");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" — retrying in 5 seconds");
      delay(5000);
    }
  }
}

// --- Setup (Unchanged) ---
void setup() {
  Serial.begin(115200);

  // Initialize sensor pins
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

// --- Main Loop (Print statements MODIFIED) ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1. Get average reading from Sensor 1
  float distance1 = getAverageReading(TRIG_PIN_1, ECHO_PIN, NUM_SAMPLES);
  Serial.print("Sensor 1 Avg (GPIO 4): ");
  Serial.print(distance1);
  Serial.println(" feet"); // <-- Changed from cm

  // 2. Get average reading from Sensor 2
  float distance2 = getAverageReading(TRIG_PIN_2, ECHO_PIN, NUM_SAMPLES);
  Serial.print("Sensor 2 Avg (GPIO 5): ");
  Serial.print(distance2);
  Serial.println(" feet"); // <-- Changed from cm

  float averageDistance = -1.0;

  // 3. Calculate the average of the two sensors
  if (distance1 != -1.0 && distance2 != -1.0) {
    averageDistance = (distance1 + distance2) / 2.0;
  } else if (distance1 != -1.0) {
    averageDistance = distance1; // Only sensor 1 is working
  } else if (distance2 != -1.0) {
    averageDistance = distance2; // Only sensor 2 is working
  } else {
    Serial.println("Both sensors failed to read.");
  }

  // 4. Publish if we have a valid reading
  if (averageDistance != -1.0) {
    Serial.print("Overall Average Distance: ");
    Serial.print(averageDistance);
    Serial.println(" feet"); // <-- Changed from cm

    // Prepare message for MQTT (convert float to string)
    char msgBuffer[10];
    dtostrf(averageDistance, 4, 2, msgBuffer); 

    // 5. Publish the message
    client.publish(topic, msgBuffer);
    Serial.print("🚀 Message published to ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(msgBuffer);
  }

  // Wait 5 seconds before the next reading and publish
  delay(5000);
}