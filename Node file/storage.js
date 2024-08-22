const mqtt = require('mqtt');
const axios = require('axios');

const appId = '';
const accessKey = '';
const mqttBroker = '';
const tenantId = '';
const deviceId = '';

// InfluxDB configuration
const influxdbUrl = 'http://localhost:8086';
const influxdbToken = '';
const influxdbOrg = '';
const influxdbBucket = '';

// Connect to MQTT server
const mqttClient = mqtt.connect(mqttBroker, {
  username: '',
  password: ''
});

mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  // Subscribe to MQTT topic
  mqttClient.subscribe(`v3/${appId}@${tenantId}/devices/${deviceId}/up`);
});

mqttClient.on('message', async (topic, message) => {
  console.log(`Received message from topic: ${topic}`);
  try {
    // Parse the received JSON data
    const data = JSON.parse(message.toString());
    // Extract sensor data
    const sensorData = data.uplink_message.decoded_payload;

    // Build data points to be written to InfluxDB
    const points = [];
    if (sensorData.temperature_1 !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} temperature_1=${sensorData.temperature_1}`);
    if (sensorData.temperature_2 !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} temperature_2=${sensorData.temperature_2}`);
    if (sensorData.temperature_3 !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} temperature_3=${sensorData.temperature_3}`);
    if (sensorData.humidity !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} humidity=${sensorData.humidity}`);
    if (sensorData.light_cycle_duration !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} light_cycle_duration=${sensorData.light_cycle_duration}`);
    if (sensorData.temp_change_1 !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} temp_change_1=${sensorData.temp_change_1}`);
    if (sensorData.temp_change_2 !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} temp_change_2=${sensorData.temp_change_2}`);
    if (sensorData.temp_change_3 !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} temp_change_3=${sensorData.temp_change_3}`);
    if (sensorData.humidity_change !== undefined) points.push(`sensor,device_id=${data.end_device_ids.device_id} humidity_change=${sensorData.humidity_change}`);

    // Write data using InfluxDB API
    const response = await axios.post(`${influxdbUrl}/api/v2/write`, points.join('\n'), {
      params: {
        org: influxdbOrg,
        bucket: influxdbBucket
      },
      headers: {
        'Authorization': `Token ${influxdbToken}`,
        'Content-Type': 'text/plain; charset=utf-8',
        'Accept': 'application/json'
      }
    });

    console.log('Sensor data written to InfluxDB successfully');
    // Print sent data for debugging
    console.log('Sent data:', points);
  } catch (error) {
    console.error('Error writing sensor data to InfluxDB:', error.message);
    // Print the error response from InfluxDB for more details
    if (error.response) {
      console.error('Response data:', error.response.data);
    }
  }
});

// Error handling for MQTT client
mqttClient.on('error', (error) => {
  console.error('MQTT client error:', error);
});

// Error handling for unexpected errors
process.on('unhandledRejection', (reason, promise) => {
  console.error('Unhandled Rejection at:', promise, 'reason:', reason);
});