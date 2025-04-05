# 🌙 Nocta – Smart Ambient Sleep Controller

Nocta is an embedded solution designed to **optimize your sleep environment** in real-time by monitoring key ambient parameters and automatically adjusting connected systems to suit your current sleep cycle stage.

Sleep better. Wake better. Live better.

---

## 🛏️ What Is Nocta?

**Nocta** is a smart environmental controller that uses sensors and actuators to **analyze and adapt** your room’s conditions during sleep. It enhances your rest by tailoring temperature, humidity, lighting, noise, and more—depending on your sleep stage (e.g., light, deep, REM).

### 🎯 Key Features

- 🌡️ **Temperature Control**: Adjusts AC and heating systems to optimal levels.
- 💧 **Humidity Regulation**: Controls a humidifier to maintain ideal humidity.
- 🔇 **Noise Awareness**: Monitors sound levels and minimizes disturbances.
- 🌗 **Light Sensitivity**: Dims or opens blinds to align with circadian rhythms.
- 🧠 **Sleep Cycle Adaptation**: Responds dynamically to the user’s sleep stage (light/deep/REM).
- 📊 **Real-time Monitoring**: Live tracking of ambient conditions via connected sensors.

---

## 🧠 How It Works

1. **Sensor Array**: Collects data on light, sound, humidity, and temperature.
2. **Sleep Stage Input**: Sleep cycle stage is detected via a wearable or app, or estimated based on trends.
3. **Controller**: An embedded microcontroller (ESP32-based) processes data and makes environment decisions.
4. **Actuation**: Commands are sent to:
   - Air conditioner or heating unit
   - Humidifier
   - Smart blinds or lights
5. **Feedback Loop**: Adjustments continue through the night, based on sensor feedback and expected needs per sleep stage.

---

## ⚙️ Technologies Used

- **Hardware**: ESP32, DHT22 / BME280, LDR, sound sensor, relays or smart plugs, motor/servo for blinds
- **Software**: C++ (Arduino framework), MQTT/Wi-Fi communication, optional mobile app integration
- **Optional Integrations**: Sleep tracking devices (e.g., Fitbit, Oura, etc.), Home Assistant, Apple Health

---

## 🧪 Current Status

- [x] Sensor module implemented  
- [x] Basic AC/heater control  
- [x] Humidifier integration  
- [x] Light/blind control  
- [ ] Sleep cycle input integration  
- [ ] Adaptive control logic per cycle  
- [ ] Mobile/dashboard visualization  

---

## 🚀 Getting Started

1. **Clone this repo**
   ```bash
   git clone https://github.com/your-org/nocta.git
   cd nocta
