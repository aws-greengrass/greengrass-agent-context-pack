const greengrasscoreipc = require('aws-iot-device-sdk-v2').greengrasscoreipc;

class TemperatureProcessor {
    constructor() {
        this.ipcClient = null;
    }

    async start() {
        console.log('Temperature Processor component starting...');
        
        try {
            this.ipcClient = greengrasscoreipc.createClient();
            await this.ipcClient.connect();
            
            const subscribeRequest = {
                topic: 'sensors/temperature'
            };

            const streamingOperation = this.ipcClient.subscribeToTopic(subscribeRequest);
            
            streamingOperation.on('message', (message) => {
                this.onSensorData(message);
            });
            
            streamingOperation.on('streamError', (error) => {
                console.error('Stream error:', error);
            });
            
            streamingOperation.on('ended', () => {
                console.log('Subscription stream ended');
            });
            
            await streamingOperation.activate();
            
            console.log('Subscribed to sensors/temperature');
            console.log('Waiting for sensor data...');
            
        } catch (error) {
            console.error('Error starting component:', error);
            process.exit(1);
        }
    }

    async onSensorData(message) {
        try {
            const data = message.jsonMessage.message;
            
            const sensorId = data.sensor_id;
            const temperature = data.temperature;
            
            console.log(`Received from sensor ${sensorId}: ${temperature}°F`);
            
            if (temperature > 80) {
                const alertData = {
                    sensor_id: sensorId,
                    temperature: temperature,
                    alert: 'HIGH_TEMPERATURE'
                };
                
                const publishRequest = {
                    topic: 'component/alerts',
                    publishMessage: {
                        jsonMessage: {
                            message: alertData
                        }
                    }
                };
                
                await this.ipcClient.publishToTopic(publishRequest);
                console.log('Alert sent to AlertHandler component');
            }
            
        } catch (error) {
            console.error('Error processing sensor data:', error);
        }
    }
}

// Start the component.
const processor = new TemperatureProcessor();
processor.start();