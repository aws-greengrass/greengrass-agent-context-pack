# V1 Lambda Migration Examples

Complete examples showing V1 Lambda to V2 component migration patterns for Greengrass Nucleus Lite.

## Examples

### 1. Local Communication (`local_communication/`)
Temperature processor that receives sensor data from another Lambda and sends alerts.

**Files**:
- `v1_processor_lambda.py` - Original V1 Lambda using greengrasssdk
- `v2_temperature_processor.py` - Migrated V2 component using IPC
- `temperature_processor_recipe.json` - V2 component recipe

**Demonstrates**:
- Local pub/sub between components
- Message processing and conditional logic
- IPC authorization for local topics

### 2. Cloud Communication (`cloud_communication/`)
Device controller that receives commands from IoT Core and sends telemetry back.

**Files**:
- `v1_controller_lambda.py` - Original V1 Lambda using greengrasssdk
- `v2_controller.py` - Migrated V2 component using IPC
- `controller_recipe.json` - V2 component recipe

**Demonstrates**:
- IoT Core pub/sub (cloud communication)
- Command processing from cloud
- Telemetry publishing to cloud
- IPC authorization for IoT Core topics

## Key Migration Patterns

### SDK Replacement
**V1**: `greengrasssdk.client('iot-data')`  
**V2**: `awsiot.greengrasscoreipc.connect()`

### Local Publish/Subscribe
**V1**: Implicit via group subscriptions  
**V2**: Explicit IPC operations with authorization policies

### IoT Core Publish/Subscribe
**V1**: Same API as local (transparent routing)  
**V2**: Separate IPC operations (`PublishToIoTCore`, `SubscribeToIoTCore`)

### Authorization
**V1**: Group-level subscriptions in group definition  
**V2**: Component-level IPC policies in recipe

## Usage

Each example directory contains:
1. **V1 Lambda** - Original implementation for comparison
2. **V2 Component** - Migrated implementation
3. **Recipe** - Component configuration with IPC authorization

Compare V1 and V2 files side-by-side to understand migration changes.

## Deployment

See main migration guide at `../migration/migrate-v1-lambda-to-v2-component.md` for complete deployment instructions.

## Additional Resources

- **SDK Reference**: `../migration/sdk-migration-reference.md` - Complete SDK migration patterns for all languages
- **Migration Guide**: `../migration/migrate-v1-lambda-to-v2-component.md` - Step-by-step migration workflow
