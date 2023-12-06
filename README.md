# smart-farm-agriculture-


This project involves an IoT system built using an ESP32 microcontroller to monitor various environmental conditions in a farm. It utilizes sensors to measure temperature, humidity, soil moisture, light intensity, gas levels, and motion detection. Additionally, it controls a pump to water the crops based on sensor readings.

## Overview

The system employs an ESP32 microcontroller and various sensors:

- DHT11 sensor for temperature and humidity readings
- Soil moisture sensor to monitor soil moisture levels
- Light sensor to measure ambient light intensity
- Gas sensor to detect gas levels
- Motion sensor for motion detection

It interacts with a Firebase database for data storage and retrieval. The ESP32 communicates with the Firebase Realtime Database to update sensor readings and control the pump based on preset conditions.

## Features

- Read temperature, humidity, soil moisture, light intensity, gas levels, and motion detection data.
- Send sensor data to Firebase for storage and analysis.
- Control a pump for watering based on soil moisture readings.
- Monitor gas levels and trigger alerts when necessary.

## Setup

1. Clone this repository.
2. Set up your Arduino IDE with ESP32 board support.
3. Install necessary libraries like Firebase, DHT, and ArduinoJson.
4. Update the `ssid`, `password`, `firebaseHost`, and `firebaseAuth` variables in the code with your network credentials and Firebase details.

## Usage

1. Flash the ESP32 with the provided code.
2. Ensure proper connections of sensors and the pump to the ESP32.
3. Access Firebase to monitor sensor readings and pump control remotely.

## Circuit Diagram

Insert a circuit diagram or a Fritzing schematic to illustrate the connections between the ESP32, sensors, and pump.

## Contributing

Contributions to enhance functionality or fix issues are welcome! Feel free to fork the repository and create a pull request.

## License

This project is licensed under [MIT License](LICENSE).
