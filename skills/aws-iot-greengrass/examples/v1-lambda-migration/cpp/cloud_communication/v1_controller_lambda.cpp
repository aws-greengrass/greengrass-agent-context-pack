#include <aws/greengrass/greengrasssdk.h>
#include <iostream>
#include <string>
#include <memory>
#include <jansson.h>
#include <ctime>
#include <unistd.h>

class DeviceController {
private:
    std::unique_ptr<aws_greengrass_iot_data_client, decltype(&aws_greengrass_iot_data_client_destroy)> iot_client;
    
    static void message_callback_wrapper(const char *topic, const uint8_t *payload, 
                                        size_t payload_len, void *user_data) {
        auto* controller = static_cast<DeviceController*>(user_data);
        controller->on_cloud_command(topic, payload, payload_len);
    }
    
    std::string get_device_status() {
        // Simulate getting device status.
        return "online";
    }

public:
    DeviceController() 
        : iot_client(aws_greengrass_iot_data_client_new(), 
                     aws_greengrass_iot_data_client_destroy) {
        if (!iot_client) {
            throw std::runtime_error("Failed to create Greengrass IoT client");
        }
    }
    
    void on_cloud_command(const char *topic, const uint8_t *payload, size_t payload_len) {
        // Parse incoming command from IoT Core.
        std::string payload_str(reinterpret_cast<const char*>(payload), payload_len);
        
        json_error_t error;
        json_t *event = json_loads(payload_str.c_str(), 0, &error);
        
        if (!event) {
            std::cerr << "Error parsing JSON: " << error.text << std::endl;
            return;
        }
        
        // Extract command and device_id.
        json_t *command_obj = json_object_get(event, "command");
        json_t *device_id_obj = json_object_get(event, "device_id");
        
        const char *command = json_string_value(command_obj);
        const char *device_id = device_id_obj ? json_string_value(device_id_obj) : "device1";
        
        std::cout << "Received command from cloud: " << command << std::endl;
        
        // Process command.
        if (command && std::string(command) == "get_status") {
            std::string status = get_device_status();
            
            // Send telemetry back to IoT Core.
            json_t *telemetry_data = json_object();
            json_object_set_new(telemetry_data, "device_id", json_string(device_id));
            json_object_set_new(telemetry_data, "status", json_string(status.c_str()));
            json_object_set_new(telemetry_data, "timestamp", json_real(std::time(nullptr)));
            
            char *telemetry_payload = json_dumps(telemetry_data, JSON_COMPACT);
            
            // Publish telemetry to IoT Core.
            std::string telemetry_topic = "telemetry/" + std::string(device_id);
            
            aws_greengrass_publish_params params = {
                .topic = telemetry_topic.c_str(),
                .payload = reinterpret_cast<uint8_t*>(telemetry_payload),
                .payload_len = strlen(telemetry_payload)
            };
            
            aws_greengrass_iot_data_publish(iot_client.get(), &params);
            
            std::cout << "Telemetry sent to cloud: " << telemetry_payload << std::endl;
            
            free(telemetry_payload);
            json_decref(telemetry_data);
        }
        
        json_decref(event);
    }
    
    void subscribe_to_topic(const std::string& topic) {
        aws_greengrass_subscribe_params subscribe_params = {
            .topic = topic.c_str(),
            .callback = message_callback_wrapper,
            .user_data = this
        };
        
        aws_greengrass_iot_data_subscribe(iot_client.get(), &subscribe_params);
        
        std::cout << "Device Controller Lambda started" << std::endl;
        std::cout << "Subscribed to " << topic << std::endl;
        std::cout << "Waiting for commands from IoT Core..." << std::endl;
    }
    
    void run() {
        // Keep the Lambda running.
        while (true) {
            sleep(1);
        }
    }
};

int main(int argc, char *argv[]) {
    try {
        DeviceController controller;
        controller.subscribe_to_topic("commands/device1");
        controller.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}