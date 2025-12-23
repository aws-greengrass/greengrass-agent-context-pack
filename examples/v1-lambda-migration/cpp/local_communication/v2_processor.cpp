#include <gg/ipc/client.hpp>
#include <gg/buffer.hpp>
#include <gg/object.hpp>
#include <gg/types.hpp>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

struct AlertData {
    std::string sensor_id;
    int64_t temperature;
};

static std::mutex alert_mutex;
static std::condition_variable alert_cv;
static AlertData pending_alert;
static bool has_pending_alert = false;

void alert_publisher_thread() {
    auto& client = gg::ipc::Client::get();
    
    while (true) {
        std::unique_lock<std::mutex> lock(alert_mutex);
        alert_cv.wait(lock, [] { return has_pending_alert; });
        
        AlertData alert = pending_alert;
        has_pending_alert = false;
        lock.unlock();
        
        // Create alert payload as JSON string.
        std::string json_payload = "{\"sensor_id\":\"" + alert.sensor_id + 
                                  "\",\"temperature\":" + std::to_string(alert.temperature) + 
                                  ",\"alert\":\"HIGH_TEMPERATURE\"}";
        
        // Convert to Buffer and publish.
        gg::Buffer buffer(json_payload);
        auto error = client.publish_to_topic("component/alerts", buffer);
        
        if (error) {
            std::cerr << "Failed to publish alert\n";
        } else {
            std::cout << "Alert sent to AlertHandler component\n";
        }
    }
}

class SensorCallback : public gg::ipc::LocalTopicCallback {
    void operator()(
        std::string_view topic,
        gg::Object payload,
        gg::ipc::Subscription& handle
    ) override {
        (void) topic;
        (void) handle;
        
        // Payload is a Buffer containing JSON string.
        if (payload.index() != GG_TYPE_BUF) {
            std::cerr << "Expected Buffer message\n";
            return;
        }
        
        // Extract buffer using gg::get.
        auto buffer = gg::get<std::span<uint8_t>>(payload);
        std::string json_str(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        
        // Simple JSON parsing for demo.
        std::string sensor_id = "sensor1"; 
        int64_t temperature = 0;
        
        // Extract temperature (simple string search).
        size_t temp_pos = json_str.find("\"temperature\":");
        if (temp_pos != std::string::npos) {
            temp_pos += 14; // Skip "temperature".
            size_t end_pos = json_str.find_first_of(",}", temp_pos);
            if (end_pos != std::string::npos) {
                temperature = std::stoll(json_str.substr(temp_pos, end_pos - temp_pos));
            }
        }
        
        std::cout << "Received from sensor " << sensor_id << ": " 
                  << temperature << "°F\n";
        
        if (temperature > 80) {
            std::lock_guard<std::mutex> lock(alert_mutex);
            pending_alert = {sensor_id, temperature};
            has_pending_alert = true;
            alert_cv.notify_one();
        }
    }
};

int main() {
    // Disable stdout buffering for real-time logging.
    std::cout.setf(std::ios::unitbuf);
    
    std::cout << "Temperature Processor component starting..." << std::endl;
    
    auto& client = gg::ipc::Client::get();
    std::cout << "Got client instance" << std::endl;
    
    auto error = client.connect();
    std::cout << "Connect returned, error code: " << error.value() << std::endl;
    
    if (error) {
        std::cerr << "Failed to connect to Greengrass nucleus: " << error.message() << std::endl;
        return 1;
    }
    
    std::cout << "Connected to Greengrass IPC" << std::endl;
    
    // Start alert publisher thread.
    std::thread alert_thread(alert_publisher_thread);
    alert_thread.detach();
    
    // Handler must be static lifetime if subscription handle is not held.
    static SensorCallback handler;
    error = client.subscribe_to_topic("sensors/temperature", handler);
    
    if (error) {
        std::cerr << "Failed to subscribe to topic: " << error.message() << std::endl;
        return 1;
    }
    
    std::cout << "Subscribed to sensors/temperature" << std::endl;
    std::cout << "Waiting for sensor data..." << std::endl;
    
    // Keep running.
    while (true) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
    
    return 0;
}