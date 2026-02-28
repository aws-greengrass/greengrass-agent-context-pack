#!/bin/bash
# Export Greengrass V1 group configuration for migration to V2
# Usage: ./export_ggv1.sh <group-id> [output-dir] [region]

set -e

GROUP_ID="${1}"
OUTPUT_DIR="${2:-./ggv1_export}"
REGION="${3:-us-west-2}"

if [ -z "$GROUP_ID" ]; then
    echo "Usage: $0 <group-id> [output-dir] [region]"
    echo ""
    echo "Arguments:"
    echo "  group-id    : Greengrass V1 group ID (required)"
    echo "  output-dir  : Directory to save exported files (default: ./ggv1_export)"
    echo "  region      : AWS region (default: us-west-2)"
    echo ""
    echo "Example:"
    echo "  $0 7b5848b2-c186-4c16-b641-407a78a4bfd0 ./v1-export us-west-2"
    exit 1
fi

export AWS_DEFAULT_REGION="$REGION"
mkdir -p "$OUTPUT_DIR"

echo "Exporting Greengrass V1 Group: $GROUP_ID"
echo "Region: $REGION"
echo "Output: $OUTPUT_DIR"
echo ""

# Get group and latest version
VER=$(aws greengrass get-group --group-id "$GROUP_ID" --query 'LatestVersion' --output text)
aws greengrass get-group-version --group-id "$GROUP_ID" --group-version-id "$VER" > "$OUTPUT_DIR/group.json"

# Extract and export Lambda functions
FUNC_ARN=$(jq -r '.Definition.FunctionDefinitionVersionArn' "$OUTPUT_DIR/group.json")
if [ "$FUNC_ARN" != "null" ]; then
    FUNC_ID=$(echo "$FUNC_ARN" | awk -F'/' '{print $(NF-2)}')
    FUNC_VER=$(echo "$FUNC_ARN" | awk -F'/' '{print $NF}')
    aws greengrass get-function-definition-version \
        --function-definition-id "$FUNC_ID" \
        --function-definition-version-id "$FUNC_VER" > "$OUTPUT_DIR/functions.json"
    echo "✓ Exported Lambda functions"
    
    # Download Lambda function code
    mkdir -p "$OUTPUT_DIR/lambda-code"
    FUNC_COUNT=$(jq '.Definition.Functions | length' "$OUTPUT_DIR/functions.json")
    for i in $(seq 0 $((FUNC_COUNT - 1))); do
        FUNC_ARN=$(jq -r ".Definition.Functions[$i].FunctionArn" "$OUTPUT_DIR/functions.json")
        FUNC_NAME=$(echo "$FUNC_ARN" | awk -F':' '{print $(NF-1)}')
        
        # Skip AWS-managed Greengrass functions (ARN pattern: arn:aws:lambda:::function:GG*)
        if [[ "$FUNC_ARN" == *":::"* ]]; then
            echo "  Skipping AWS-managed function: $FUNC_NAME"
            continue
        fi
        
        echo "  Downloading $FUNC_NAME..."
        PRESIGNED_URL=$(aws lambda get-function --function-name "$FUNC_ARN" --query 'Code.Location' --output text)
        wget -q -O "$OUTPUT_DIR/lambda-code/${FUNC_NAME}.zip" "$PRESIGNED_URL"
        unzip -q "$OUTPUT_DIR/lambda-code/${FUNC_NAME}.zip" -d "$OUTPUT_DIR/lambda-code/$FUNC_NAME"
        rm "$OUTPUT_DIR/lambda-code/${FUNC_NAME}.zip"
    done
    echo "✓ Downloaded Lambda code to $OUTPUT_DIR/lambda-code/"
fi

# Extract and export subscriptions
SUB_ARN=$(jq -r '.Definition.SubscriptionDefinitionVersionArn' "$OUTPUT_DIR/group.json")
if [ "$SUB_ARN" != "null" ]; then
    SUB_ID=$(echo "$SUB_ARN" | awk -F'/' '{print $(NF-2)}')
    SUB_VER=$(echo "$SUB_ARN" | awk -F'/' '{print $NF}')
    aws greengrass get-subscription-definition-version \
        --subscription-definition-id "$SUB_ID" \
        --subscription-definition-version-id "$SUB_VER" > "$OUTPUT_DIR/subscriptions.json"
    echo "✓ Exported subscriptions"
fi

echo ""
echo "Export complete: $OUTPUT_DIR/"
echo ""
echo "Files created:"
ls -lh "$OUTPUT_DIR"
