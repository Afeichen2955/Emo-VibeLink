/*
  情绪感知互动光影系统 - Arduino端代码
  支持XIAO ESP32C3 / XIAO nRF52840 Sense / XIAO SAMD21
*/

#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <DHT.h>
#include <Wire.h>

// 引脚定义
#define LED_STRIP_PIN 6      // LED灯带数据引脚
#define SERVO_PIN 9          // 舵机控制引脚
#define DHT_PIN 2           // 温湿度传感器引脚
#define PIR_PIN 3           // PIR运动传感器引脚
#define LIGHT_SENSOR_PIN A0 // 光敏传感器引脚
#define KNOB_PIN A1         // 旋钮传感器引脚
#define IR_RECEIVER_PIN 7   // 红外接收器引脚

// LED灯带设置
#define LED_COUNT 30        // LED数量（1米灯带约30个）
Adafruit_NeoPixel strip(LED_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// 舵机设置
Servo petServo;

// 温湿度传感器设置
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// 加速度传感器（如果使用XIAO nRF52840 Sense的内置LSM6DS3）
#ifdef USE_LSM6DS3
  #include <Arduino_LSM6DS3.h>
#endif

// 全局变量
float temperature = 25.0;
float humidity = 50.0;
int lightLevel = 500;
bool motionDetected = false;
float accelX = 0, accelY = 0, accelZ = 0;
int knobValue = 0;
unsigned long lastSensorRead = 0;
unsigned long lastDataSend = 0;

// LED动画状态
int currentColor[3] = {0, 255, 0}; // RGB
int targetColor[3] = {0, 255, 0};
float brightness = 0.5;
unsigned long lastLedUpdate = 0;
int animationMode = 0; // 0=呼吸灯, 1=流动, 2=闪烁

void setup() {
  Serial.begin(9600);
  
  // 初始化LED灯带
  strip.begin();
  strip.show();
  strip.setBrightness(128);
  
  // 初始化舵机
  petServo.attach(SERVO_PIN);
  petServo.write(90); // 中间位置
  
  // 初始化传感器
  dht.begin();
  pinMode(PIR_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(KNOB_PIN, INPUT);
  pinMode(IR_RECEIVER_PIN, INPUT);
  
  // 初始化加速度传感器（如果可用）
  #ifdef USE_LSM6DS3
    if (!IMU.begin()) {
      Serial.println("Failed to initialize IMU!");
    }
  #endif
  
  Serial.println("System initialized!");
  
  // 启动动画
  startupAnimation();
}

void loop() {
  unsigned long currentTime = millis();
  
  // 每500ms读取传感器
  if (currentTime - lastSensorRead >= 500) {
    readSensors();
    lastSensorRead = currentTime;
  }
  
  // 每100ms发送数据到Python
  if (currentTime - lastDataSend >= 100) {
    sendSensorData();
    lastDataSend = currentTime;
  }
  
  // 更新LED动画
  if (currentTime - lastLedUpdate >= 50) {
    updateLedAnimation();
    lastLedUpdate = currentTime;
  }
  
  // 处理串口命令
  if (Serial.available()) {
    processCommand();
  }
}

void readSensors() {
  // 读取温湿度
  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();
  if (!isnan(newTemp) && !isnan(newHum)) {
    temperature = newTemp;
    humidity = newHum;
  }
  
  // 读取光线
  lightLevel = analogRead(LIGHT_SENSOR_PIN);
  
  // 读取运动
  motionDetected = digitalRead(PIR_PIN);
  
  // 读取旋钮
  knobValue = analogRead(KNOB_PIN);
  
  // 读取加速度（如果可用）
  #ifdef USE_LSM6DS3
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(accelX, accelY, accelZ);
    }
  #endif
}

void sendSensorData() {
  // 发送格式: "TEMP:25,HUM:50,LIGHT:300,MOTION:1,KNOB:512,ACCEL:0.5"
  Serial.print("TEMP:");
  Serial.print(temperature);
  Serial.print(",HUM:");
  Serial.print(humidity);
  Serial.print(",LIGHT:");
  Serial.print(lightLevel);
  Serial.print(",MOTION:");
  Serial.print(motionDetected ? 1 : 0);
  Serial.print(",KNOB:");
  Serial.print(knobValue);
  Serial.print(",ACCEL:");
  Serial.print(sqrt(accelX*accelX + accelY*accelY + accelZ*accelZ));
  Serial.println();
}

void processCommand() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  
  // 解析命令格式: "LED:COLOR,BRIGHTNESS" 或 "SERVO:ACTION"
  if (command.startsWith("LED:")) {
    String params = command.substring(4);
    int commaIndex = params.indexOf(',');
    if (commaIndex > 0) {
      String color = params.substring(0, commaIndex);
      int brightnessValue = params.substring(commaIndex + 1).toInt();
      
      setLedColor(color);
      brightness = brightnessValue / 100.0;
      strip.setBrightness(brightness * 255);
    }
  }
  else if (command.startsWith("SERVO:")) {
    String action = command.substring(6);
    performServoAction(action);
  }
  else if (command.startsWith("ANIM:")) {
    String mode = command.substring(5);
    if (mode == "BREATH") animationMode = 0;
    else if (mode == "FLOW") animationMode = 1;
    else if (mode == "BLINK") animationMode = 2;
  }
}

void setLedColor(String color) {
  if (color == "RED") {
    targetColor[0] = 255; targetColor[1] = 0; targetColor[2] = 0;
  } else if (color == "GREEN") {
    targetColor[0] = 0; targetColor[1] = 255; targetColor[2] = 0;
  } else if (color == "BLUE") {
    targetColor[0] = 0; targetColor[1] = 0; targetColor[2] = 255;
  } else if (color == "YELLOW") {
    targetColor[0] = 255; targetColor[1] = 255; targetColor[2] = 0;
  } else if (color == "PURPLE") {
    targetColor[0] = 128; targetColor[1] = 0; targetColor[2] = 128;
  } else if (color == "CYAN") {
    targetColor[0] = 0; targetColor[1] = 255; targetColor[2] = 255;
  } else if (color == "WHITE") {
    targetColor[0] = 255; targetColor[1] = 255; targetColor[2] = 255;
  }
}

void performServoAction(String action) {
  if (action == "feed") {
    // 喂食动作：上下摆动
    for (int i = 0; i < 3; i++) {
      petServo.write(60);
      delay(200);
      petServo.write(120);
      delay(200);
    }
    petServo.write(90);
  }
  else if (action == "play") {
    // 玩耍动作：快速左右摆动
    for (int i = 0; i < 5; i++) {
      petServo.write(45);
      delay(100);
      petServo.write(135);
      delay(100);
    }
    petServo.write(90);
  }
  else if (action == "pet") {
    // 抚摸动作：缓慢摆动
    for (int i = 0; i < 2; i++) {
      for (int pos = 70; pos <= 110; pos++) {
        petServo.write(pos);
        delay(10);
      }
      for (int pos = 110; pos >= 70; pos--) {
        petServo.write(pos);
        delay(10);
      }
    }
    petServo.write(90);
  }
  else if (action == "happy") {
    // 开心：快速小幅摆动
    for (int i = 0; i < 10; i++) {
      petServo.write(85);
      delay(50);
      petServo.write(95);
      delay(50);
    }
    petServo.write(90);
  }
  else if (action == "sad") {
    // 沮丧：缓慢下垂
    for (int pos = 90; pos >= 45; pos--) {
      petServo.write(pos);
      delay(20);
    }
    delay(1000);
    petServo.write(90);
  }
}

void updateLedAnimation() {
  // 平滑过渡到目标颜色
  for (int i = 0; i < 3; i++) {
    if (currentColor[i] < targetColor[i]) {
      currentColor[i] = min(currentColor[i] + 5, targetColor[i]);
    } else if (currentColor[i] > targetColor[i]) {
      currentColor[i] = max(currentColor[i] - 5, targetColor[i]);
    }
  }
  
  switch (animationMode) {
    case 0: // 呼吸灯效果
      breathAnimation();
      break;
    case 1: // 流动效果
      flowAnimation();
      break;
    case 2: // 闪烁效果
      blinkAnimation();
      break;
    default:
      solidColor();
  }
  
  strip.show();
}

void breathAnimation() {
  static float phase = 0;
  phase += 0.05;
  float breathBrightness = (sin(phase) + 1) / 2; // 0到1之间
  
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, 
      currentColor[0] * breathBrightness,
      currentColor[1] * breathBrightness,
      currentColor[2] * breathBrightness
    );
  }
}

void flowAnimation() {
  static int offset = 0;
  offset = (offset + 1) % LED_COUNT;
  
  for (int i = 0; i < LED_COUNT; i++) {
    float intensity = (sin((i + offset) * 0.3) + 1) / 2;
    strip.setPixelColor(i,
      currentColor[0] * intensity,
      currentColor[1] * intensity,
      currentColor[2] * intensity
    );
  }
}

void blinkAnimation() {
  static bool on = true;
  static unsigned long lastBlink = 0;
  
  if (millis() - lastBlink > 500) {
    on = !on;
    lastBlink = millis();
  }
  
  if (on) {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, currentColor[0], currentColor[1], currentColor[2]);
    }
  } else {
    strip.clear();
  }
}

void solidColor() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, currentColor[0], currentColor[1], currentColor[2]);
  }
}

void startupAnimation() {
  // 启动时的彩虹效果
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < LED_COUNT; i++) {
      int pixelHue = (i * 65536L / LED_COUNT) + (j * 256);
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    delay(10);
  }
  
  // 清空
  strip.clear();
  strip.show();
}
