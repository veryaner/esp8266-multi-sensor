#!/usr/bin/env python3
"""
ESP8266 Sensor Network MQTT Subscriber

This script subscribes to MQTT topics published by the ESP8266 sensor network
and displays the received sensor data in real-time.

Requirements:
    pip install paho-mqtt

Usage:
    python mqtt_subscriber.py [broker_ip] [broker_port]
"""

import paho.mqtt.client as mqtt
import json
import sys
import time
from datetime import datetime

class SensorMQTTSubscriber:
    def __init__(self, broker="localhost", port=1883):
        self.broker = broker
        self.port = port
        self.client = mqtt.Client()
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.on_disconnect = self.on_disconnect
        
        # Store latest sensor data
        self.sensor_data = {
            'temperature': None,
            'humidity': None,
            'motion': None,
            'luminescence': None,
            'timestamp': None,
            'mqtt_connected': False
        }
        
        # Statistics
        self.message_count = 0
        self.last_message_time = None
    
    def on_connect(self, client, userdata, flags, rc):
        """Called when connected to MQTT broker"""
        if rc == 0:
            print(f"✅ Connected to MQTT broker at {self.broker}:{self.port}")
            
            # Subscribe to all sensor topics (with wildcard for location)
            topics = [
                "+/temperature",
                "+/humidity", 
                "+/motion",
                "+/luminescence",
                "+/status",
                "+/all"
            ]
            
            for topic in topics:
                client.subscribe(topic)
                print(f"📡 Subscribed to {topic}")
            
            print("\n" + "="*60)
            print("🎯 Waiting for sensor data...")
            print("="*60)
            
        else:
            print(f"❌ Connection failed with code {rc}")
    
    def on_disconnect(self, client, userdata, rc):
        """Called when disconnected from MQTT broker"""
        if rc != 0:
            print(f"⚠️  Unexpected disconnection (code: {rc})")
        else:
            print("🔌 Disconnected from MQTT broker")
    
    def on_message(self, client, userdata, msg):
        """Called when a message is received"""
        self.message_count += 1
        self.last_message_time = datetime.now()
        
        topic = msg.topic
        payload = msg.payload.decode('utf-8')
        
        print(f"\n📨 Message #{self.message_count} received at {self.last_message_time.strftime('%H:%M:%S')}")
        print(f"📡 Topic: {topic}")
        print(f"📊 Payload: {payload}")
        
        # Parse location from topic (format: location/type)
        topic_parts = topic.split('/')
        if len(topic_parts) != 2:
            print(f"⚠️  Unexpected topic format: {topic}")
            return
            
        location = topic_parts[0]
        sensor_type = topic_parts[1]
        
        # Update sensor data based on topic
        if sensor_type == "all":
            try:
                data = json.loads(payload)
                self.sensor_data.update(data)
                self.display_sensor_data()
            except json.JSONDecodeError:
                print("❌ Invalid JSON payload")
        
        elif sensor_type == "temperature":
            try:
                self.sensor_data['temperature'] = float(payload)
                print(f"🌡️  Temperature: {self.sensor_data['temperature']}°C")
            except ValueError:
                print("❌ Invalid temperature value")
        
        elif sensor_type == "humidity":
            try:
                self.sensor_data['humidity'] = float(payload)
                print(f"💧 Humidity: {self.sensor_data['humidity']}%")
            except ValueError:
                print("❌ Invalid humidity value")
        
        elif sensor_type == "motion":
            self.sensor_data['motion'] = payload == "1"
            print(f"👁️  Motion: {'YES' if self.sensor_data['motion'] else 'NO'}")
        
        elif sensor_type == "luminescence":
            try:
                self.sensor_data['luminescence'] = int(payload)
                print(f"☀️  Luminescence: {self.sensor_data['luminescence']}")
            except ValueError:
                print("❌ Invalid luminescence value")
        
        elif sensor_type == "status":
            print(f"📊 Status: {payload}")
        
        else:
            print(f"📡 Unknown sensor type: {sensor_type}")
    
    def display_sensor_data(self):
        """Display current sensor data in a formatted way"""
        print("\n" + "="*50)
        print("📊 CURRENT SENSOR DATA")
        print("="*50)
        
        if self.sensor_data['temperature'] is not None:
            print(f"🌡️  Temperature: {self.sensor_data['temperature']}°C")
        
        if self.sensor_data['humidity'] is not None:
            print(f"💧 Humidity: {self.sensor_data['humidity']}%")
        
        if self.sensor_data['motion'] is not None:
            print(f"👁️  Motion: {'YES' if self.sensor_data['motion'] else 'NO'}")
        
        if self.sensor_data['luminescence'] is not None:
            print(f"☀️  Luminescence: {self.sensor_data['luminescence']}")
        
        if self.sensor_data['timestamp'] is not None:
            timestamp = datetime.fromtimestamp(self.sensor_data['timestamp'] / 1000)
            print(f"⏰ Timestamp: {timestamp.strftime('%Y-%m-%d %H:%M:%S')}")
        
        print(f"📡 MQTT Connected: {'YES' if self.sensor_data['mqtt_connected'] else 'NO'}")
        print("="*50)
    
    def connect(self):
        """Connect to MQTT broker"""
        try:
            print(f"🔌 Connecting to MQTT broker at {self.broker}:{self.port}...")
            self.client.connect(self.broker, self.port, 60)
            return True
        except Exception as e:
            print(f"❌ Connection failed: {e}")
            return False
    
    def start(self):
        """Start the MQTT client loop"""
        if self.connect():
            try:
                self.client.loop_forever()
            except KeyboardInterrupt:
                print("\n🛑 Stopping MQTT subscriber...")
                self.client.disconnect()
                print("✅ Disconnected")
        else:
            print("❌ Failed to connect to MQTT broker")
    
    def get_statistics(self):
        """Get connection statistics"""
        return {
            'message_count': self.message_count,
            'last_message_time': self.last_message_time,
            'current_data': self.sensor_data
        }

def main():
    # Get broker settings from command line or use defaults
    broker = sys.argv[1] if len(sys.argv) > 1 else "localhost"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 1883
    
    print("="*60)
    print("🚀 ESP8266 Sensor Network MQTT Subscriber")
    print("="*60)
    print(f"📡 Broker: {broker}:{port}")
    print("📋 Topics (with location prefix):")
    print("   • {location}/temperature")
    print("   • {location}/humidity") 
    print("   • {location}/motion")
    print("   • {location}/luminescence")
    print("   • {location}/status")
    print("   • {location}/all (JSON)")
    print("   • Example: living_room/temperature")
    print("="*60)
    
    # Create and start subscriber
    subscriber = SensorMQTTSubscriber(broker, port)
    
    try:
        subscriber.start()
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")

if __name__ == "__main__":
    main() 