#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "FTTH-2ACD";
const char* password = "9495312626";

// HiveMQ broker details
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_username = "hivemq.webclient.1760861168522";
const char* mqtt_password = "q0c7JF8Kfv@%e9LQ!#Bn";
const char* topic = "/hari";

// --- Sensor Definitions ---
// Sensor 1
#define TRIG_PIN_1 4    // GPIO 4
#define ECHO_PIN_1 16   // GPIO 16 (RX2)

// Sensor 2
#define TRIG_PIN_2 5    // GPIO 5
#define ECHO_PIN_2 17   // GPIO 17 (TX2)

#define NUM_SAMPLES 5   // Number of samples to average

WiFiClient espClient;
PubSubClient client(espClient);

// --- WiFi Setup ---
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

// --- Get Single Sensor Reading ---
float getSensorReading(int trigPin, int echoPin) {
  // Ensure trigger pin is low
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10-microsecond pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo pin (timeout of 30000 us = ~5 meters)
  long duration = pulseIn(echoPin, HIGH, 30000);

  // --- Check for timeout ---
  if (duration == 0) {
    return -1.0; 
  }
  
  // --- Check for out-of-range (>400cm) ---
  float distance_cm_check = (duration * 0.0343) / 2.0;
  if (distance_cm_check > 400) {
    return -1.0; 
  }

  // --- START: User's calculation ---
  float distance_feet;
  distance_feet = duration * 0.034 / 2 / 30.48;
  distance_feet = 7.1 - distance_feet + 0.977;
  distance_feet = distance_feet - 0.5;
  // --- END: User's calculation ---

  return distance_feet;
}

// --- Get Averaged Sensor Reading ---
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

// --- MQTT Reconnect ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    
    // Create unique Client ID
    String clientId = "ESP32-";
    clientId += WiFi.macAddress();
    
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

// --- Setup ---
void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT); // Sensor 1 Echo

  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT); // Sensor 2 Echo

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

// --- Main Loop ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1. Get average reading from Sensor 1 (Using ECHO_PIN_1)
  float distance1 = getAverageReading(TRIG_PIN_1, ECHO_PIN_1, NUM_SAMPLES);
  Serial.print("Sensor 1 (D4/D16): ");
  Serial.print(distance1);
  Serial.println(" ft");

  // 2. Get average reading from Sensor 2 (Using ECHO_PIN_2)
  float distance2 = getAverageReading(TRIG_PIN_2, ECHO_PIN_2, NUM_SAMPLES);
  Serial.print("Sensor 2 (D5/D17): ");
  Serial.print(distance2);
  Serial.println(" ft");

  float averageDistance = -1.0;

  // 3. Logic to average them
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
    Serial.print("Overall Average: ");
    Serial.print(averageDistance);
    Serial.println(" ft");

    char msgBuffer[10];
    dtostrf(averageDistance, 4, 2, msgBuffer); 

    client.publish(topic, msgBuffer);
    Serial.print("🚀 Published to ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(msgBuffer);
  }

  delay(5000);
}