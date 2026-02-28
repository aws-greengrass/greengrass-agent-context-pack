# Sensor Simulator Component

A comprehensive Greengrass component that simulates multiple sensor types with realistic data patterns, including drift, noise, and quality indicators. Perfect for testing IoT applications without physical sensors.

## Features

- Multiple sensor types (temperature, humidity, pressure, etc.)
- Realistic data patterns with drift and noise
- Configurable intervals per sensor
- Quality indicators (good, warning, error)
- Multi-threaded sensor simulation
- File or log output modes
- Extensible sensor configurations
- Universal runtime compatibility - works on both Greengrass and Lite

## Configuration

```json
{
  "sensors": [
    {
      "id": "temp-001",
      "type": "temperature",
      "interval": 30,
      "baseValue": 22.0,
      "variance": 3.0,
      "unit": "°C"
    },
    {
      "id": "humid-001", 
      "type": "humidity",
      "interval": 45,
      "baseValue": 60.0,
      "variance": 15.0,
      "unit": "%"
    }
  ],
  "outputMode": "file",
  "outputPath": "/tmp/sensor-data.json",
  "enableDrift": true,
  "enableNoise": true
}
```

### Sensor Configuration
- `id`: Unique sensor identifier
- `type`: Sensor type (temperature, humidity, pressure, battery, etc.)
- `interval`: Reading interval in seconds
- `baseValue`: Base/nominal sensor value
- `variance`: Maximum deviation from base value
- `unit`: Measurement unit

### Global Settings
- `outputMode`: "file" or "log"
- `outputPath`: File path for sensor data (if file mode)
- `enableDrift`: Enable slow drift over time
- `enableNoise`: Enable realistic noise patterns

## Sensor Types

### Pre-configured Types
- **Temperature**: °C, realistic HVAC ranges
- **Humidity**: %, typical indoor ranges
- **Pressure**: hPa, atmospheric pressure
- **Battery**: %, device battery levels
- **Light**: lux, ambient light levels
- **Motion**: boolean, motion detection
- **Custom**: User-defined sensors

## Output Format

Each sensor reading includes:

```json
{
  "sensorId": "temp-001",
  "sensorType": "temperature",
  "value": 23.45,
  "unit": "°C",
  "timestamp": "2024-01-01T12:00:00.000Z",
  "quality": "good"
}
```

Quality indicators:
- `good`: Normal operation
- `warning`: Value approaching limits
- `error`: Value outside expected range

## Deployment Steps

### 1. Prepare Artifacts
```bash
cd examples/sensor-simulator
zip -r sensor-simulator.zip src/
```

### 2. Upload to S3
```bash
aws s3 cp sensor-simulator.zip s3://YOUR_BUCKET/sensor-simulator/1.0.0/
```

### 3. Update Recipe
Edit `recipe.json` and replace `YOUR_BUCKET` with your S3 bucket name.

### 4. Create Component
```bash
aws greengrassv2 create-component-version \
    --inline-recipe fileb://recipe.json
```

### 5. Deploy to Device
Create a deployment with this component.

## Testing Locally

Test without Greengrass:

```bash
cd src
export GG_OUTPUT_MODE="log"
export GG_OUTPUT_PATH="/tmp/test-sensors.json"
python3 main.py
```

## Usage Examples

### IoT Greenhouse Monitoring
```json
{
  "sensors": [
    {"id": "greenhouse-temp", "type": "temperature", "baseValue": 25.0, "variance": 5.0, "interval": 60},
    {"id": "greenhouse-humid", "type": "humidity", "baseValue": 70.0, "variance": 20.0, "interval": 60},
    {"id": "soil-moisture", "type": "moisture", "baseValue": 40.0, "variance": 15.0, "interval": 300}
  ]
}
```

### Industrial Equipment Monitoring
```json
{
  "sensors": [
    {"id": "motor-temp", "type": "temperature", "baseValue": 65.0, "variance": 10.0, "interval": 30},
    {"id": "vibration", "type": "vibration", "baseValue": 2.5, "variance": 1.0, "interval": 15},
    {"id": "pressure", "type": "pressure", "baseValue": 1013.25, "variance": 50.0, "interval": 45}
  ]
}
```

## Integration with Other Components

### With IPC Publisher
Modify to publish readings via IPC:
```python
# Add IPC publishing to write_reading method
self.publish_to_ipc(reading)
```

### With IoT Core Publisher
Send readings to AWS IoT Core:
```python
# Add IoT Core publishing
self.publish_to_iot_core(reading)
```

## Verification

### Check Output File
```bash
tail -f /tmp/sensor-data.json
```

### Monitor Component Logs
```bash
sudo tail -f /greengrass/v2/logs/com.example.SensorSimulator.log
```

### Analyze Data Patterns
```bash
# Count readings per sensor
cat /tmp/sensor-data.json | jq -r '.sensorId' | sort | uniq -c

# Check quality distribution
cat /tmp/sensor-data.json | jq -r '.quality' | sort | uniq -c
```

## Troubleshooting

- **No output file**: Check output path permissions and parent directory
- **Thread errors**: Verify sensor configurations are valid
- **High CPU usage**: Increase sensor intervals or reduce number of sensors
- **Memory growth**: Implement log rotation for long-running deployments
