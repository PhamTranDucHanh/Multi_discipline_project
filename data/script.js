// ==================== HOME GAUGES ====================
function initGauges() {
    // Gauge trạng thái cảnh báo
    window.gaugeStatus = new JustGage({
        id: "gauge_status",
        value: 0,
        min: 0,
        max: 2,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: false,
        levelColors: ["#4CAF50", "#FFC107", "#F44336"],
        customSectors: [
            {color: "#4CAF50", lo: 0, hi: 0},
            {color: "#FFC107", lo: 1, hi: 1},
            {color: "#F44336", lo: 2, hi: 2}
        ],
        label: ""
    });

    window.gaugeTemp = new JustGage({
        id: "gauge_temp",
        value: 0, // Giá trị ban đầu
        min: -10,
        max: 50,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });

    window.gaugeHumi = new JustGage({
        id: "gauge_humi",
        value: 0, // Giá trị ban đầu
        min: 0,
        max: 100,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    });

    window.gaugeGas = new JustGage({
        id: "gauge_gas",
        value: 0,
        min: 0,
        max: 4095, // Hiển thị giá trị analog khí gas
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#292727ff", "#FFC107", "#f4d7d7ff"] // Dark, Medium, Bright
    });
}
// Nếu muốn lấy giá trị hiệu ứng khi gửi dữ liệu:
// const effect = document.getElementById('customEffect').value;
// ==================== NEOPIXEL MODE UI ====================
function onNeopixelModeChange() {
    const mode = document.getElementById('neopixelMode').value;
    document.querySelectorAll('.neopixel-mode').forEach(el => el.style.display = 'none');
    if (mode === 'custom') document.getElementById('neopixelCustom').style.display = 'block';
    if (mode === 'temp') document.getElementById('neopixelTemp').style.display = 'block';
    if (mode === 'humi') document.getElementById('neopixelHumi').style.display = 'block';
    if (mode === 'light') document.getElementById('neopixelLight').style.display = 'block';
}
window.onNeopixelModeChange = onNeopixelModeChange;

// ==================== NEOPIXEL APPLY BUTTON ====================
document.getElementById('applyNeopixelSettings').onclick = function() {
    const mode = document.getElementById('neopixelMode').value;
    let payload = { page: "neopixel", mode: mode };
    // Helper: convert hex to RGB array
    function hexToRgb(hex) {
        hex = hex.replace('#', '');
        if (hex.length === 3) hex = hex[0]+hex[0]+hex[1]+hex[1]+hex[2]+hex[2];
        var bigint = parseInt(hex, 16);
        return [ (bigint >> 16) & 255, (bigint >> 8) & 255, bigint & 255 ];
    }
    if (mode === 'custom') {
        const rgb = hexToRgb(document.getElementById('neopixelColorBar').value);
        payload.color = rgb; // Đảm bảo gửi dạng array [r,g,b]
        payload.effect = document.getElementById('customEffect') ? document.getElementById('customEffect').value : null;
    } else if (mode === 'temp') {
        payload.temp = {
            low: {
                min: Number(document.getElementById('tempLowMin').value),
                max: Number(document.getElementById('tempLowMax').value),
                color: hexToRgb(document.getElementById('tempLowColor').value)
            },
            mid: {
                min: Number(document.getElementById('tempMidMin').value),
                max: Number(document.getElementById('tempMidMax').value),
                color: hexToRgb(document.getElementById('tempMidColor').value)
            },
            high: {
                min: Number(document.getElementById('tempHighMin').value),
                max: Number(document.getElementById('tempHighMax').value),
                color: hexToRgb(document.getElementById('tempHighColor').value)
            }
        };
    } else if (mode === 'humi') {
        payload.humi = {
            low: {
                min: Number(document.getElementById('humiLowMin').value),
                max: Number(document.getElementById('humiLowMax').value),
                color: hexToRgb(document.getElementById('humiLowColor').value)
            },
            mid: {
                min: Number(document.getElementById('humiMidMin').value),
                max: Number(document.getElementById('humiMidMax').value),
                color: hexToRgb(document.getElementById('humiMidColor').value)
            },
            high: {
                min: Number(document.getElementById('humiHighMin').value),
                max: Number(document.getElementById('humiHighMax').value),
                color: hexToRgb(document.getElementById('humiHighColor').value)
            }
        };
    } else if (mode === 'light') {
        payload.light = {
            low: {
                min: Number(document.getElementById('lightLowMin').value),
                max: Number(document.getElementById('lightLowMax').value),
                color: hexToRgb(document.getElementById('lightLowColor').value)
            },
            mid: {
                min: Number(document.getElementById('lightMidMin').value),
                max: Number(document.getElementById('lightMidMax').value),
                color: hexToRgb(document.getElementById('lightMidColor').value)
            },
            high: {
                min: Number(document.getElementById('lightHighMin').value),
                max: Number(document.getElementById('lightHighMax').value),
                color: hexToRgb(document.getElementById('lightHighColor').value)
            }
        };
    }
    console.log("NeoPixel gửi:", JSON.stringify(payload)); // In ra để debug
    Send_Data(JSON.stringify(payload));
    alert("Đã gửi cấu hình NeoPixel!");
};
// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', onLoad);

function onLoad(event) {
    loadRelaysFromStorage();
    initWebSocket();
    initGauges();
    loadRelaysFromStorage() 
    renderRelays(); // <-- Đảm bảo hiển thị lại danh sách relay
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("Gửi:", data);
    } else {
        console.warn("WebSocket chưa sẵn sàng!");
        alert("WebSocket chưa kết nối!");
    }
}

function onMessage(event) {
    console.log("Nhận:", event.data);
    try {
        var data = JSON.parse(event.data);
        // Xử lý cập nhật trạng thái relay từ ESP32
        if(data.type === "device") { 
            refreshRelayUI(data.value);
        }
        // ================== Cập nhật gauge trạng thái ==================
        if (data.type === "sensor_data") {
            // Cập nhật gauge trạng thái nếu có trường status
                            if (window.gaugeStatus && typeof data.status !== 'undefined') {
                            let label = "";
                            let color = "#4CAF50";
                            if (data.status == 0) { label = "NORMAL"; color = "#4CAF50"; }
                            else if (data.status == 1) { label = "GAS LEAK"; color = "#FFC107"; }
                            else if (data.status == 2) { label = "BURN"; color = "#F44336"; }
                            else { label = "UNKNOWN"; color = "#888"; }
                            window.gaugeStatus.refresh(data.status);
                            // Xóa label cũ nếu có (tránh bị ghi đè bởi JustGage)
                            let oldLabel = document.getElementById('gauge_status_label');
                            if (oldLabel) oldLabel.remove();
                            // Thêm label mới
                            let labelDiv = document.createElement('div');
                            labelDiv.id = 'gauge_status_label';
                            labelDiv.style.textAlign = 'center';
                            labelDiv.style.fontWeight = 'bold';
                            labelDiv.style.fontSize = '1.2em';
                            labelDiv.style.marginTop = '8px';
                            labelDiv.innerText = label;
                            labelDiv.style.color = color;
                            document.getElementById('gauge_status').appendChild(labelDiv);
                            }
            // Cập nhật các gauge còn lại
            if (window.gaugeTemp && typeof data.temperature !== 'undefined') {
                window.gaugeTemp.refresh(data.temperature);
            }
            if (window.gaugeHumi && typeof data.humidity !== 'undefined') {
                window.gaugeHumi.refresh(data.humidity);
            }
            if (window.gaugeGas && typeof data.smokeValue !== 'undefined') {
                window.gaugeGas.refresh(data.smokeValue);
            }
        }
    } catch (e) {
        console.warn("Không phải JSON hợp lệ:", event.data);
    }
}


// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}


// ==================== HOME GAUGES ====================
function initGauges() {
    // Khởi tạo các đồng hồ đo và lưu vào biến toàn cục (window.)
    // để các hàm khác có thể truy cập
    window.gaugeTemp = new JustGage({
        id: "gauge_temp",
        value: 0, // Giá trị ban đầu
        min: -10,
        max: 50,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });

    window.gaugeHumi = new JustGage({
        id: "gauge_humi",
        value: 0, // Giá trị ban đầu
        min: 0,
        max: 100,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    });

    window.gaugeGas = new JustGage({
        id: "gauge_gas",
        value: 0,
        min: 0,
        max: 4095, // Hiển thị giá trị analog khí gas
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#292727ff", "#FFC107", "#f4d7d7ff"] // Dark, Medium, Bright
    });
}


// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}
function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("Please fill all fields!");
    relayList.push({ id: Date.now(), name, gpio, state: false });
    saveRelaysToStorage();
    renderRelays();
    closeAddRelayDialog();
}
function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
      <i class="fa-solid fa-bolt device-icon"></i>
      <h3>${r.name}</h3>
      <p>GPIO: ${r.gpio}</p>
      <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
        ${r.state ? 'ON' : 'OFF'}
      </button>
      <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
    `;
        container.appendChild(card);
    });
}
function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        saveRelaysToStorage();
        const relayJSON = JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: relay.gpio
            }
        });
        Send_Data(relayJSON);
        renderRelays();
    }
}

function refreshRelayUI(update) {
    // update: { name, status: "ON"|"OFF", gpio,  }
    let relay = relayList.find(r => Number(r.gpio) === Number(update.gpio) && r.name === update.name);
    if (!relay) return;
    relay.state = (update.status === "ON");
    saveRelaysToStorage();
    renderRelays(); // redraw relay cards
}

function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}
function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}
function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    saveRelaysToStorage();
    renderRelays();
    closeConfirmDelete();
}


// ==================== SETTINGS FORM (BỔ SUNG) ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const password = document.getElementById("password").value.trim();
    const token = document.getElementById("token").value.trim();
    const server = document.getElementById("server").value.trim();
    const port = document.getElementById("port").value.trim();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: ssid,
            password: password,
            token: token,
            server: server,
            port: port
        }
    });

    Send_Data(settingsJSON);
    alert("Cấu hình đã được gửi đến thiết bị!");
});

// Lưu danh sách relay vào localStorage
function saveRelaysToStorage() {
    localStorage.setItem('relayList', JSON.stringify(relayList));
}

// Đọc danh sách relay từ localStorage (nếu có)
function loadRelaysFromStorage() {
    const data = localStorage.getItem('relayList');
    if (data) {
        try {
            relayList = JSON.parse(data);
        } catch (e) {
            relayList = [];
        }
    } else {
        relayList = [];
    }
}