'use strict';

// ── Constants ─────────────────────────────────────────────────────────────
const GAUGE_R   = 48;
const GAUGE_C   = 2 * Math.PI * GAUGE_R;   // ≈ 301.59
const GAUGE_ARC = GAUGE_C * 0.75;          // 270° ≈ 226.19

const STATUS_MAP = {
  0: { label: 'NORMAL',   cls: 'normal',  icon: '✅' },
  1: { label: 'GAS LEAK', cls: 'gas',     icon: '⚠️' },
  2: { label: 'BURN',     cls: 'burn',    icon: '🔥' },
};

const CHART_MAX_PTS = 60;

// ── State ─────────────────────────────────────────────────────────────────
const chartLabels  = [];
const chartTemp    = [];
const chartHumi    = [];
const chartSmoke   = [];

// ── Chart.js setup ────────────────────────────────────────────────────────
const chartTH = new Chart(document.getElementById('chartTH'), {
  type: 'line',
  data: {
    labels: chartLabels,
    datasets: [
      {
        label: 'Nhiệt độ (°C)',
        data: chartTemp,
        borderColor: '#ff7043',
        backgroundColor: 'rgba(255,112,67,.08)',
        borderWidth: 2,
        pointRadius: 0,
        tension: 0.3,
        yAxisID: 'yTemp',
        fill: true,
      },
      {
        label: 'Độ ẩm (%)',
        data: chartHumi,
        borderColor: '#29b6f6',
        backgroundColor: 'rgba(41,182,246,.08)',
        borderWidth: 2,
        pointRadius: 0,
        tension: 0.3,
        yAxisID: 'yHumi',
        fill: true,
      },
    ],
  },
  options: {
    responsive: true,
    interaction: { mode: 'index', intersect: false },
    animation: { duration: 300 },
    scales: {
      x: {
        grid: { color: '#21262d' },
        ticks: { color: '#8b949e', maxTicksLimit: 8, font: { size: 10 } },
      },
      yTemp: {
        type: 'linear', position: 'left',
        grid: { color: '#21262d' },
        ticks: { color: '#ff7043', font: { size: 10 } },
        title: { display: true, text: '°C', color: '#ff7043', font: { size: 10 } },
      },
      yHumi: {
        type: 'linear', position: 'right',
        min: 0, max: 100,
        grid: { drawOnChartArea: false },
        ticks: { color: '#29b6f6', font: { size: 10 } },
        title: { display: true, text: '%', color: '#29b6f6', font: { size: 10 } },
      },
    },
    plugins: {
      legend: { labels: { color: '#e6edf3', font: { size: 11 } } },
    },
  },
});

const chartSmokeChart = new Chart(document.getElementById('chartSmoke'), {
  type: 'line',
  data: {
    labels: chartLabels,
    datasets: [
      {
        label: 'Khí ADC',
        data: chartSmoke,
        borderColor: '#ab47bc',
        backgroundColor: 'rgba(171,71,188,.08)',
        borderWidth: 2,
        pointRadius: 0,
        tension: 0.3,
        fill: true,
      },
    ],
  },
  options: {
    responsive: true,
    animation: { duration: 300 },
    scales: {
      x: {
        grid: { color: '#21262d' },
        ticks: { color: '#8b949e', maxTicksLimit: 8, font: { size: 10 } },
      },
      y: {
        min: 0, max: 4095,
        grid: { color: '#21262d' },
        ticks: { color: '#ab47bc', font: { size: 10 } },
      },
    },
    plugins: {
      legend: { labels: { color: '#e6edf3', font: { size: 11 } } },
    },
  },
});

// ── Gauge helpers ─────────────────────────────────────────────────────────
function setGauge(fillId, value, min, max) {
  const pct    = Math.max(0, Math.min(1, (value - min) / (max - min)));
  const filled = GAUGE_ARC * pct;
  const gap    = GAUGE_C - filled;
  document.getElementById(fillId).style.strokeDasharray = `${filled} ${gap}`;
}

function setSmokeGaugeColor(smokeValue) {
  const el = document.getElementById('gSmokeFill');
  el.classList.remove('smoke-low', 'smoke-mid', 'smoke-high');
  if      (smokeValue < 500)  el.classList.add('smoke-low');
  else if (smokeValue < 2000) el.classList.add('smoke-mid');
  else                        el.classList.add('smoke-high');
}

// ── Status helpers ────────────────────────────────────────────────────────
function updateStatus(result) {
  const info = STATUS_MAP[result] ?? { label: 'UNKNOWN', cls: 'unknown', icon: '❓' };
  const badge = document.getElementById('statusBadge');
  badge.className = `status-badge ${info.cls}`;
  document.getElementById('statusIcon').textContent  = info.icon;
  document.getElementById('statusLabel').textContent = info.label;
  document.getElementById('resultCode').textContent  = result;

  const banner = document.getElementById('alertBanner');
  if (result === 1) {
    banner.className = 'alert-banner gas';
    document.getElementById('alertIcon').textContent  = '⚠️';
    document.getElementById('alertTitle').textContent = 'CẢNH BÁO RÒ KHÍ!';
    document.getElementById('alertText').textContent  = 'Phát hiện rò rỉ khí gas – hãy kiểm tra ngay!';
  } else if (result === 2) {
    banner.className = 'alert-banner burn';
    document.getElementById('alertIcon').textContent  = '🔥';
    document.getElementById('alertTitle').textContent = 'NGUY HIỂM – CHÁY!';
    document.getElementById('alertText').textContent  = 'Phát hiện dấu hiệu cháy – sơ tán ngay lập tức!';
  } else {
    banner.className = 'alert-banner hidden';
  }
}

// ── Status badge mapping for table ───────────────────────────────────────
function statusBadgeHtml(result) {
  const info = STATUS_MAP[result] ?? { label: 'UNKNOWN', cls: 'unknown' };
  return `<span class="badge badge-${info.cls}">${info.label}</span>`;
}

// ── Main sensor data handler ──────────────────────────────────────────────
function handleSensorData(data) {
  const temp  = parseFloat(data.temperature);
  const humi  = parseFloat(data.humidity);
  const smoke = parseInt(data.smokeValue, 10);
  const res   = parseInt(data.result, 10);
  const ts    = data.timestamp || new Date().toLocaleTimeString('vi-VN');

  // Gauges
  setGauge('gTempFill',  temp,  0,   60);
  setGauge('gHumiFill',  humi,  0,  100);
  setGauge('gSmokeFill', smoke, 0, 4095);
  setSmokeGaugeColor(smoke);

  document.getElementById('gTempVal').textContent  = temp.toFixed(1);
  document.getElementById('gHumiVal').textContent  = humi.toFixed(1);
  document.getElementById('gSmokeVal').textContent = smoke;

  // Status
  updateStatus(res);

  // Last update
  document.getElementById('lastUpdate').textContent = ts;

  // Charts — push and trim
  const label = ts.length > 8 ? ts.slice(-8) : ts;
  chartLabels.push(label);
  chartTemp.push(temp);
  chartHumi.push(humi);
  chartSmoke.push(smoke);

  if (chartLabels.length > CHART_MAX_PTS) {
    chartLabels.shift();
    chartTemp.shift();
    chartHumi.shift();
    chartSmoke.shift();
  }

  chartTH.update('none');
  chartSmokeChart.update('none');

  // Table — prepend row, keep 10
  const tbody = document.getElementById('tableBody');
  const tr = document.createElement('tr');
  tr.innerHTML = `
    <td>${ts}</td>
    <td>${temp.toFixed(1)}</td>
    <td>${humi.toFixed(1)}</td>
    <td>${smoke}</td>
    <td>${statusBadgeHtml(res)}</td>
  `;
  tbody.prepend(tr);
  while (tbody.rows.length > 10) tbody.deleteRow(tbody.rows.length - 1);
}

// ── Load history on page open ─────────────────────────────────────────────
async function loadHistory() {
  try {
    const res = await fetch('/api/sensor/history?limit=60');
    const rows = await res.json();
    rows.forEach(r => handleSensorData(r));
  } catch (_) { /* server not ready yet */ }
}

async function loadStats() {
  try {
    const res  = await fetch('/api/sensor/stats');
    const data = await res.json();
    if (!data.total) return;

    document.getElementById('tMin').textContent = data.temp_min  != null ? data.temp_min.toFixed(1)  : '--';
    document.getElementById('tAvg').textContent = data.temp_avg  != null ? data.temp_avg.toFixed(1)  : '--';
    document.getElementById('tMax').textContent = data.temp_max  != null ? data.temp_max.toFixed(1)  : '--';

    document.getElementById('hMin').textContent = data.humi_min  != null ? data.humi_min.toFixed(1)  : '--';
    document.getElementById('hAvg').textContent = data.humi_avg  != null ? data.humi_avg.toFixed(1)  : '--';
    document.getElementById('hMax').textContent = data.humi_max  != null ? data.humi_max.toFixed(1)  : '--';

    document.getElementById('sMin').textContent = data.smoke_min != null ? data.smoke_min : '--';
    document.getElementById('sAvg').textContent = data.smoke_avg != null ? data.smoke_avg : '--';
    document.getElementById('sMax').textContent = data.smoke_max != null ? data.smoke_max : '--';

    document.getElementById('totalReadings').textContent = `Tổng: ${data.total} bản ghi`;
  } catch (_) {}
}

// ── WebSocket connection ──────────────────────────────────────────────────
let ws;
let reconnectTimer;

function connect() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  ws = new WebSocket(`${proto}://${location.host}/ws`);

  ws.onopen = () => {
    document.getElementById('connDot').className = 'conn-dot online';
    document.getElementById('connText').textContent = 'Đã kết nối';
    clearTimeout(reconnectTimer);
    loadStats();
  };

  ws.onmessage = ({ data }) => {
    try {
      const msg = JSON.parse(data);
      if (msg.type === 'sensor_data') {
        handleSensorData(msg);
        loadStats();
      }
    } catch (_) {}
  };

  ws.onclose = () => {
    document.getElementById('connDot').className = 'conn-dot offline';
    document.getElementById('connText').textContent = 'Mất kết nối – thử lại…';
    reconnectTimer = setTimeout(connect, 3000);
  };

  ws.onerror = () => ws.close();
}

// ── Boot ──────────────────────────────────────────────────────────────────
loadHistory().then(() => {
  loadStats();
  connect();
});
