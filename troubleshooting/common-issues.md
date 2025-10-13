// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Greengrass Troubleshooting Guide

## Runtime-Specific Issues

### Greengrass Lite Issues
- **Limited IPC Support**: Check [supported IPC commands](../greengrass-lite.md#supported-ipc-commands)
- **Recipe Compatibility**: Only basic recipe types supported
- **Memory Constraints**: Monitor resource usage on constrained devices

### Greengrass Nucleus Issues
- **Dependency Loops**: Check service dependency graph
- **Configuration Conflicts**: Validate configuration schema
- **Resource Exhaustion**: Monitor JVM heap usage

## Common Component Issues

### Component Won't Start
```bash
# Check component logs
sudo tail -f /greengrass/v2/logs/com.example.MyComponent.log

# Verify component status
sudo /greengrass/v2/bin/greengrass-cli component list
```

### IPC Communication Failures
```bash
# Check IPC daemon status (Lite)
sudo systemctl status ggipcd

# Verify IPC permissions in component recipe
```

### Configuration Update Issues
```bash
# Check configuration service logs
sudo tail -f /greengrass/v2/logs/ggconfigd.log

# Validate configuration format
```

## Deployment Troubleshooting

### Deployment Stuck
1. Check deployment status: `aws greengrassv2 get-deployment`
2. Review component dependencies
3. Verify device connectivity
4. Check local deployment logs

### Component Installation Failures
```bash
# Check installation logs
sudo tail -f /greengrass/v2/logs/greengrass.log

# Verify artifact permissions
ls -la /greengrass/v2/packages/artifacts/
```

## Performance Issues

### Memory Usage (Lite)
```bash
# Monitor memory usage
free -h
ps aux | grep greengrass

# Optimize component memory usage
```

### CPU Usage (Nucleus)
```bash
# Monitor JVM performance
jstat -gc $(pgrep -f greengrass)

# Tune JVM parameters
```

## Network Connectivity

### IoT Core Connection Issues
```bash
# Test connectivity
aws iot describe-endpoint --endpoint-type iot:Data-ATS

# Check certificates
openssl x509 -in device.pem.crt -text -noout
```

### Local Network Issues
```bash
# Check local IPC socket
ls -la /tmp/greengrass-ipc-*

# Verify component network permissions
```

## Log Analysis

### Key Log Locations
- **Nucleus**: `/greengrass/v2/logs/greengrass.log`
- **Components**: `/greengrass/v2/logs/[component-name].log`
- **System**: `/var/log/syslog` or `/var/log/messages`

### Log Level Configuration
```json
{
  "system": {
    "rootpath": "/greengrass/v2",
    "logging": {
      "level": "DEBUG",
      "format": "TEXT"
    }
  }
}
```

## Recovery Procedures

### Component Recovery
```bash
# Restart specific component
sudo /greengrass/v2/bin/greengrass-cli component restart --names com.example.MyComponent

# Reset component to last known good state
sudo /greengrass/v2/bin/greengrass-cli deployment create --merge "com.example.MyComponent=1.0.0"
```

### System Recovery
```bash
# Restart Greengrass service
sudo systemctl restart greengrass

# Factory reset (use with caution)
sudo rm -rf /greengrass/v2/config/config.yaml
sudo systemctl restart greengrass
```
