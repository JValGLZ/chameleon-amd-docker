#!/bin/bash
set -e

echo "=== XSBench Energy and Performance Test ==="
echo "Running XSBench with energy + performance monitoring..."
echo ""

# Check if we're running in a container environment or if we should use docker
if command -v docker &> /dev/null && [ -f "XSBench" ]; then
    echo "Running XSBench in Docker container..."
    output=$(docker run --privileged --rm -v /sys:/sys -v $(pwd):/xsbench papi-cpu-powercap:latest /xsbench/XSBench)
    echo "$output"
    
    # Extract verification line for comparison
    echo ""
    echo "=== Key Results ==="
    echo "$output" | grep "Verification checksum"
    echo "=== XSBench Test Complete ==="
elif [ -f "XSBench" ]; then
    echo "Running XSBench directly..."
    ./XSBench
else
    echo "ERROR: XSBench binary not found. Please compile first."
    exit 1
fi
