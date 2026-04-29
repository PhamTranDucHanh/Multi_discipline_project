@echo off
echo [*] Cai dat thu vien...
pip install -r requirements.txt

echo.
echo [*] Khoi dong server tai http://0.0.0.0:8000
echo     Mo trinh duyet: http://localhost:8000
echo     ESP32 gui POST toi: http://<IP_MAY_TINH>:8000/api/sensor
echo.
uvicorn app:app --host 0.0.0.0 --port 8000 --reload
