// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Greengrass Lite Container Troubleshooting

## Common Issues

### Services Not Starting
**Symptom:** `systemctl status` shows services as "inactive (dead)"
**Solution:** 
```bash
# Create runtime directory
podman exec ggl mkdir -p /run/greengrass
podman exec ggl chown ggcore:ggcore /run/greengrass

# Enable services first
podman exec ggl systemctl enable ggl.core.*.service

# Then restart target
podman exec ggl systemctl restart greengrass-lite.target
```

### MQTT Connection Failures
**Symptom:** No MQTT traffic in logs, connection timeouts
**Solution:**
```bash
# Verify IoT policy is attached
aws iot list-attached-policies --target CERT_ARN

# Check certificate ownership
podman exec ggl ls -la /var/lib/greengrass/certs/
# Should be owned by ggcore:ggcore
```

### Certificate Permission Errors
**Symptom:** "Permission denied" errors accessing certificates
**Solution:**
```bash
podman exec ggl chown -R ggcore:ggcore /var/lib/greengrass/certs
```

### Container Build Failures
**Symptom:** Build fails with mount errors
**Solution:** Use the simplified Dockerfile that copies source instead of bind mounts

### Service Restart Loops
**Symptom:** Services keep restarting
**Solution:**
```bash
# Check logs for specific errors
podman exec ggl journalctl -u ggl.core.ggconfigd.service --no-pager -n 20

# Reset failed state
podman exec ggl systemctl reset-failed
```

## Verification Commands

```bash
# Check all services are running
podman exec ggl systemctl list-units --type=service --state=active | grep ggl

# Check MQTT connectivity
podman exec ggl journalctl -u ggl.core.iotcored.service --no-pager -n 10

# Check configuration
podman exec ggl cat /etc/greengrass/config.yaml

# Check certificates
podman exec ggl ls -la /var/lib/greengrass/certs/
```

## Cleanup Commands

```bash
# Stop and remove container
podman stop ggl && podman rm ggl

# Remove AWS resources
aws iot detach-policy --policy-name GreengrassLitePolicy --target CERT_ARN
aws iot detach-thing-principal --thing-name GreengrassLiteCore --principal CERT_ARN
aws iot update-certificate --certificate-id CERT_ID --new-status INACTIVE
aws iot delete-certificate --certificate-id CERT_ID
aws iot delete-thing --thing-name GreengrassLiteCore
aws iot delete-policy --policy-name GreengrassLitePolicy

# Remove temporary files
rm -rf /tmp/greengrass-lite-setup-*
```
