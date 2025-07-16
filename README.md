MyTempBridge (ESP32 + HomeSpan)

An ESP32-based HomeKit bridge that polls CGMiner stats and exposes four virtual temperature sensors:
- Chip-One, Chip-Two, Chip-Three (from MTavg)
- Air-Inlet (from Temp)

---

## 🔧 Setup Instructions

1. **Clone or download** this repo.
2. Open `MyTempBridge.ino` in **Arduino IDE**.
3. Install **HomeSpan** and **WiFi** libraries if prompted.
4. Replace placeholders:
   - SSD: `<YOUR_WIFI_SSID>`
   - Password: `<YOUR_WIFI_PASSWORD>`
   - CGMiner IP and Port
5. Choose **ESP32 board** (+ correct COM port) and click **Upload**.
6. After upload, add accessory in Apple Home app using default setup code 46637726
7. Check that temperature values update every 30s.

---

## 🛠 How it works

- Wi-Fi connects to your network.
- Every 30 seconds it queries CGMiner (`{"command":"stats"}`).
- Zones `MTavg[...]` and `Temp[...]` are extracted and parsed.
- Parsed values update HomeKit sensors.

---

## ⚙️ Customization

- Change polling frequency via `INTERVAL_MS`.
- Adjust HomeKit accessory names (`"Chip-One"` etc.).
- Add more sensors by copying `MyTempSensor`.
