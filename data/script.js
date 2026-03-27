let minTemp = Infinity, maxTemp = -Infinity;
let minHumi = Infinity, maxHumi = -Infinity;
let minGas = Infinity, maxGas = -Infinity;
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
    initCharts();
    loadRelaysFromStorage() 
    renderRelays(); // <-- Đảm bảo hiển thị lại danh sách relay
}

function onOpen(event) {
    console.log('Connection opened');
    updateConnectionStatus(true);
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

let connectionTimeout;

function updateConnectionStatus(isOnline) {
    const badge = document.getElementById('connBadge');
    const textNode = document.getElementById('connText');
    if(badge && textNode) {
        if(isOnline) {
            badge.classList.remove('offline');
            textNode.innerText = 'LIVE';
        } else {
            badge.classList.add('offline');
            textNode.innerText = 'OFFLINE';
        }
    }
}

function resetConnectionTimer() {
    updateConnectionStatus(true); // Đang có data -> Online
    clearTimeout(connectionTimeout); // Xóa bộ đếm cũ
    // ESP32 gửi 1 giây/lần. Nếu sau 3 giây không có tín hiệu -> Báo Offline
    connectionTimeout = setTimeout(() => {
        updateConnectionStatus(false);
    }, 3000); 
}

function onMessage(event) {
    console.log("Nhận:", event.data);
    try {
        resetConnectionTimer();
        var data = JSON.parse(event.data);
        if(data.type === "device") { refreshRelayUI(data.value); }
        
        if (data.type === "sensor_data") {
            // Cập nhật số liệu và tìm Min/Max
            if(data.temperature !== undefined) {
                if(data.temperature < minTemp) minTemp = data.temperature;
                if(data.temperature > maxTemp) maxTemp = data.temperature;
                document.getElementById("min_temp").innerText = minTemp;
                document.getElementById("max_temp").innerText = maxTemp;
                document.getElementById("rt_temp").innerText = data.temperature + "°C";
                updateChart(chartTemp, data.temperature);
            }

            if(data.humidity !== undefined) {
                if(data.humidity < minHumi) minHumi = data.humidity;
                if(data.humidity > maxHumi) maxHumi = data.humidity;
                document.getElementById("min_humi").innerText = minHumi;
                document.getElementById("max_humi").innerText = maxHumi;
                document.getElementById("rt_humi").innerText = data.humidity + "%";
                updateChart(chartHumi, data.humidity);
            }

            if(data.smokeValue !== undefined) {
                if(data.smokeValue < minGas) minGas = data.smokeValue;
                if(data.smokeValue > maxGas) maxGas = data.smokeValue;
                document.getElementById("min_gas").innerText = minGas;
                document.getElementById("max_gas").innerText = maxGas;
                document.getElementById("rt_gas").innerText = data.smokeValue;
                updateChart(chartGas, data.smokeValue);
            }

            // Cập nhật số liệu nhỏ trên góc phải đồ thị
            if(data.temperature !== undefined) document.getElementById("rt_temp").innerText = data.temperature + "°C";
            if(data.humidity !== undefined) document.getElementById("rt_humi").innerText = data.humidity + "%";
            if(data.smokeValue !== undefined) document.getElementById("rt_gas").innerText = data.smokeValue;

            // Cập nhật đồ thị chạy ngang
            if(data.temperature !== undefined) updateChart(chartTemp, data.temperature);
            if(data.humidity !== undefined) updateChart(chartHumi, data.humidity);
            if(data.smokeValue !== undefined) updateChart(chartGas, data.smokeValue);

            // Cập nhật thẻ Trạng thái AI
            if (typeof data.result !== 'undefined') {
                let badge = document.getElementById("statusBadge");
                badge.style.background = ""; 
                badge.style.color = "";
                if (data.result == 0) { 
                    badge.className = "status-badge safe";
                    badge.innerHTML = `<i class="fa-solid fa-circle-check"></i> <span>NORMAL</span>`;
                } else if (data.result == 1) { 
                    badge.className = "status-badge warn";
                    badge.innerHTML = `<i class="fa-solid fa-triangle-exclamation"></i> <span>GAS LEAK</span>`;
                } else if (data.result == 2) { 
                    badge.className = "status-badge danger";
                    badge.innerHTML = `<i class="fa-solid fa-fire"></i> <span>BURN</span>`;
                } else {
                    badge.className = "status-badge unknown"; 
                    badge.style.background = "#e5e7eb";
                    badge.style.color = "#6b7280";
                    badge.innerHTML = `<i class="fa-solid fa-circle-question"></i> <span>UNKNOWN</span>`;
                }
            }
        }
    } catch (e) {
        console.warn("Không phải JSON hợp lệ:", event.data);
    }
}

// ==================== KHỞI TẠO ĐỒ THỊ CHART.JS ====================
let chartTemp, chartHumi, chartGas;
const maxDataPoints = 50; // Giữ 50 điểm trên đồ thị

function initCharts() {
    const commonOptions = {
        responsive: true, maintainAspectRatio: false, animation: false,
        plugins: { legend: { display: false }, tooltip: { enabled: false } },
        elements: { 
            point: { radius: 0 }, 
            line: { tension: 0.4, borderWidth: 2.5 } 
        },
        layout: { padding: { top: 5, bottom: 0, left: 0, right: 0 } }
    };

    // Hàm tạo trục Y KHÔNG FIX CỨNG MIN/MAX, để Chart.js tự động co giãn
    function getYScale() {
        return {
            display: true,
            position: 'left', 
            border: { display: false }, 
            grid: { color: '#f3f4f6' }, 
            ticks: { color: '#9ca3af', font: { family: "'Poppins', sans-serif", size: 10 } },
            grace: '20%' // Tạo thêm 20% khoảng không gian "thở" ở trên/dưới đỉnh sóng
        };
    }

    function createGradient(ctx, colorStart, colorEnd) {
        let gradient = ctx.createLinearGradient(0, 0, 0, 120); 
        gradient.addColorStop(0, colorStart); gradient.addColorStop(1, colorEnd);
        return gradient;
    }

    // 1. Đồ thị Nhiệt độ
    let ctxTemp = document.getElementById('chartTemp').getContext('2d');
    chartTemp = new Chart(ctxTemp, {
        type: 'line',
        data: { labels: [], datasets: [{ data: [], borderColor: '#fcd34d', backgroundColor: createGradient(ctxTemp, 'rgba(252, 211, 77, 0.4)', 'rgba(252, 211, 77, 0)'), fill: true }] },
        options: { ...commonOptions, scales: { x: { display: false }, y: getYScale() } }
    });

    // 2. Đồ thị Độ ẩm
    let ctxHumi = document.getElementById('chartHumi').getContext('2d');
    chartHumi = new Chart(ctxHumi, {
        type: 'line',
        data: { labels: [], datasets: [{ data: [], borderColor: '#93c5fd', backgroundColor: createGradient(ctxHumi, 'rgba(147, 197, 253, 0.4)', 'rgba(147, 197, 253, 0)'), fill: true }] },
        options: { ...commonOptions, scales: { x: { display: false }, y: getYScale() } }
    });

    // 3. Đồ thị Khí Gas
    let ctxGas = document.getElementById('chartGas').getContext('2d');
    chartGas = new Chart(ctxGas, {
        type: 'line',
        data: { labels: [], datasets: [{ data: [], borderColor: '#86efac', backgroundColor: createGradient(ctxGas, 'rgba(134, 239, 172, 0.4)', 'rgba(134, 239, 172, 0)'), fill: true }] },
        options: { ...commonOptions, scales: { x: { display: false }, y: getYScale() } }
    });
}

function updateChart(chart, newData) {
    const timeNow = new Date().toLocaleTimeString();
    chart.data.labels.push(timeNow);
    chart.data.datasets[0].data.push(newData);
    
    // Trượt đồ thị khi quá số điểm tối đa
    if (chart.data.labels.length > maxDataPoints) {
        chart.data.labels.shift(); 
        chart.data.datasets[0].data.shift();
    }
    chart.update();
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
    const telegram_bot_token = document.getElementById("telegramBotToken").value.trim();
    const telegram_chat_id = document.getElementById("telegramChatId").value.trim();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: ssid,
            password: password,
            token: token,
            server: server,
            port: port,
            telegram_bot_token: telegram_bot_token,
            telegram_chat_id: telegram_chat_id
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

function updateClock() {
    const clockEl = document.getElementById('clock');
    if (clockEl) {
        const now = new Date();
        const timeString = now.toLocaleTimeString('vi-VN', { hour12: false });
        const dateString = now.toLocaleDateString('vi-VN');
        clockEl.innerText = `${timeString} | ${dateString}`;
    }
}
// Chạy ngay khi tải trang và lặp lại mỗi 1 giây
updateClock();
setInterval(updateClock, 1000);