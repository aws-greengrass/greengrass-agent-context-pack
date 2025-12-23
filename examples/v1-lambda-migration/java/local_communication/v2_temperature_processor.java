import software.amazon.awssdk.aws.greengrass.GreengrassCoreIPCClientV2;
import software.amazon.awssdk.aws.greengrass.model.PublishMessage;
import software.amazon.awssdk.aws.greengrass.model.PublishToTopicRequest;
import software.amazon.awssdk.aws.greengrass.model.JsonMessage;
import software.amazon.awssdk.aws.greengrass.model.SubscribeToTopicRequest;
import software.amazon.awssdk.aws.greengrass.model.SubscriptionResponseMessage;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class TemperatureProcessor {
    private static GreengrassCoreIPCClientV2 ipcClient;

    public static void main(String[] args) {
        System.out.println("Temperature Processor component starting...");

        try (GreengrassCoreIPCClientV2 client = GreengrassCoreIPCClientV2.builder().build()) {
            ipcClient = client;

            SubscribeToTopicRequest subscribeRequest = new SubscribeToTopicRequest()
                .withTopic("sensors/temperature");

            ipcClient.subscribeToTopic(
                subscribeRequest, 
                TemperatureProcessor::onSensorData,
                Optional.empty(),
                Optional.empty()
            );

            System.out.println("Subscribed to sensors/temperature");
            System.out.println("Waiting for sensor data...");

            while (true) {
                Thread.sleep(1000);
            }
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }

    public static void onSensorData(SubscriptionResponseMessage message) {
        try {
            Map<String, Object> data = message.getJsonMessage().getMessage();
            String sensorId = (String) data.get("sensor_id");
            Number temp = (Number) data.get("temperature");
            int temperature = temp.intValue();

            System.out.println("Received from sensor " + sensorId + ": " + temperature + "F");

            if (temperature > 80) {
                Map<String, Object> alertData = new HashMap<>();
                alertData.put("sensor_id", sensorId);
                alertData.put("temperature", temperature);
                alertData.put("alert", "HIGH_TEMPERATURE");

                JsonMessage jsonMessage = new JsonMessage().withMessage(alertData);
                PublishMessage publishMessage = new PublishMessage().withJsonMessage(jsonMessage);
                PublishToTopicRequest publishRequest = new PublishToTopicRequest()
                    .withTopic("component/alerts")
                    .withPublishMessage(publishMessage);

                ipcClient.publishToTopic(publishRequest);
                System.out.println("Alert sent to AlertHandler component");
            }
        } catch (Exception e) {
            System.err.println("Error processing sensor data: " + e.getMessage());
        }
    }
}
