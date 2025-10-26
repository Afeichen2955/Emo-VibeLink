## 🌟 Emotion-Sensing Interactive Light and Sound System

An innovative IoT interactive project that uses Arduino sensors to collect environmental data, creating a virtual pet system capable of perceiving and responding to environmental changes.

## 📋 Project Features

### Core Functions

1.  **Multi-Sensor Environmental Perception**

      - Temperature and Humidity Monitoring (DHT22)
      - Light Intensity Detection (Photoresistor Sensor)
      - Motion Detection (PIR Sensor)
      - Acceleration Sensing (3-axis Accelerometer)
      - Knob Control Input

2.  **Intelligent Emotion System**

      - Emotion calculation based on environmental parameters
      - Dynamic emotional states (Joyful, Happy, Normal, Gloomy, Sad)
      - Emotions affect Energy and Happiness values

3.  **Real-Time Interactive Feedback**

      - LED strip color changes with emotion
      - Servo motor action response
      - Real-time animation on the web interface
      - WebSocket for bi-directional communication

4.  **Rich Visual Effects**

      - Dynamic virtual pet animation
      - Particle effect system
      - Breathing, flowing, and flashing LED effects
      - Responsive UI design

5.  **Emotional Soundscape (New Music Module)**

      - Buzzer/Speaker outputs simple tones and rhythms mapped to the pet's current emotion (e.g., high pitch for Joyful, low and slow for Sad).

## 🛠️ Hardware Requirements

### Essential Components

  - Seeed XIAO Development Board (ESP32C3/nRF52840 Sense/SAMD21)
  - 1 Meter Waterproof LED Strip (WS2812B)
  - Servo Motor
  - DHT22 Temperature and Humidity Sensor
  - PIR Motion Sensor
  - Photoresistor Sensor
  - Rotary Knob Sensor
  - **Buzzer or Mini Speaker Module** (For Emotional Soundscape)

### Optional Components

  - 20-key Mini Remote Control + IR Receiver
  - 3-axis Accelerometer
  - 4-color LED Module
  - XIAO Multi-functional Expansion Board

## 📦 Software Installation

### 1\. Python Environment Setup

```bash
# Install Python dependencies
pip install -r requirements.txt
```

### 2\. Arduino Setup

1.  Install Arduino IDE
2.  Install necessary libraries:
      - Adafruit NeoPixel
      - DHT sensor library
      - Servo
3.  Select the correct board type for your development board
4.  Upload `arduino_code.ino` to the development board

### 3\. Hardware Connection

#### Pin Definitions

| Component | Arduino Pin |
| :--- | :--- |
| LED Strip | D6 |
| Servo Motor | D9 |
| DHT22 | D2 |
| PIR Sensor | D3 |
| Photoresistor | A0 |
| Knob | A1 |
| IR Receiver | D7 |
| **Buzzer/Speaker** | **D8 (Example Pin)** |

## 🚀 Running the Project

### 1\. Start the Python Server

```bash
python app.py
```

The server will start at http://localhost:5000

### 2\. Access the Web Interface

Open your browser and visit http://localhost:5000

### 3\. Start Interacting

  - Click "Feed" to increase energy
  - Click "Play" to increase happiness value
  - Click "Pet" to soothe the pet
  - Observe how environmental changes affect the pet's emotion

## 🎨 System Working Principle

### Data Flow

1.  **Sensor Acquisition** → Arduino reads all sensor data
2.  **Serial Communication** → Data is sent to Python via USB
3.  **Data Processing** → Python calculates the emotional state
4.  **WebSocket Push** → Web interface is updated in real-time
5.  **Feedback Control** → Controls LED and Servo responses **and Buzzer tone/rhythm**

### Emotion Algorithm

```python
Emotion_Value = Temp_Comfort_Score * 0.3 + Light_Comfort_Score * 0.3 + 
                Happiness_Value * 0.3 + Interaction_Frequency * 0.1
```

### LED Color Mapping

  - 🟢 Green: Very Happy (Emotion Value $\ge$ 80)
  - 🔵 Blue: Happy (Emotion Value $\ge$ 60)
  - 🟡 Yellow: Normal (Emotion Value $\ge$ 40)
  - 🔴 Red: Gloomy (Emotion Value \< 40)

## 🔧 Custom Configuration

### Modify Sensor Thresholds

Adjust the following in `app.py`:

```python
# Temperature Comfort Range
TEMP_COMFORT_MIN = 20
TEMP_COMFORT_MAX = 26

# Ideal Light Value
LIGHT_IDEAL = 60
```

### Add New Interaction Actions

1.  Add a button to the webpage
2.  Add processing logic in Python
3.  Add corresponding servo **or sound** action in Arduino

### LED Animation Modes

Switch modes by sending commands via serial:

  - `ANIM:BREATH` - Breathing light
  - `ANIM:FLOW` - Flowing effect
  - `ANIM:BLINK` - Flashing effect

## 📊 Data Monitoring

The system provides real-time data monitoring:

  - Environmental Temperature/Humidity
  - Light Intensity
  - Motion Detection Status
  - Emotion/Energy/Happiness Value trends

## 🎮 Extended Gameplay

### 1\. Music Interaction

Integrate a sound sensor to let the pet respond to music rhythm.

### 2\. Weather Linkage

Connect to a weather API to let outside weather affect the pet's mood.

### 3\. Multi-User Interaction

Add a user system for multiple people to collectively care for the virtual pet.

### 4\. AI Dialogue

Integrate voice recognition for voice interaction with the pet.

### 5\. Mobile Control

Develop a mobile app for remote pet care.

## 🐛 Troubleshooting

### Arduino Connection Failure

1.  Check USB connection
2.  Confirm the correct serial port
3.  Check baud rate setting (9600)

### No Sensor Data

1.  Check wiring correctness
2.  Verify sensor power supply (3.3V or 5V)
3.  Test individual sensor functionality

### Webpage Inaccessible

1.  Confirm the Python server is running
2.  Check firewall settings
3.  Try using 127.0.0.1 instead of localhost

## 📝 Project Structure

```
emotion-sensing-system/
├── app.py                 # Python Backend Server
├── arduino_code.ino       # Arduino Firmware
├── templates/
│   └── index.html        # Web Interface
├── requirements.txt       # Python Dependencies
└── README.md             # Project Documentation
```

## 🌈 Future Plans

  - [ ] Add data storage and historical records
  - [ ] Implement machine learning for emotion prediction
  - [ ] Support multiple virtual pets
  - [ ] Add social sharing features
  - [ ] Develop a mobile application

## 📄 Open Source License

MIT License

## 🤝 Contribution Guide

Feel free to submit Issues and Pull Requests\!

## 💡 Creative Source

This project aims to explore the combination of IoT and emotional computing, creating an intelligent system that can perceive the environment and express emotions through hardware sensors and software algorithms, bringing a new interactive experience to users.

-----

🎉 Enjoy the fun of interacting with your virtual companion\!
