#include <aws/greengrass/greengrasssdk.h>
#include <iostream>
#include <string>
#include <memory>
#include <jansson.h> // For JSON parsing.
#include <unistd.h>

class TemperatureProcessor {
private:
    std::unique_ptr<aws_greengrass_iot_data_client, 
                    decltype(&aws_greengrass_iot_data_client_destroy)> iot_client;
    
    static void message_callback_wrapper(const char *topic, 
                                        const uint8_t *payload, 
                                        size_t payload_len, 
                                        void *user_data) {
        auto* processor = static_cast<TemperatureProcessor*>(user_data);
        processor->on_message_received(topic, payload, payload_len);
    }

public:
    TemperatureProcessor() 
        : iot_client(aws_greengrass_iot_data_client_new(), 
                     aws_greengrass_iot_data_client_destroy) {
        if (!iot_client) {
            throw std::runtime_error("Failed to create Greengrass IoT client");
        }
    }

    void on_message_received(const char *topic, 
                            const uint8_t *payload, 
                            size_t payload_len) {
        // Parse the incoming message.
        std::string payload_str(reinterpret_cast<const char*>(payload), payload_len);
        
        json_error_t error;
        json_t *event = json_loads(payload_str.c_str(), 0, &error);
        
        if (!event) {
            std::cerr << "Error parsing JSON: " << error.text << std::endl;
            return;
        }

        json_t *sensor_id_obj = json_object_get(event, "sensor_id");
        json_t *temperature_obj = json_object_get(event, "temperature");
        
        const char *sensor_id = json_string_value(sensor_id_obj);
        int temperature = json_integer_value(temperature_obj);
        
        std::cout << "Received from sensor " << sensor_id 
                  << ": " << temperature << "°F" << std::endl;

        if (temperature > 80) {
            send_alert(sensor_id, temperature);
        }

        json_decref(event);
    }

    void send_alert(const char *sensor_id, int temperature) {
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
            .payload = reinterpret_cast<uint8_t*>(alert_payload),
            .payload_len = strlen(alert_payload)
        };
        
        aws_greengrass_iot_data_publish(iot_client.get(), &params);
        
        std::cout << "Alert sent to subscriber Lambda" << std::endl;

        free(alert_payload);
        json_decref(alert_data);
    }

    void subscribe_to_topic(const std::string& topic) {
        aws_greengrass_subscribe_params subscribe_params = {
            .topic = topic.c_str(),
            .callback = message_callback_wrapper,
            .user_data = this
        };
        
        aws_greengrass_iot_data_subscribe(iot_client.get(), &subscribe_params);
        
        std::cout << "Temperature Processor Lambda started" << std::endl;
        std::cout << "Subscribed to " << topic << std::endl;
        std::cout << "Waiting for sensor data..." << std::endl;
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
        TemperatureProcessor processor;
        processor.subscribe_to_topic("sensors/temperature");
        processor.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}