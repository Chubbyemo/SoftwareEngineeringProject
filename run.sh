#!/bin/bash

# Exit on error
set -e

echo "=== Running The Braendi Dog Game ==="

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: build directory not found. Please run build.sh first."
    exit 1
fi

cd build

# Check if executables exist
if [ ! -f "./Server" ] || [ ! -f "./Client" ]; then
    echo "Error: Server or Client executable not found. Please run build.sh first."
    exit 1
fi

# Default values
SERVER_IP="127.0.0.1"
SERVER_PORT="12345"
NUM_CLIENTS=2

# Parse command line arguments (optional)
while [[ $# -gt 0 ]]; do
    case $1 in
        -n|--num-clients)
            NUM_CLIENTS="$2"
            shift 2
            ;;
        -p|--port)
            SERVER_PORT="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [-n|--num-clients NUM] [-p|--port PORT]"
            exit 1
            ;;
    esac
done

# Start the server in background
echo "Starting server on $SERVER_IP:$SERVER_PORT..."
./Server $SERVER_IP $SERVER_PORT &
SERVER_PID=$!

# Wait a bit for server to start
sleep 2

# Start clients
echo "Starting $NUM_CLIENTS client(s)..."
for i in $(seq 1 $NUM_CLIENTS); do
    echo "Starting client $i..."
    ./Client &
    sleep 1
done

echo "=== Game is running ==="
echo "Server PID: $SERVER_PID"
echo "Press Ctrl+C to stop all processes"

# Wait for server process
wait $SERVER_PID