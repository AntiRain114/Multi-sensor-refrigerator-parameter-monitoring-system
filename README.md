# Multi-sensor-refrigerator-parameter-monitoring-system

## Overview
This project implements an advanced multi-sensor system for monitoring refrigerator conditions using IoT technology. It employs HDC1080 temperature and humidity sensors, LoRaWAN communication, and cloud-based data processing and visualization.

## Hardware Components
- Arduino MKR WAN 1310 processor board
- Three HDC1080 temperature and humidity sensors
- Photoresistor for door status detection
- Lithium battery for power supply
- LoRaWAN communication module

## Software Components
- Node.js script for cloud integration
- MQTT for data transmission
- InfluxDB for data storage
- Grafana for data visualization

## System Architecture
1. **Data Acquisition**: HDC1080 sensors collect temperature and humidity data at predetermined intervals.
2. **LoRaWAN Transmission**: Data is transmitted to The Things Network (TTN).
3. **Cloud Processing**: A Node.js script receives data from TTN via MQTT.
4. **Data Storage**: Processed data is stored in InfluxDB.
5. **Visualization**: Grafana provides real-time dashboards and analysis.

## Installation
1. Set up the hardware components in the refrigerator according to the 3D grid pattern specified in the methodology.
2. Configure the Arduino MKR WAN 1310 with the provided code (link to code file).
3. Set up a TTN account and configure your device.
4. Deploy the Node.js script on your cloud server (link to script).
5. Install and configure InfluxDB (instructions).
6. Set up Grafana and import the provided dashboard (link to dashboard configuration).

## Usage
- The system automatically collects and transmits data at optimized intervals.
- Access the Grafana dashboard for real-time monitoring and analysis.
- Adjust transmission intervals and sensor configurations as needed for your specific use case.

## Data Security
- Data transmission is encrypted.
- Implement appropriate security measures for your cloud server and database.

## Contributing
We welcome contributions to this project. Please read our contributing guidelines (link) before submitting pull requests.

## License
This project is licensed under the MIT License - see the LICENSE.md file for details.

## Acknowledgments
- Texas Instruments for the HDC1080 sensor
- The Things Network for LoRaWAN infrastructure
- (Any other acknowledgments)
