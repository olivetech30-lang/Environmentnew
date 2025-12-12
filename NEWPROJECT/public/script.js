// API Base URL — update for production
const API_BASE = window.location.origin;

// DOM Elements
const tempValueEl = document.getElementById('temp-value');
const humValueEl = document.getElementById('hum-value');
const tempStatusEl = document.getElementById('temp-status');
const humStatusEl = document.getElementById('hum-status');
const heatStatusEl = document.getElementById('heat-status');
const readingsBody = document.getElementById('readings-body');

let lastTemp = null;
let lastHum = null;

// Fetch and update latest data
async function fetchLatest() {
  try {
    const res = await fetch(`${API_BASE}/api/latest`);
    const data = await res.json();

    if (data.temperature !== undefined && data.humidity !== undefined) {
      tempValueEl.textContent = `${data.temperature}°C`;
      humValueEl.textContent = `${data.humidity}%`;

      // Update insights
      updateInsights(data.temperature, data.humidity);
    }
  } catch (e) {
    console.error('Failed to fetch latest:', e);
  }
}

function updateInsights(temp, hum) {
  // Temperature status
  if (temp < 18) tempStatusEl.textContent = 'Below comfortable';
  else if (temp > 26) tempStatusEl.textContent = 'Too warm';
  else tempStatusEl.textContent = 'Comfortable';

  // Humidity status
  if (hum >= 40 && hum <= 60) humStatusEl.textContent = 'Optimal';
  else if (hum < 40) humStatusEl.textContent = 'Dry';
  else humStatusEl.textContent = 'Humid';

  // Heat index (simplified)
  if (temp > 30) heatStatusEl.textContent = 'High';
  else heatStatusEl.textContent = 'Moderate';
}

// Fetch and render change log
async function fetchReadings() {
  try {
    const res = await fetch(`${API_BASE}/api/readings`);
    const readings = await res.json();

    // Clear table
    readingsBody.innerHTML = '';

    // Render in reverse chronological order
    readings.slice().reverse().forEach(r => {
      const row = document.createElement('tr');
      const time = new Date(r.timestamp).toLocaleString();
      row.innerHTML = `
        <td>${time}</td>
        <td>${r.temperature}</td>
        <td>${r.humidity}</td>
      `;
      readingsBody.appendChild(row);
    });
  } catch (e) {
    console.error('Failed to fetch readings:', e);
  }
}

// Initial load + polling
fetchLatest();
fetchReadings();

setInterval(() => {
  fetchLatest();
  fetchReadings();
}, 5000); // Update every 5 seconds