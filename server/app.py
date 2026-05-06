from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import sqlite3
import json
from datetime import datetime
from contextlib import contextmanager
from typing import List
import asyncio
import websockets

app = FastAPI(title="ESP32 Sensor Monitor")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

DB_PATH = "sensor_data.db"

# ────────────────────────────────────────────────────────────────────────────
# Database Setup
# ────────────────────────────────────────────────────────────────────────────

@contextmanager
def get_db():
    """Context manager để quản lý kết nối SQLite"""
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
    finally:
        conn.close()


def init_db():
    """Khởi tạo bảng nếu chưa tồn tại"""
    with get_db() as conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS sensor_readings (
                id          INTEGER PRIMARY KEY AUTOINCREMENT,
                temperature REAL    NOT NULL,
                humidity    REAL    NOT NULL,
                smoke_value INTEGER NOT NULL,
                result      INTEGER NOT NULL,
                timestamp   TEXT    DEFAULT (datetime('now','localtime'))
            )
        """)
        conn.commit()


# ────────────────────────────────────────────────────────────────────────────
# WebSocket Manager (quản lý các client kết nối dashboard)
# ────────────────────────────────────────────────────────────────────────────

class WsManager:
    """Quản lý tất cả WebSocket client kết nối tới /ws (dashboard)"""
    def __init__(self):
        self._conns: List[WebSocket] = []

    async def connect(self, ws: WebSocket):
        await ws.accept()
        self._conns.append(ws)
        print(f"[WS] Dashboard connected. Total: {len(self._conns)}")

    def disconnect(self, ws: WebSocket):
        if ws in self._conns:
            self._conns.remove(ws)
        print(f"[WS] Dashboard disconnected. Total: {len(self._conns)}")

    async def broadcast(self, data: str):
        """Gửi dữ liệu tới tất cả dashboard client"""
        dead = []
        for ws in self._conns:
            try:
                await ws.send_text(data)
            except Exception as e:
                print(f"[WS] Error sending to client: {e}")
                dead.append(ws)
        for ws in dead:
            self.disconnect(ws)


manager = WsManager()


# ────────────────────────────────────────────────────────────────────────────
# ESP32 WebSocket Client (kết nối tới ESP32 để nhận dữ liệu)
# ────────────────────────────────────────────────────────────────────────────

class ESP32Client:
    """Client WebSocket kết nối tới ESP32 để lắng nghe dữ liệu"""
    def __init__(self, esp32_ip: str, esp32_port: int = 80):
        self.esp32_ip = esp32_ip
        self.esp32_port = esp32_port
        self.uri = f"ws://{esp32_ip}:{esp32_port}/ws"
        self.connected = False

    async def connect_and_listen(self):
        """Kết nối WebSocket tới ESP32 và lắng nghe dữ liệu"""
        while True:
            try:
                print(f"[ESP32 Client] Connecting to {self.uri}...")
                async with websockets.connect(self.uri, ping_interval=10, ping_timeout=10) as ws:
                    self.connected = True
                    print(f"[ESP32 Client] ✓ Connected successfully!")
                    
                    while True:
                        try:
                            message = await ws.recv()
                            await self.handle_message(message)
                        except websockets.exceptions.ConnectionClosed:
                            print("[ESP32 Client] ESP32 connection closed")
                            break
                        except Exception as e:
                            print(f"[ESP32 Client] Error receiving data: {e}")
                            break
                            
            except Exception as e:
                print(f"[ESP32 Client] Connection failed: {e}")
                self.connected = False
                print("[ESP32 Client] Retrying connection in 5 seconds...")
                await asyncio.sleep(5)

    async def handle_message(self, message: str):
        """Xử lý dữ liệu nhận từ ESP32"""
        try:
            data = json.loads(message)
            
            if data.get("type") == "sensor_data":
                temp = data.get("temperature")
                humi = data.get("humidity")
                smoke = data.get("smokeValue")
                status = data.get("result")
                
                print(f"[ESP32 Rx] T={temp}°C, H={humi}%, Gas={smoke}, Status={status}")
                
                # Ghi vào database
                await self.save_to_db(temp, humi, smoke, status)
                
                # Broadcast tới dashboard
                broadcast_data = {
                    "type": "sensor_data",
                    "temperature": temp,
                    "humidity": humi,
                    "smokeValue": smoke,
                    "result": status,
                    "timestamp": datetime.now().strftime("%H:%M:%S"),
                }
                await manager.broadcast(json.dumps(broadcast_data))
                
        except json.JSONDecodeError:
            print(f"[ESP32 Client] Invalid JSON format: {message}")
        except Exception as e:
            print(f"[ESP32 Client] Processing error: {e}")

    async def save_to_db(self, temperature, humidity, smoke_value, result):
        """Ghi dữ liệu vào SQLite"""
        try:
            with get_db() as conn:
                conn.execute(
                    "INSERT INTO sensor_readings (temperature, humidity, smoke_value, result) VALUES (?,?,?,?)",
                    (temperature, humidity, smoke_value, result),
                )
                conn.commit()
            print(f"[DB] ✓ Saved: T={temperature}, H={humidity}, Gas={smoke_value}, Status={result}")
        except Exception as e:
            print(f"[DB] ✗ Failed: {e}")


# Global ESP32 client instance
esp32_client = None


# ────────────────────────────────────────────────────────────────────────────
# Startup & Shutdown
# ────────────────────────────────────────────────────────────────────────────

@app.on_event("startup")
async def startup():
    """Chạy khi server khởi động"""
    global esp32_client
    
    init_db()
    print("[*] Database initialized successfully")
    
    # Đọc IP ESP32 từ file config hoặc env (tuỳ chọn)
    # Tạm thời hardcode, sau có thể đọc từ file
    ESP32_IP = "10.92.221.159"  # [TODO] Thay bằng IP ESP32 thực tế
    
    esp32_client = ESP32Client(ESP32_IP, 80)
    asyncio.create_task(esp32_client.connect_and_listen())
    print(f"[*] ESP32 Client running (IP: {ESP32_IP})")


# ────────────────────────────────────────────────────────────────────────────
# REST API Endpoints (truy vấn database)
# ────────────────────────────────────────────────────────────────────────────

@app.get("/api/sensor/latest")
def latest():
    """Lấy bản ghi cảm biến mới nhất"""
    with get_db() as conn:
        row = conn.execute(
            "SELECT temperature, humidity, smoke_value, result, timestamp "
            "FROM sensor_readings ORDER BY id DESC LIMIT 1"
        ).fetchone()
    if not row:
        return {}
    r = dict(row)
    r["type"] = "sensor_data"
    r["smokeValue"] = r.pop("smoke_value")
    return r


@app.get("/api/sensor/history")
def history(limit: int = 60):
    """Lấy lịch sử N bản ghi gần đây"""
    with get_db() as conn:
        rows = conn.execute(
            "SELECT temperature, humidity, smoke_value, result, timestamp "
            "FROM sensor_readings ORDER BY id DESC LIMIT ?",
            (limit,),
        ).fetchall()
    result = []
    for r in reversed(rows):
        d = dict(r)
        d["smokeValue"] = d.pop("smoke_value")
        result.append(d)
    return result


@app.get("/api/sensor/stats")
def stats():
    """Lấy thống kê (min/max/avg)"""
    with get_db() as conn:
        row = conn.execute("""
            SELECT
                COUNT(*)                      AS total,
                MIN(temperature)              AS temp_min,
                MAX(temperature)              AS temp_max,
                ROUND(AVG(temperature), 1)    AS temp_avg,
                MIN(humidity)                 AS humi_min,
                MAX(humidity)                 AS humi_max,
                ROUND(AVG(humidity), 1)       AS humi_avg,
                MIN(smoke_value)              AS smoke_min,
                MAX(smoke_value)              AS smoke_max,
                ROUND(AVG(smoke_value), 0)    AS smoke_avg
            FROM sensor_readings
        """).fetchone()
    return dict(row) if row else {}


# ────────────────────────────────────────────────────────────────────────────
# WebSocket Endpoint (dashboard client)
# ────────────────────────────────────────────────────────────────────────────

@app.websocket("/ws")
async def ws_endpoint(ws: WebSocket):
    """WebSocket endpoint cho dashboard (trình duyệt)"""
    await manager.connect(ws)
    
    # Gửi bản ghi mới nhất ngay khi client kết nối
    with get_db() as conn:
        row = conn.execute(
            "SELECT temperature, humidity, smoke_value, result, timestamp "
            "FROM sensor_readings ORDER BY id DESC LIMIT 1"
        ).fetchone()
    if row:
        d = dict(row)
        d["type"] = "sensor_data"
        d["smokeValue"] = d.pop("smoke_value")
        await ws.send_text(json.dumps(d))
    
    try:
        while True:
            # Chỉ lắng nghe, không cần xử lý tin nhắn từ client
            await ws.receive_text()
    except WebSocketDisconnect:
        manager.disconnect(ws)


# ────────────────────────────────────────────────────────────────────────────
# Static Files (dashboard UI)
# ────────────────────────────────────────────────────────────────────────────

app.mount("/", StaticFiles(directory="static", html=True), name="static")