#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/sdk.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define COMMAND_TOPIC "commands/device1"
#define TELEMETRY_TOPIC "telemetry/device1"

typedef struct {
    char device_id[64];
    char command[64];
} CommandData;

static pthread_mutex_t command_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t command_cond = PTHREAD_COND_INITIALIZER;
static CommandData pending_command;
static bool has_pending_command = false;

const char* get_device_status(void) {
    // Simulate getting device status.
    return "online";
}

static void *telemetry_publisher_thread(void *arg) {
    (void) arg;
    
    while (true) {
        pthread_mutex_lock(&command_mutex);
        while (!has_pending_command) {
            pthread_cond_wait(&command_cond, &command_mutex);
        }
        
        CommandData cmd = pending_command;
        has_pending_command = false;
        pthread_mutex_unlock(&command_mutex);
        
        // Process command.
        if (strcmp(cmd.command, "get_status") == 0) {
            const char *status = get_device_status();
            
            // Create telemetry JSON string.
            char telemetry_json[512];
            snprintf(telemetry_json, sizeof(telemetry_json),
                     "{\"device_id\":\"%s\",\"status\":\"%s\",\"timestamp\":%ld}",
                     cmd.device_id, status, time(NULL));
            
            GgBuffer telemetry_buf = {
                .data = (uint8_t *)telemetry_json,
                .len = strlen(telemetry_json)
            };
            
            // Publish telemetry to IoT Core.
            GgError ret = ggipc_publish_to_iot_core(GG_STR(TELEMETRY_TOPIC), telemetry_buf, 0);
            
            if (ret != GG_ERR_OK) {
                fprintf(stderr, "Failed to publish telemetry to IoT Core\n");
            } else {
                printf("Telemetry sent to cloud: device_id=%s, status=%s\n", cmd.device_id, status);
            }
        }
    }
    
    return NULL;
}

static void on_cloud_command(
    void *ctx, GgBuffer topic, GgBuffer payload, GgIpcSubscriptionHandle handle
) {
    (void) ctx;
    (void) topic;
    (void) handle;
    
    printf("Received command from IoT Core: %.*s\n", (int)payload.len, payload.data);
    
    // Parse JSON payload (comes as raw buffer from IoT Core).
    // For simplicity, we'll do basic string parsing.
    
    // Extract command and device_id from JSON string.
    char payload_str[512];
    snprintf(payload_str, sizeof(payload_str), "%.*s", (int)payload.len, payload.data);
    
    // Simple JSON parsing (looking for "command":"get_status").
    char *command_start = strstr(payload_str, "\"command\"");
    char *device_id_start = strstr(payload_str, "\"device_id\"");
    
    if (command_start) {
        pthread_mutex_lock(&command_mutex);
        
        char *cmd_value = strstr(command_start, ":");
        if (cmd_value) {
            cmd_value = strchr(cmd_value, '"');
            if (cmd_value) {
                cmd_value++;
                char *cmd_end = strchr(cmd_value, '"');
                if (cmd_end) {
                    size_t cmd_len = cmd_end - cmd_value;
                    if (cmd_len < sizeof(pending_command.command)) {
                        strncpy(pending_command.command, cmd_value, cmd_len);
                        pending_command.command[cmd_len] = '\0';
                    }
                }
            }
        }
        
        // Extract device_id or use default.
        if (device_id_start) {
            char *dev_value = strstr(device_id_start, ":");
            if (dev_value) {
                dev_value = strchr(dev_value, '"');
                if (dev_value) {
                    dev_value++;
                    char *dev_end = strchr(dev_value, '"');
                    if (dev_end) {
                        size_t dev_len = dev_end - dev_value;
                        if (dev_len < sizeof(pending_command.device_id)) {
                            strncpy(pending_command.device_id, dev_value, dev_len);
                            pending_command.device_id[dev_len] = '\0';
                        }
                    }
                }
            }
        } else {
            strcpy(pending_command.device_id, "device1");
        }
        
        printf("Received command from cloud: %s\n", pending_command.command);
        
        has_pending_command = true;
        pthread_cond_signal(&command_cond);
        pthread_mutex_unlock(&command_mutex);
    }
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Device Controller component starting...\n");
    
    gg_sdk_init();
    
    GgError ret = ggipc_connect();
    if (ret != GG_ERR_OK) {
        fprintf(stderr, "Failed to connect to Greengrass nucleus\n");
        exit(1);
    }
    printf("Connected to Greengrass IPC\n");
    
    // Start telemetry publisher thread.
    pthread_t telemetry_thread;
    if (pthread_create(&telemetry_thread, NULL, telemetry_publisher_thread, NULL) != 0) {
        fprintf(stderr, "Failed to create telemetry publisher thread\n");
        exit(1);
    }
    
    // Subscribe to commands from IoT Core.
    GgIpcSubscriptionHandle handle;
    ret = ggipc_subscribe_to_iot_core(
        GG_STR(COMMAND_TOPIC),
        0,
        &on_cloud_command,
        NULL,
        &handle
    );
    
    if (ret != GG_ERR_OK) {
        fprintf(stderr, "Failed to subscribe to IoT Core topic\n");
        exit(1);
    }
    
    printf("Subscribed to %s\n", COMMAND_TOPIC);
    printf("Waiting for commands from IoT Core...\n");
    
    while (true) {
        sleep(1);
    }
    
    return 0;
}