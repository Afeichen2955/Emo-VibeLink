"""
情绪感知互动光影系统 - Python后端
使用Flask + WebSocket实现实时数据传输
"""

from flask import Flask, render_template, jsonify, request
from flask_socketio import SocketIO, emit
import serial
import serial.tools.list_ports
import json
import threading
import time
import random
from datetime import datetime
import numpy as np

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app, cors_allowed_origins="*")

# 全局变量存储系统状态
class PetSystem:
    def __init__(self):
        self.mood = 50  # 情绪值 0-100
        self.energy = 70  # 能量值 0-100
        self.happiness = 60  # 快乐值 0-100
        self.environment = {
            'temperature': 25,
            'humidity': 50,
            'light_level': 50,
            'motion_detected': False,
            'sound_level': 30
        }
        self.interaction_history = []
        self.arduino_connected = False
        self.serial_port = None
        
    def calculate_mood(self):
        """根据环境和互动计算宠物情绪"""
        # 温度舒适度（20-26度最舒适）
        temp_comfort = 100 - abs(self.environment['temperature'] - 23) * 5
        temp_comfort = max(0, min(100, temp_comfort))
        
        # 光线舒适度
        light_comfort = 100 - abs(self.environment['light_level'] - 60) * 2
        light_comfort = max(0, min(100, light_comfort))
        
        # 互动频率影响
        recent_interactions = len([i for i in self.interaction_history 
                                 if time.time() - i < 60])  # 最近1分钟的互动
        interaction_bonus = min(30, recent_interactions * 10)
        
        # 综合计算情绪
        self.mood = (temp_comfort * 0.3 + 
                    light_comfort * 0.3 + 
                    self.happiness * 0.3 + 
                    interaction_bonus * 0.1)
        
        # 情绪影响能量
        if self.mood > 70:
            self.energy = min(100, self.energy + 0.5)
        elif self.mood < 30:
            self.energy = max(0, self.energy - 0.5)
            
        return self.mood
    
    def get_pet_state(self):
        """获取宠物当前状态描述"""
        if self.mood >= 80:
            return "ecstatic", "你的虚拟伙伴感到非常快乐！"
        elif self.mood >= 60:
            return "happy", "你的虚拟伙伴心情不错~"
        elif self.mood >= 40:
            return "neutral", "你的虚拟伙伴感觉还行"
        elif self.mood >= 20:
            return "sad", "你的虚拟伙伴有点沮丧..."
        else:
            return "upset", "你的虚拟伙伴需要关怀！"

pet_system = PetSystem()

def connect_arduino():
    """自动连接Arduino"""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'Arduino' in port.description or 'USB' in port.description:
            try:
                pet_system.serial_port = serial.Serial(port.device, 9600, timeout=1)
                pet_system.arduino_connected = True
                print(f"Arduino连接成功: {port.device}")
                return True
            except:
                continue
    return False

def read_arduino_data():
    """持续读取Arduino数据"""
    while True:
        if pet_system.arduino_connected and pet_system.serial_port:
            try:
                if pet_system.serial_port.in_waiting:
                    line = pet_system.serial_port.readline().decode('utf-8').strip()
                    if line:
                        process_sensor_data(line)
            except:
                pet_system.arduino_connected = False
                time.sleep(5)
                connect_arduino()
        else:
            time.sleep(5)
            connect_arduino()
        time.sleep(0.1)

def process_sensor_data(data):
    """处理从Arduino接收的传感器数据"""
    try:
        # 解析格式: "TEMP:25,HUM:50,LIGHT:300,MOTION:1,ACCEL:0.5"
        values = {}
        for item in data.split(','):
            if ':' in item:
                key, value = item.split(':')
                values[key] = float(value)
        
        # 更新环境数据
        if 'TEMP' in values:
            pet_system.environment['temperature'] = values['TEMP']
        if 'HUM' in values:
            pet_system.environment['humidity'] = values['HUM']
        if 'LIGHT' in values:
            pet_system.environment['light_level'] = min(100, values['LIGHT'] / 10)
        if 'MOTION' in values:
            pet_system.environment['motion_detected'] = values['MOTION'] > 0
            if values['MOTION'] > 0:
                pet_system.interaction_history.append(time.time())
        
        # 计算新的情绪值
        pet_system.calculate_mood()
        
        # 发送更新到前端
        socketio.emit('sensor_update', {
            'environment': pet_system.environment,
            'mood': pet_system.mood,
            'energy': pet_system.energy,
            'happiness': pet_system.happiness,
            'state': pet_system.get_pet_state()[0],
            'message': pet_system.get_pet_state()[1]
        })
        
        # 控制LED灯带
        control_led_strip()
        
    except Exception as e:
        print(f"数据处理错误: {e}")

def control_led_strip():
    """根据情绪控制LED灯带"""
    if pet_system.arduino_connected and pet_system.serial_port:
        # 根据情绪设置颜色
        if pet_system.mood >= 80:
            color = "GREEN"  # 开心-绿色
        elif pet_system.mood >= 60:
            color = "BLUE"   # 愉快-蓝色
        elif pet_system.mood >= 40:
            color = "YELLOW" # 一般-黄色
        else:
            color = "RED"    # 沮丧-红色
        
        # 发送命令到Arduino
        try:
            command = f"LED:{color},{int(pet_system.mood)}\n"
            pet_system.serial_port.write(command.encode())
        except:
            pass

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/status')
def get_status():
    return jsonify({
        'mood': pet_system.mood,
        'energy': pet_system.energy,
        'happiness': pet_system.happiness,
        'environment': pet_system.environment,
        'state': pet_system.get_pet_state()[0],
        'message': pet_system.get_pet_state()[1],
        'arduino_connected': pet_system.arduino_connected
    })

@socketio.on('connect')
def handle_connect():
    emit('connected', {'data': 'Connected to server'})

@socketio.on('user_interaction')
def handle_interaction(data):
    """处理用户交互"""
    interaction_type = data.get('type')
    
    if interaction_type == 'feed':
        pet_system.energy = min(100, pet_system.energy + 20)
        pet_system.happiness = min(100, pet_system.happiness + 10)
    elif interaction_type == 'play':
        pet_system.happiness = min(100, pet_system.happiness + 15)
        pet_system.energy = max(0, pet_system.energy - 10)
    elif interaction_type == 'pet':
        pet_system.happiness = min(100, pet_system.happiness + 5)
    
    pet_system.interaction_history.append(time.time())
    pet_system.calculate_mood()
    
    # 发送舵机控制命令
    if pet_system.arduino_connected and pet_system.serial_port:
        try:
            command = f"SERVO:{interaction_type}\n"
            pet_system.serial_port.write(command.encode())
        except:
            pass
    
    emit('interaction_response', {
        'type': interaction_type,
        'mood': pet_system.mood,
        'energy': pet_system.energy,
        'happiness': pet_system.happiness
    }, broadcast=True)

@socketio.on('remote_control')
def handle_remote(data):
    """处理遥控器输入"""
    button = data.get('button')
    # 这里可以添加遥控器按键映射逻辑
    print(f"Remote button pressed: {button}")

# 模拟数据生成（用于测试，没有Arduino时）
def simulate_sensor_data():
    """模拟传感器数据用于测试"""
    while not pet_system.arduino_connected:
        # 模拟温度变化
        pet_system.environment['temperature'] += random.uniform(-0.5, 0.5)
        pet_system.environment['temperature'] = max(15, min(35, pet_system.environment['temperature']))
        
        # 模拟光线变化
        pet_system.environment['light_level'] += random.uniform(-5, 5)
        pet_system.environment['light_level'] = max(0, min(100, pet_system.environment['light_level']))
        
        # 随机运动检测
        pet_system.environment['motion_detected'] = random.random() > 0.8
        
        if pet_system.environment['motion_detected']:
            pet_system.interaction_history.append(time.time())
        
        # 计算情绪
        pet_system.calculate_mood()
        
        # 发送到前端
        socketio.emit('sensor_update', {
            'environment': pet_system.environment,
            'mood': pet_system.mood,
            'energy': pet_system.energy,
            'happiness': pet_system.happiness,
            'state': pet_system.get_pet_state()[0],
            'message': pet_system.get_pet_state()[1]
        })
        
        time.sleep(2)

if __name__ == '__main__':
    # 启动Arduino数据读取线程
    arduino_thread = threading.Thread(target=read_arduino_data, daemon=True)
    arduino_thread.start()
    
    # 启动模拟数据线程（测试用）
    simulation_thread = threading.Thread(target=simulate_sensor_data, daemon=True)
    simulation_thread.start()
    
    # 启动Flask应用
    socketio.run(app, debug=True, host='0.0.0.0', port=5000)
