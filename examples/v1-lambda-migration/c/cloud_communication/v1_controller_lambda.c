#include <aws/greengrass/greengrasssdk.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include <time.h>

static aws_greengrass_iot_data_client *iot_client = NULL;

const char* get_device_status(void) {
    // Simulate getting device status.
    return "online";
}

void on_cloud_command(const char *topic, const uint8_t *payload, size_t payload_len, void *user_data) {
    // Parse incoming command from IoT Core.
    char *payload_str = strndup((char *)payload, payload_len);
    json_error_t error;
    json_t *event = json_loads(payload_str, 0, &error);
    free(payload_str);
    
    if (!event) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return;
    }
    
    // Extract command and device_id.
    json_t *command_obj = json_object_get(event, "command");
    json_t *device_id_obj = json_object_get(event, "device_id");
    
    const char *command = json_string_value(command_obj);
    const char *device_id = device_id_obj ? json_string_value(device_id_obj) : "device1";
    
    printf("Received command from cloud: %s\n", command);
    
    // Process command.
    if (command && strcmp(command, "get_status") == 0) {
        const char *status = get_device_status();
        
        // Send telemetry back to IoT Core.
        json_t *telemetry_data = json_object();
        json_object_set_new(telemetry_data, "device_id", json_string(device_id));
        json_object_set_new(telemetry_data, "status", json_string(status));
        json_object_set_new(telemetry_data, "timestamp", json_real(time(NULL)));
        
        char *telemetry_payload = json_dumps(telemetry_data, JSON_COMPACT);
        
        // Publish telemetry to IoT Core.
        char telemetry_topic[256];
        snprintf(telemetry_topic, sizeof(telemetry_topic), "telemetry/%s", device_id);
        
        aws_greengrass_publish_params params = {
            .topic = telemetry_topic,
            .payload = (uint8_t *)telemetry_payload,
            .payload_len = strlen(telemetry_payload)
        };
        
        aws_greengrass_iot_data_publish(iot_client, &params);
        
        printf("Telemetry sent to cloud: %s\n", telemetry_payload);
    
        free(telemetry_payload);
        json_decref(telemetry_data);
    }
    
    json_decref(event);
}

int main(int argc, char *argv[]) {
    // Initialize Greengrass SDK.
    iot_client = aws_greengrass_iot_data_client_new();
    
    // Subscribe to commands from IoT Core.
    aws_greengrass_subscribe_params subscribe_params = {
        .topic = "commands/device1",
        .callback = on_cloud_command,
        .user_data = NULL
    };
    
    aws_greengrass_iot_data_subscribe(iot_client, &subscribe_params);
    
    printf("Device Controller Lambda started\n");
    printf("Subscribed to commands/device1\n");
    printf("Waiting for commands from IoT Core...\n");
    
    // Keep the Lambda running.
    while (1) {
        sleep(1);
    }
    
    return 0;
}