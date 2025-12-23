# Greengrass V1 to V2 SDK Migration Reference

Complete reference for migrating Greengrass SDK code across all supported languages.
Each language includes all 4 scenarios: Local Publish, Local Subscribe, IoT Core Publish, IoT Core Subscribe.

## Python

### Local Publish
```python
# V1
import greengrasssdk
client = greengrasssdk.client('iot-data')
client.publish(topic='local/topic', payload=json.dumps(data))

# V2
from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2
from awsiot.greengrasscoreipc.model import PublishMessage, JsonMessage
ipc_client = GreengrassCoreIPCClientV2()
ipc_client.publish_to_topic(topic='local/topic', publish_message=PublishMessage(json_message=JsonMessage(message=data)))
```

### Local Subscribe
```python
# V1: Implicit via group subscriptions
def lambda_handler(event, context):
    data = event  # Message arrives as event parameter

# V2
from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2
ipc_client = GreengrassCoreIPCClientV2()

def on_message(event):
    message = event.message.json_message.message
    
ipc_client.subscribe_to_topic(topic='local/topic', on_stream_event=on_message)
```

### IoT Core Publish
```python
# V1
client.publish(topic='cloud/topic', payload=json.dumps(data))

# V2
from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2
from awsiot.greengrasscoreipc.model import QOS
ipc_client = GreengrassCoreIPCClientV2()
ipc_client.publish_to_iot_core(
    topic_name='cloud/topic',
    qos=QOS.AT_LEAST_ONCE,
    payload=json.dumps(data).encode('utf-8')
)
```

### IoT Core Subscribe
```python
# V1: Implicit via group subscriptions
def lambda_handler(event, context):
    command = event.get('command')  # Message from IoT Core

# V2
def on_command(event):
    data = json.loads(event.message.payload.decode('utf-8'))
    command = data.get('command')

ipc_client.subscribe_to_iot_core(
    topic_name='commands/device',
    qos=QOS.AT_LEAST_ONCE,
    on_stream_event=on_command
)
```

---

## Java

### Local Publish
```java
// V1
import com.amazonaws.greengrass.javasdk.IotDataClient;
import com.amazonaws.greengrass.javasdk.model.PublishRequest;
IotDataClient client = new IotDataClient();
PublishRequest request = new PublishRequest()
    .withTopic("local/topic")
    .withPayload(ByteBuffer.wrap(payload.getBytes()));
client.publish(request);

// V2
import software.amazon.awssdk.aws.greengrass.GreengrassCoreIPCClientV2;
import software.amazon.awssdk.aws.greengrass.model.*;
GreengrassCoreIPCClientV2 ipcClient = GreengrassCoreIPCClientV2.builder().build();
JsonMessage jsonMessage = new JsonMessage().withMessage(data);
PublishMessage publishMessage = new PublishMessage().withJsonMessage(jsonMessage);
PublishToTopicRequest request = new PublishToTopicRequest()
    .withTopic("local/topic")
    .withPublishMessage(publishMessage);
ipcClient.publishToTopic(request);
```

### Local Subscribe
```java
// V1: Implicit via group subscriptions
public String handleRequest(Map<String, Object> event, Context context) {
    String sensorId = (String) event.get("sensor_id");  // Message from subscription
}

// V2
import software.amazon.awssdk.aws.greengrass.GreengrassCoreIPCClientV2;
import software.amazon.awssdk.aws.greengrass.model.*;
import java.util.Optional;
GreengrassCoreIPCClientV2 ipcClient = GreengrassCoreIPCClientV2.builder().build();
SubscribeToTopicRequest subscribeRequest = new SubscribeToTopicRequest()
    .withTopic("local/topic");
ipcClient.subscribeToTopic(
    subscribeRequest,
    this::onMessage,
    Optional.empty(),
    Optional.empty()
);

public void onMessage(SubscriptionResponseMessage message) {
    Map<String, Object> data = message.getJsonMessage().getMessage();
}
```

### IoT Core Publish
```java
// V1
PublishRequest request = new PublishRequest()
    .withTopic("cloud/topic")
    .withPayload(ByteBuffer.wrap(payload.getBytes()));
client.publish(request);

// V2
import software.amazon.awssdk.aws.greengrass.GreengrassCoreIPCClientV2;
import software.amazon.awssdk.aws.greengrass.model.*;
GreengrassCoreIPCClientV2 ipcClient = GreengrassCoreIPCClientV2.builder().build();
PublishToIoTCoreRequest request = new PublishToIoTCoreRequest()
    .withTopicName("cloud/topic")
    .withQos(QOS.AT_LEAST_ONCE)
    .withPayload(payload.getBytes());
ipcClient.publishToIoTCore(request);
```

### IoT Core Subscribe
```java
// V1: Implicit via group subscriptions
public String handleRequest(Map<String, Object> event, Context context) {
    String command = (String) event.get("command");  // From IoT Core
}

// V2
import software.amazon.awssdk.aws.greengrass.GreengrassCoreIPCClientV2;
import software.amazon.awssdk.aws.greengrass.model.*;
import java.nio.charset.StandardCharsets;
import java.util.Optional;
GreengrassCoreIPCClientV2 ipcClient = GreengrassCoreIPCClientV2.builder().build();
SubscribeToIoTCoreRequest subscribeRequest = new SubscribeToIoTCoreRequest()
    .withTopicName("commands/device")
    .withQos(QOS.AT_LEAST_ONCE);
ipcClient.subscribeToIoTCore(
    subscribeRequest,
    this::onCommand,
    Optional.empty(),
    Optional.empty()
);

public void onCommand(IoTCoreMessage message) {
    String payload = new String(message.getMessage().getPayload(), StandardCharsets.UTF_8);
}
```

---

## Node.js

### Local Publish
```javascript
// V1
const greengrasssdk = require('aws-greengrass-core-sdk');
const iotClient = new greengrasssdk.IotData();
iotClient.publish({topic: 'local/topic', payload: JSON.stringify(data)});

// V2
const greengrasscoreipc = require('aws-iot-device-sdk-v2').greengrasscoreipc;
const ipcClient = greengrasscoreipc.createClient();
await ipcClient.connect();
await ipcClient.publishToTopic({
    topic: 'local/topic',
    publishMessage: {jsonMessage: {message: data}}
});
```

### Local Subscribe
```javascript
// V1: Implicit via group subscriptions
exports.handler = function(event, context) {
    const sensorId = event.sensor_id;  // Message from subscription
};

// V2
const greengrasscoreipc = require('aws-iot-device-sdk-v2').greengrasscoreipc;
const ipcClient = greengrasscoreipc.createClient();
await ipcClient.connect();
const streamingOperation = ipcClient.subscribeToTopic({topic: 'local/topic'});
streamingOperation.on('message', (message) => {
    const data = message.jsonMessage.message;
});
await streamingOperation.activate();
```

### IoT Core Publish
```javascript
// V1
iotClient.publish({topic: 'cloud/topic', payload: JSON.stringify(data)});

// V2
const greengrasscoreipc = require('aws-iot-device-sdk-v2').greengrasscoreipc;
const ipcClient = greengrasscoreipc.createClient();
await ipcClient.connect();
await ipcClient.publishToIoTCore({
    topicName: 'cloud/topic',
    qos: 1,
    payload: Buffer.from(JSON.stringify(data), 'utf-8')
});
```

### IoT Core Subscribe
```javascript
// V1: Implicit via group subscriptions
exports.handler = function(event, context) {
    const command = event.command;  // From IoT Core
};

// V2
const greengrasscoreipc = require('aws-iot-device-sdk-v2').greengrasscoreipc;
const ipcClient = greengrasscoreipc.createClient();
await ipcClient.connect();
const streamingOperation = ipcClient.subscribeToIoTCore({
    topicName: 'commands/device',
    qos: 1
});
streamingOperation.on('message', (message) => {
    const payload = message.message.payload.toString('utf-8');
    const data = JSON.parse(payload);
});
await streamingOperation.activate();
```

---

## C

### Local Publish
```c
// V1
#include <aws/greengrass/greengrasssdk.h>
aws_greengrass_iot_data_client *client = aws_greengrass_iot_data_client_new();
aws_greengrass_publish_params params = {
    .topic = "local/topic",
    .payload = (uint8_t *)payload,
    .payload_len = strlen(payload)
};
aws_greengrass_iot_data_publish(client, &params);

// V2
#include <gg/ipc/client.h>
#include <gg/sdk.h>
gg_sdk_init();
GgError ret = ggipc_connect();
GgError ret = ggipc_publish_to_topic_json(GG_STR("local/topic"), payload_map);
```

### Local Subscribe
```c
// V1: Implicit via group subscriptions
void on_message_received(const char *topic, const uint8_t *payload, size_t payload_len, void *user_data) {
    // Process message
}

// V2
#include <gg/ipc/client.h>
#include <gg/sdk.h>
gg_sdk_init();
GgError ret = ggipc_connect();
GgError ret = ggipc_subscribe_to_topic(
    GG_STR("local/topic"),
    on_message_callback,
    NULL
);
```

### IoT Core Publish
```c
// V1
aws_greengrass_publish_params params = {
    .topic = "cloud/topic",
    .payload = (uint8_t *)payload,
    .payload_len = strlen(payload)
};
aws_greengrass_iot_data_publish(client, &params);

// V2
#include <gg/ipc/client.h>
#include <gg/sdk.h>
gg_sdk_init();
GgError ret = ggipc_connect();
GgError ret = ggipc_publish_to_iot_core(
    GG_STR("cloud/topic"),
    GG_QOS_AT_LEAST_ONCE,
    payload_buffer
);
```

### IoT Core Subscribe
```c
// V1: Implicit via group subscriptions
// Message arrives in handler function

// V2
#include <gg/ipc/client.h>
#include <gg/sdk.h>
gg_sdk_init();
GgError ret = ggipc_connect();
GgError ret = ggipc_subscribe_to_iot_core(
    GG_STR("commands/device"),
    GG_QOS_AT_LEAST_ONCE,
    on_command_callback,
    NULL
);
```
GgError ret = ggipc_subscribe_to_iot_core(
    GG_STR("commands/device"),
    GG_QOS_AT_LEAST_ONCE,
    on_command_callback,
    NULL
);
```

---

## C++

### Local Publish
```cpp
// V1
#include <aws/greengrass/greengrasssdk.h>
std::unique_ptr<aws_greengrass_iot_data_client, 
                decltype(&aws_greengrass_iot_data_client_destroy)> iot_client(
    aws_greengrass_iot_data_client_new(), 
    aws_greengrass_iot_data_client_destroy);
aws_greengrass_publish_params params = {
    .topic = "local/topic",
    .payload = reinterpret_cast<uint8_t*>(payload),
    .payload_len = strlen(payload)
};
aws_greengrass_iot_data_publish(iot_client.get(), &params);

// V2
#include <gg/ipc/client.hpp>
#include <gg/buffer.hpp>
auto& client = gg::ipc::Client::get();
client.connect();
gg::Buffer buffer(payload_string);
auto error = client.publish_to_topic("local/topic", buffer);
```

### Local Subscribe
```cpp
// V1
static void message_callback_wrapper(const char *topic, 
                                    const uint8_t *payload, 
                                    size_t payload_len, 
                                    void *user_data) {
    // Process message
}
aws_greengrass_subscribe_params subscribe_params = {
    .topic = "local/topic",
    .callback = message_callback_wrapper,
    .user_data = this
};
aws_greengrass_iot_data_subscribe(iot_client.get(), &subscribe_params);

// V2
#include <gg/ipc/client.hpp>
class MessageCallback : public gg::ipc::LocalTopicCallback {
    void operator()(std::string_view topic, gg::Object payload, 
                   gg::ipc::Subscription& handle) override {
        // Process message
    }
};
auto& client = gg::ipc::Client::get();
auto error = client.connect();
static MessageCallback handler;
auto error = client.subscribe_to_topic("local/topic", handler);
```

### IoT Core Publish
```cpp
// V1
aws_greengrass_publish_params params = {
    .topic = "cloud/topic",
    .payload = reinterpret_cast<uint8_t*>(payload),
    .payload_len = strlen(payload)
};
aws_greengrass_iot_data_publish(iot_client.get(), &params);

// V2
#include <gg/ipc/client.hpp>
#include <gg/buffer.hpp>
auto& client = gg::ipc::Client::get();
auto error = client.connect();
gg::Buffer buffer(payload_string);
auto error = client.publish_to_iot_core("cloud/topic", buffer);
```

### IoT Core Subscribe
```cpp
// V1
aws_greengrass_subscribe_params subscribe_params = {
    .topic = "commands/device",
    .callback = message_callback_wrapper,
    .user_data = this
};
aws_greengrass_iot_data_subscribe(iot_client.get(), &subscribe_params);

// V2
#include <gg/ipc/client.hpp>
class CloudCallback : public gg::ipc::IoTCoreTopicCallback {
    void operator()(std::string_view topic, gg::Object payload,
                   gg::ipc::Subscription& handle) override {
        // Process message
    }
};
auto& client = gg::ipc::Client::get();
auto error = client.connect();
static CloudCallback handler;
auto error = client.subscribe_to_iot_core("commands/device", handler);
```

---

## SDK Packages

| Language | Package | Installation |
|----------|---------|--------------|
| Python | awsiotsdk | `pip install awsiotsdk` |
| Java | software.amazon.awssdk.aws.greengrass | Maven/Gradle dependency |
| Node.js | aws-iot-device-sdk-v2 | `npm install aws-iot-device-sdk-v2` |
| C | aws-iot-device-sdk-embedded-C | Build from source |
| C++ | aws-iot-device-sdk-cpp-v2 | Build from source |
