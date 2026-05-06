@echo off
setlocal enabledelayedexpansion


REM Install dependencies
echo [*] Installing dependencies...
pip install -r requirements.txt

echo.
echo =====================================
echo [*] Starting FastAPI server
echo =====================================
echo.
echo [INFO] Dashboard: http://localhost:8000
echo [INFO] API Latest: http://localhost:8000/api/sensor/latest
echo [INFO] API History: http://localhost:8000/api/sensor/history?limit=100
echo [INFO] API Stats: http://localhost:8000/api/sensor/stats
echo.
echo [!] Check ESP32 IP in app.py (ESP32_IP = "10.92.221.159")
echo.

REM Start uvicorn
uvicorn app:app --host 0.0.0.0 --port 8000 --reload

pause