#!/usr/bin/env python3

import json
import logging
import math
import os
import random
import sys
import threading
import time
from datetime import datetime, timezone
from pathlib import Path

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger('SensorSimulator')

class SensorSimulator:
    def __init__(self, sensor_config):
        self.config = sensor_config
        self.drift_offset = 0.0
        self.last_value = sensor_config['baseValue']
        self.start_time = time.time()
        
    def generate_reading(self):
        """Generate a realistic sensor reading with drift and noise"""
        current_time = time.time()
        
        # Base value with optional drift over time
        base = self.config['baseValue']
        if self.config.get('enableDrift', True):
            # Slow drift over time (±10% over 1 hour)
            drift_rate = 0.1 * self.config['baseValue'] / 3600  # per second
            time_elapsed = current_time - self.start_time
            self.drift_offset = math.sin(time_elapsed / 1800) * drift_rate * time_elapsed
        
        # Add variance and noise
        variance = self.config.get('variance', 1.0)
        if self.config.get('enableNoise', True):
            # Random walk for realistic sensor behavior
            change = random.gauss(0, variance * 0.1)
            self.last_value += change
            
            # Keep within reasonable bounds
            min_val = base - variance
            max_val = base + variance
            self.last_value = max(min_val, min(max_val, self.last_value))
        else:
            # Simple random value within variance
            self.last_value = base + random.uniform(-variance, variance)
        
        # Apply drift
        final_value = self.last_value + self.drift_offset
        
        # Round based on sensor type
        if self.config['type'] in ['temperature', 'pressure']:
            final_value = round(final_value, 2)
        elif self.config['type'] in ['humidity', 'battery']:
            final_value = round(final_value, 1)
        else:
            final_value = round(final_value, 2)
        
        return {
            'sensorId': self.config['id'],
            'sensorType': self.config['type'],
            'value': final_value,
            'unit': self.config.get('unit', 'units'),
            'timestamp': datetime.now(timezone.utc).isoformat(),
            'quality': self.get_quality_indicator(final_value)
        }
    
    def get_quality_indicator(self, value):
        """Determine data quality based on value ranges"""
        base = self.config['baseValue']
        variance = self.config.get('variance', 1.0)
        
        if abs(value - base) > variance * 0.8:
            return 'warning'
        elif abs(value - base) > variance * 1.2:
            return 'error'
        else:
            return 'good'

class SensorSimulatorComponent:
    def __init__(self):
        self.config = self.load_configuration()
        self.simulators = []
        self.running = True
        self.setup_simulators()
        self.setup_output()
        
    def load_configuration(self):
        """Load component configuration"""
        try:
            # Default configuration
            config = {
                "sensors": [
                    {
                        "id": "temp-001",
                        "type": "temperature", 
                        "interval": 30,
                        "baseValue": 22.0,
                        "variance": 3.0,
                        "unit": "°C"
                    }
                ],
                "outputMode": "file",
                "outputPath": "/tmp/sensor-data.json",
                "enableDrift": True,
                "enableNoise": True
            }
            
            # Override with environment variables for testing
            if os.environ.get('GG_SENSOR_CONFIG'):
                config = json.loads(os.environ.get('GG_SENSOR_CONFIG'))
            
            config["outputMode"] = os.environ.get('GG_OUTPUT_MODE', config["outputMode"])
            config["outputPath"] = os.environ.get('GG_OUTPUT_PATH', config["outputPath"])
            
            return config
        except Exception as e:
            logger.error(f"Failed to load configuration: {e}")
            raise    

    def setup_simulators(self):
        """Initialize sensor simulators"""
        for sensor_config in self.config['sensors']:
            # Add global settings to each sensor
            sensor_config['enableDrift'] = self.config.get('enableDrift', True)
            sensor_config['enableNoise'] = self.config.get('enableNoise', True)
            
            simulator = SensorSimulator(sensor_config)
            self.simulators.append(simulator)
            logger.info(f"Initialized sensor: {sensor_config['id']} ({sensor_config['type']})")
    
    def setup_output(self):
        """Setup output file/directory"""
        if self.config['outputMode'] == 'file':
            output_path = Path(self.config['outputPath'])
            output_path.parent.mkdir(parents=True, exist_ok=True)
            logger.info(f"Output configured: {output_path}")
    
    def write_reading(self, reading):
        """Write sensor reading to configured output"""
        try:
            if self.config['outputMode'] == 'file':
                # Append to JSON lines file
                with open(self.config['outputPath'], 'a') as f:
                    f.write(json.dumps(reading) + '\n')
            elif self.config['outputMode'] == 'log':
                # Log the reading
                logger.info(f"Sensor reading: {json.dumps(reading)}")
            else:
                # Default to logging
                logger.info(f"Sensor reading: {json.dumps(reading)}")
                
        except Exception as e:
            logger.error(f"Failed to write reading: {e}")
    
    def sensor_thread(self, simulator):
        """Thread function for individual sensor"""
        logger.info(f"Starting sensor thread: {simulator.config['id']}")
        
        while self.running:
            try:
                reading = simulator.generate_reading()
                self.write_reading(reading)
                
                # Log warnings for out-of-range values
                if reading['quality'] in ['warning', 'error']:
                    logger.warning(f"Sensor {reading['sensorId']} quality: {reading['quality']} (value: {reading['value']})")
                
                time.sleep(simulator.config['interval'])
                
            except Exception as e:
                logger.error(f"Error in sensor thread {simulator.config['id']}: {e}")
                time.sleep(5)  # Brief pause before retry
    
    def run(self):
        """Main component loop"""
        logger.info("Sensor Simulator component starting...")
        logger.info(f"Configuration: {json.dumps(self.config, indent=2)}")
        
        try:
            # Start a thread for each sensor
            threads = []
            for simulator in self.simulators:
                thread = threading.Thread(
                    target=self.sensor_thread,
                    args=(simulator,),
                    daemon=True
                )
                thread.start()
                threads.append(thread)
            
            # Keep main thread alive
            while self.running:
                time.sleep(1)
                
        except KeyboardInterrupt:
            logger.info("Sensor Simulator component stopping...")
            self.running = False
            
            # Wait for threads to finish
            for thread in threads:
                thread.join(timeout=5)
                
        except Exception as e:
            logger.error(f"Unexpected error: {e}")
            sys.exit(1)

if __name__ == "__main__":
    component = SensorSimulatorComponent()
    component.run()
