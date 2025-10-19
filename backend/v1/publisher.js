// publisher.js
const mqtt = require('mqtt');

// Connection details
const brokerUrl = 'mqtts://broker.hivemq.com:8883';
const options = {
  username: 'hivemq.webclient.1760861168522',
  password: 'q0c7JF8Kfv@%e9LQ!#Bn'
};

// The topic and message to publish
const topic = '/hari';
const message = 'hi gianna';

// Connect to the broker
const client = mqtt.connect(brokerUrl, options);

// Handle the 'connect' event
client.on('connect', () => {
  console.log('✅ Connected to HiveMQ broker!');

  // Publish the message
  client.publish(topic, message, (err) => {
    if (err) {
      console.error('Publish error:', err);
    } else {
      console.log(`🚀 Message published to topic "${topic}": "${message}"`);
    }
    
    // Close the connection after publishing
    client.end();
  });
});

// Handle errors
client.on('error', (err) => {
  console.error('Connection error:', err);
  client.end();
});
