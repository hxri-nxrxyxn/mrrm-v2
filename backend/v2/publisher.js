const mqtt = require('mqtt');

// Connection details
const brokerUrl = 'mqtts://broker.hivemq.com:8883';
const options = {
  username: 'hivemq.webclient.1760861168522',
  password: 'q0c7JF8Kfv@%e9LQ!#Bn'
};

// The topic to publish to
const topic = '/hari';

// Connect to the broker
const client = mqtt.connect(brokerUrl, options);

// Handle the 'connect' event
client.on('connect', () => {
  console.log('✅ Connected to HiveMQ broker!');

  // Publish a message every second
  setInterval(() => {
    // Generate random values between 4 and 9, fixed to 2 decimal places
    const s1 = parseFloat((Math.random() * (9 - 4) + 4).toFixed(2));
    const s2 = parseFloat((Math.random() * (9 - 4) + 4).toFixed(2));

    const payload = { s1, s2 };
    const message = JSON.stringify(payload);

    client.publish(topic, message, (err) => {
      if (err) {
        console.error('Publish error:', err);
      } else {
        console.log(`🚀 Message published to topic "${topic}": ${message}`);
      }
    });
  }, 1000);
});

// Handle errors
client.on('error', (err) => {
  console.error('Connection error:', err);
  client.end();
});