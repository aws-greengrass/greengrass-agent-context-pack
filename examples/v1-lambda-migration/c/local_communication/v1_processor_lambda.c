#include <aws/greengrass/greengrasssdk.h>
#include <stdio.h>
#include <string.h>
#include <jansson.h>  // For JSON parsing.

static aws_greengrass_iot_data_client *iot_client = NULL;

void on_message_received(const char *topic, const uint8_t *payload, size_t payload_len, void *user_data) {
    // Parse the incoming message.
    char *payload_str = strndup((char *)payload, payload_len);
    json_error_t error;
    json_t *event = json_loads(payload_str, 0, &error);
    free(payload_str);
    
    if (!event) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return;
    }
    
    // Receive from publisher Lambda.
    json_t *sensor_id_obj = json_object_get(event, "sensor_id");
    json_t *temperature_obj = json_object_get(event, "temperature");
    
    const char *sensor_id = json_string_value(sensor_id_obj);
    int temperature = json_integer_value(temperature_obj);
    
    printf("Received from sensor %s: %d°F\n", sensor_id, temperature);
    
    // Process: Check if temperature is high.
    if (temperature > 80) {
        // Create alert data.
        json_t *alert_data = json_object();
        json_object_set_new(alert_data, "sensor_id", json_string(sensor_id));
        json_object_set_new(alert_data, "temperature", json_integer(temperature));
        json_object_set_new(alert_data, "alert", json_string("HIGH_TEMPERATURE"));
        
        // Convert to JSON string.
        char *alert_payload = json_dumps(alert_data, JSON_COMPACT);
        
        // Publish to another Lambda using greengrasssdk.
        aws_greengrass_publish_params params = {
            .topic = "lambda/alerts",
            .payload = (uint8_t *)alert_payload,
            .payload_len = strlen(alert_payload)
        };
        
        aws_greengrass_iot_data_publish(iot_client, &params);
        
        printf("Alert sent to subscriber Lambda\n");
        
        free(alert_payload);
        json_decref(alert_data);
    }
    
    json_decref(event);
}

int main(int argc, char *argv[]) {
    // Initialize Greengrass SDK.
    iot_client = aws_greengrass_iot_data_client_new();
    
    // Subscribe to temperature sensor topic.
    aws_greengrass_subscribe_params subscribe_params = {
        .topic = "sensors/temperature",
        .callback = on_message_received,
        .user_data = NULL
    };
    
    aws_greengrass_iot_data_subscribe(iot_client, &subscribe_params);
    
    printf("Temperature Processor Lambda started\n");
    printf("Subscribed to sensors/temperature\n");
    printf("Waiting for sensor data...\n");
    
    // Keep the Lambda running.
    while (1) {
        sleep(1);
    }
    
    return 0;
}