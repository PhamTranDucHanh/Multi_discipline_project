from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import sqlite3
import json
from datetime import datetime
from contextlib import contextmanager
from typing import List

app = FastAPI(title="ESP32 Sensor Monitor")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

DB_PATH = "sensor_data.db"


@contextmanager
def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
    finally:
        conn.close()


def init_db():
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


class WsManager:
    def __init__(self):
        self._conns: List[WebSocket] = []

    async def connect(self, ws: WebSocket):
        await ws.accept()
        self._conns.append(ws)

    def disconnect(self, ws: WebSocket):
        if ws in self._conns:
            self._conns.remove(ws)

    async def broadcast(self, data: str):
        dead = []
        for ws in self._conns:
            try:
                await ws.send_text(data)
            except Exception:
                dead.append(ws)
        for ws in dead:
            self.disconnect(ws)


manager = WsManager()


class SensorPayload(BaseModel):
    type: str
    temperature: float
    humidity: float
    smokeValue: int
    result: int


@app.on_event("startup")
async def startup():
    init_db()


# ── API ───────────────────────────────────────────────────────────────────────

@app.post("/api/sensor")
async def receive_sensor(data: SensorPayload):
    if data.type != "sensor_data":
        return {"status": "ignored"}

    with get_db() as conn:
        conn.execute(
            "INSERT INTO sensor_readings (temperature, humidity, smoke_value, result) VALUES (?,?,?,?)",
            (data.temperature, data.humidity, data.smokeValue, data.result),
        )
        conn.commit()

    await manager.broadcast(json.dumps({
        "type": "sensor_data",
        "temperature": data.temperature,
        "humidity": data.humidity,
        "smokeValue": data.smokeValue,
        "result": data.result,
        "timestamp": datetime.now().strftime("%H:%M:%S"),
    }))
    return {"status": "ok"}


@app.get("/api/sensor/latest")
def latest():
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


# ── WebSocket ─────────────────────────────────────────────────────────────────

@app.websocket("/ws")
async def ws_endpoint(ws: WebSocket):
    await manager.connect(ws)
    # Send latest reading immediately on connect
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
            await ws.receive_text()
    except WebSocketDisconnect:
        manager.disconnect(ws)


# ── Static files (must be last) ───────────────────────────────────────────────
app.mount("/", StaticFiles(directory="static", html=True), name="static")
