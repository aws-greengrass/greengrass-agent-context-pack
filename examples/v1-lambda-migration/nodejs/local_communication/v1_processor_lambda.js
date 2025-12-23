const greengrasssdk = require('aws-greengrass-core-sdk');

const iotClient = new greengrasssdk.IotData();

/**
 * Greengrass v1 Lambda function
 * Receives temperature from publisher Lambda,
 * processes it, and forwards to subscriber Lambda
 */
exports.handler = function(event, context) {
    // Receive from publisher Lambda.
    const sensorId = event.sensor_id;
    const temperature = event.temperature;
    
    console.log(`Received from sensor ${sensorId}: ${temperature}°F`);
    
    // Process: Check if temperature is high.
    if (temperature > 80) {
        const alertData = {
            sensor_id: sensorId,
            temperature: temperature,
            alert: 'HIGH_TEMPERATURE'
        };
        
        // Publish to another Lambda using greengrasssdk.
        const params = {
            topic: 'lambda/alerts',
            payload: JSON.stringify(alertData)
        };
        
        iotClient.publish(params, (err) => {
            if (err) {
                console.error('Error publishing alert:', err);
                context.fail(err);
            } else {
                console.log('Alert sent to subscriber Lambda');
                context.succeed('Success');
            }
        });
    } else {
        context.succeed('Success');
    }
};