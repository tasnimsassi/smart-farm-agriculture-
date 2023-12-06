#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"




const char* ssid = "tasnim";
const char* password = "tasnouma";
const char* firebaseHost = "smartfarm-ec140-default-rtdb.firebaseio.com";
const char* firebaseAuth = "AIzaSyC7ZPYNSLp9SuEtQI-Mucxiq0MnMkG2q3g";



#define DHTPIN 4     // Pin to which the DHT11 data pin is connected
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define LIGHT_SENSOR_PIN  36
#define LED_PIN           22
#define LED_PIN2          23
#define ANALOG_THRESHOLD  100
#define GAS_SENSOR_PIN    4
#define MOTION_SENSOR_PIN  32
#define BUZZER_PIN         17
#define POMPE_PIN 25 // Utilisation de la broche D25 de l'ESP32 pour la pompe


//bool ledState = false; // Initial state of LEDs
int Sensor_input = 4;    /*Digital pin 5 for sensor input*/
int motionStatePrevious = LOW; // État précédent du capteur de mouvement



const int sensor_pin = 39;  /* Soil moisture sensor O/P pin */
int _moisture, sensor_analog;

void setup() {
    Serial.begin(115200);
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    pinMode(LED_PIN, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);

    dht.begin();

    pinMode(MOTION_SENSOR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    //digitalWrite(BUZZER_PIN, HIGH);
    pinMode(POMPE_PIN, OUTPUT); // Définir la broche comme sortie
    digitalWrite(POMPE_PIN, LOW);
   // bool ledState = false;
}

void loop() {
    // Read sensor values
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor");
        delay(2000);
        return;
    }

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" % - Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // Send sensor values to Firebase
    sendDHTDataToFirebase(humidity, temperature);

    // Read moisture sensor
    sensor_analog = analogRead(sensor_pin);
    _moisture = (100 - ((sensor_analog / 4095.00) * 100));

    Serial.print("Moisture = ");
    Serial.print(_moisture);
    Serial.println("%");

    // Send moisture data to Firebase
    sendMoistureToFirebase(_moisture);

    // Read light and gas sensors
    int analogValue = analogRead(LIGHT_SENSOR_PIN);
    Serial.print("Light value: ");
    Serial.println(analogValue);
    sendLightSensorToFirebase(analogValue);
    

    int gasValue = analogRead(GAS_SENSOR_PIN); // Read gas sensor value
    Serial.print("Gas value: ");
    Serial.println(gasValue);
    Serial.print("\t");
    Serial.print("\t");
    sendGasToFirebase(gasValue);     // Send gas value to Firebase
   // if ((analogValue < ANALOG_THRESHOLD)|| (ledState == true)) {
        //digitalWrite(LED_PIN, HIGH);
        //digitalWrite(LED_PIN2, HIGH);
     //   ledState = true;
      //} else if((analogValue > ANALOG_THRESHOLD)|| (ledState == false) ) {
        //digitalWrite(LED_PIN, LOW);
        //digitalWrite(LED_PIN2, LOW);
        //ledState = false;
    //}
      //sendLedStateToFirebase(ledState);
    if (gasValue > 100) {
        Serial.println("Gas");
    } else {
        Serial.println("No Gas");
    }
    //capteur mouvement
     int motionStateCurrent = digitalRead(MOTION_SENSOR_PIN); // Lecture du nouvel état

    if (motionStatePrevious == LOW && motionStateCurrent == HIGH) { // Changement d'état : LOW -> HIGH
        Serial.println("Mouvement détecté !");
        digitalWrite(BUZZER_PIN, LOW);
        delay(3000);
        digitalWrite(BUZZER_PIN, HIGH);
        // Envoi de l'état du mouvement détecté à Firebase (true)
        sendMotionStateToFirebase(true);
    } else {
        // Si aucun mouvement n'est détecté
        Serial.println("PAS DE Mouvement  !");
        digitalWrite(BUZZER_PIN, HIGH);
        // Envoi de l'état "false" à Firebase seulement si l'état précédent était "true"
        if (motionStatePrevious == HIGH) {
            sendMotionStateToFirebase(false);
        }
    }

    motionStatePrevious = motionStateCurrent; // Mise à jour de l'état précédent
    
    // controle led
    // Récupérer l'état actuel des LED depuis Firebase
    bool firebaseLedState = getLedStateFromFirebase();
       Serial.println(firebaseLedState);
    // Contrôler les LED en fonction de l'état récupéré depuis Firebase
    if (firebaseLedState) {
        digitalWrite(LED_PIN, HIGH); // Allumer la LED
        digitalWrite(LED_PIN2, HIGH); 
        Serial.println("leds allumeee");
    } else {
        digitalWrite(LED_PIN, LOW); // Éteindre la LED
          digitalWrite(LED_PIN2, LOW); // Éteindre la LED
        Serial.println("leds noon" );
    }
   
    // Récupérer la valeur d'arrosage depuis Firebase
    // Check and update pump status
    bool pumpStatus = checkArrosageAndUpdatePump();
    bool wateringStatus = checkWateringStatusFromFirebase();
    Serial.println(pumpStatus);  
    Serial.println(wateringStatus);  
    if (pumpStatus || wateringStatus ) {
        digitalWrite(POMPE_PIN, LOW); // Turn on the pump
        Serial.println("Pump Status Set: on");
       
       // digitalWrite(POMPE_PIN, HIGH); // Turn on the pump
    } else {
       digitalWrite(POMPE_PIN, HIGH
       
       ); // Turn off the pump
       Serial.println("Pump Status Set: off");
       
    }
    //controlPump(pumpStatus);


    delay(2000); // Adjust this delay according to your needs
}

void sendDHTDataToFirebase(float humidity, float temperature) {
    // Construct the JSON payload for DHT data to Firebase
    String humidityStr = String(humidity);
    String temperatureStr = String(temperature);

    // Construct the JSON payload for DHT data to Firebase
    String jsonPayload = "{\"humidity\":\"" + humidityStr + "\",\"temperature\":\"" + temperatureStr + "\"}";
    

    // Construct the URL for DHT data
    String url = "https://" + String(firebaseHost) + "/sensorData/humidity.json?auth=" + String(firebaseAuth);

    // Send HTTP PUT request to update the DHT values in Firebase
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.PUT(jsonPayload);

    Serial.print("HTTP Response code (DHT data): ");
    Serial.println(httpCode);

    http.end();
}

void sendMoistureToFirebase(int moisture) {

    String moistureStr = String(moisture);

    // Construct the JSON payload for moisture data to Firebase
    String jsonPayload = "{\"pluie\":\"" + moistureStr + "\"}";

    // Construct the URL for moisture data
    String url = "https://" + String(firebaseHost) + "/moistureData/pluie.json?auth=" + String(firebaseAuth);

    // Send HTTP PUT request to update the moisture value in Firebase
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.PUT(jsonPayload);

    Serial.print("HTTP Response code (Moisture data): ");
    Serial.println(httpCode);

    http.end();
}
void sendLightSensorToFirebase(int analogValue) {
   String analogValueStr = String(analogValue); // Convertir la valeur en chaîne de caractères
    String jsonPayload = "{\"lightValue\":\"" + analogValueStr + "\"}"; // Insérer la chaîne de caractères dans le JSON
   String url = "https://" + String(firebaseHost) + "/lightData/lightValue.json?auth=" + String(firebaseAuth);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(jsonPayload);

  Serial.print("HTTP Response code (Light data): ");
  Serial.println(httpCode);

  http.end();
}

void sendGasToFirebase(int gasValue) {
  String gasValueStr = String(gasValue);
  String gasStatus = gasValue > 1800 ? "Gas Detected" : "No Gas";

  String jsonPayload = "{\"gasData\": {\"gasStatus\":\"" + gasStatus + "\",\"gasValue\":\"" + gasValueStr + "\"}}";
  String url = "https://" + String(firebaseHost) + "/gasData.json?auth=" + String(firebaseAuth);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(jsonPayload);

  Serial.print("HTTP Response code (Gas data): ");
  Serial.println(httpCode);

  http.end();
}


void sendLedStateToFirebase(bool state) {
  String jsonPayload = "{\"ledState\":" + String(state ? "true" : "false") + "}";
  String url = "https://" + String(firebaseHost) + "/ledData.json?auth=" + String(firebaseAuth);

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.PUT(jsonPayload);

  Serial.print("HTTP Response code (LED data): ");
  Serial.println(httpCode);

  http.end();
}

void sendMotionStateToFirebase(bool motionDetected) {
    // Construction des données à envoyer à Firebase
    String jsonPayload = "{\"motionDetected\":" + String(motionDetected ? "true" : "false") + "}";

    // Construction de l'URL pour les données de mouvement
    String url = "https://" + String(firebaseHost) + "/motionData.json?auth=" + String(firebaseAuth);

    // Envoi de la requête HTTP PUT pour mettre à jour l'état du mouvement dans Firebase
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.PUT(jsonPayload);

    Serial.print("HTTP Response code (Motion data): ");
    Serial.println(httpCode);

    http.end();
}

bool checkArrosageAndUpdatePump() {
    String url = "https://" + String(firebaseHost) + "/pumpStatus.json?auth=" + String(firebaseAuth);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString(); // Lire la réponse de Firebase
        Serial.println("Firebase response for pump status: " + response); // Afficher la réponse

        bool pumpStatus = response == "true"; // Convertir la réponse en booléen

        Serial.print("Pump Status from Firebase: ");
        Serial.println(pumpStatus ? "true" : "false");

        http.end(); // Terminer la requête HTTP
        return pumpStatus;
    } else {
        Serial.println("Failed to get pump status from Firebase");
        http.end(); // Terminer la requête HTTP même si elle échoue
        return false;
    }
}


bool getLedStateFromFirebase() {
    String url = "https://" + String(firebaseHost) + "/ledData/ledState.json?auth=" + String(firebaseAuth);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String ledStatus = http.getString(); // Lire l'état de la LED depuis Firebase
        bool state = ledStatus == "true"; // Convertir la chaîne en booléen

        Serial.print("LED State from Firebase: ");
        Serial.println(state ? "ON" : "OFF");

        http.end(); // Terminer la requête HTTP
        return state;
    } else {
        Serial.println("Failed to get LED state from Firebase");
        http.end(); // Terminer la requête HTTP même si elle échoue
        return false;
    }
}

bool checkWateringStatusFromFirebase() {
    String url = "https://" + String(firebaseHost) + "/wateringStatus.json?auth=" + String(firebaseAuth);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString(); // Lire la réponse de Firebase
        Serial.println("Firebase response for watering status: " + response); // Afficher la réponse

        bool wateringStatus = response == "true"; // Convertir la chaîne en booléen

        Serial.print("Watering Status from Firebase: ");
        Serial.println(wateringStatus ? "true" : "false");

        http.end(); // Terminer la requête HTTP
        return wateringStatus;
    } else {
        Serial.println("Failed to get watering status from Firebase");
        http.end(); // Terminer la requête HTTP même si elle échoue
        return false;
    }
}
