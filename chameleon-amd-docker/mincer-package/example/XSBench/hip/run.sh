#!/bin/bash
set -e

echo "=== XSBench AMD GPU Energy and Performance Test ==="
echo "Running XSBench with PAPI ROCm monitoring..."
echo ""

# Check if we're running in a container environment or if we should use docker
if command -v docker &> /dev/null && [ -f "Main.cpp" ]; then
    echo "Running XSBench in Docker container with AMD GPU support..."
    cd /home/cc
    output=$(docker run --rm --device=/dev/kfd --device=/dev/dri --group-add video \
        -v $(pwd)/mincer-package/example/XSBench:/workspace/XSBench ubuntu-rocm-papi \
        bash -c "cd /workspace/XSBench/hip && make clean && make && ./XSBench -s small -m event")
    echo "$output"
    
    # Extract key results for comparison
    echo ""
    echo "=== Key Results ==="
    echo "$output" | grep "Verification checksum" || echo "Verification checksum: Not found"
    echo "$output" | grep "Lookups/s:" || echo "Performance: Not found"
    echo "$output" | grep "GPU.*:" || echo "GPU metrics: Not found"
    echo "=== XSBench AMD GPU Test Complete ==="
elif [ -f "XSBench" ]; then
    echo "Running XSBench directly..."
    ./XSBench -s small -m event
else
    echo "ERROR: XSBench binary not found. Please compile first or ensure Docker is available."
    exit 1
fi
