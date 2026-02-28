const greengrasscoreipc = require('aws-iot-device-sdk-v2').greengrasscoreipc;

class DeviceController {
    constructor() {
        this.ipcClient = null;
    }

    async start() {
        console.log('Device Controller component starting...');
        
        try {
            this.ipcClient = greengrasscoreipc.createClient();
            await this.ipcClient.connect();
            
            const subscribeRequest = {
                topicName: 'commands/device1',
                qos: 1
            };

            const streamingOperation = this.ipcClient.subscribeToIoTCore(subscribeRequest);
            
            streamingOperation.on('message', (message) => {
                this.onCommand(message);
            });
            
            streamingOperation.on('streamError', (error) => {
                console.error('Stream error:', error);
            });
            
            streamingOperation.on('ended', () => {
                console.log('Subscription stream ended');
            });
            
            await streamingOperation.activate();
            
            console.log('Subscribed to commands/device1 from IoT Core');
            console.log('Waiting for commands from cloud...');
            
        } catch (error) {
            console.error('Error starting component:', error);
            process.exit(1);
        }
    }

    async onCommand(message) {
        try {
            const payload = message.message.payload.toString('utf-8');
            const data = JSON.parse(payload);
            
            const command = data.command;
            const deviceId = data.device_id || 'device1';
            
            console.log(`Received command from cloud: ${command}`);
            
            if (command === 'get_status') {
                const status = this.getDeviceStatus();
                
                const telemetryData = {
                    device_id: deviceId,
                    status: status,
                    timestamp: Date.now() / 1000
                };
                
                const telemetryJson = JSON.stringify(telemetryData);
                
                const publishRequest = {
                    topicName: `telemetry/${deviceId}`,
                    qos: 1,
                    payload: Buffer.from(telemetryJson, 'utf-8')
                };
                
                await this.ipcClient.publishToIoTCore(publishRequest);
                console.log(`Telemetry sent to cloud: ${telemetryJson}`);
            }
            
        } catch (error) {
            console.error('Error processing command:', error);
        }
    }

    getDeviceStatus() {
        return 'online';
    }
}

// Start the component.
const controller = new DeviceController();
controller.start();