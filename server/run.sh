#!/usr/bin/env bash
set -euo pipefail

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Virtual environment directory
VENV_DIR=".venv"

echo "====================================="
echo "  ESP32 Sensor Monitor Server"
echo "====================================="
echo

# Step 1: Create virtual environment if it doesn't exist
echo "[*] Setting up Python virtual environment..."
if [ ! -d "$VENV_DIR" ]; then
    echo "    Creating virtual environment at $VENV_DIR..."
    python3 -m venv "$VENV_DIR"
    echo "    ✓ Virtual environment created"
else
    echo "    ✓ Virtual environment already exists"
fi

# Step 2: Activate virtual environment
echo "[*] Activating virtual environment..."
source "$VENV_DIR/bin/activate"
echo "    ✓ Virtual environment activated"

# Step 3: Upgrade pip and install dependencies
echo "[*] Installing dependencies..."
python -m pip install --upgrade pip --quiet
python -m pip install -r requirements.txt
echo "    ✓ Dependencies installed"

echo
echo "====================================="
echo "[*] Starting FastAPI server"
echo "====================================="
echo
echo "[INFO] Dashboard: http://localhost:8000"
echo "[INFO] API Latest: http://localhost:8000/api/sensor/latest"
echo "[INFO] API History: http://localhost:8000/api/sensor/history?limit=100"
echo "[INFO] API Stats: http://localhost:8000/api/sensor/stats"
echo
echo "[!] Check ESP32 IP in app.py (ESP32_IP = \"10.92.221.159\")"
echo

# Step 4: Run the server
python -m uvicorn app:app --host 0.0.0.0 --port 8000 --reload