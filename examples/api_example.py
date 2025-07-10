#!/usr/bin/env python3
"""
ESP8266 Sensor Network API Example

This script demonstrates how to interact with the ESP8266 sensor network
API endpoints for data collection and device control.

Requirements:
    pip install requests

Usage:
    python api_example.py [device_ip]
"""

import requests
import json
import time
import sys
from datetime import datetime

class ESP8266SensorAPI:
    def __init__(self, device_ip="192.168.1.100"):
        self.base_url = f"http://{device_ip}"
        self.session = requests.Session()
        self.session.timeout = 10
    
    def get_sensor_data(self):
        """Get current sensor readings"""
        try:
            response = self.session.get(f"{self.base_url}/api/sensors")
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Error getting sensor data: {e}")
            return None
    
    def restart_device(self):
        """Restart the ESP8266 device"""
        try:
            response = self.session.post(f"{self.base_url}/api/restart")
            response.raise_for_status()
            print("Device restart initiated")
            return True
        except requests.exceptions.RequestException as e:
            print(f"Error restarting device: {e}")
            return False
    
    def reset_wifi(self):
        """Reset WiFi settings and create hotspot"""
        try:
            response = self.session.post(f"{self.base_url}/api/wifi/reset")
            response.raise_for_status()
            print("WiFi settings reset. Device will create hotspot 'ESP8266_Sensor_Network'")
            return True
        except requests.exceptions.RequestException as e:
            print(f"Error resetting WiFi: {e}")
            return False
    
    def check_device_status(self):
        """Check if device is online"""
        try:
            response = self.session.get(f"{self.base_url}/api/sensors", timeout=5)
            return response.status_code == 200
        except:
            return False

def print_sensor_data(data):
    """Pretty print sensor data"""
    if not data:
        print("No data received")
        return
    
    print("\n" + "="*50)
    print("ESP8266 SENSOR DATA")
    print("="*50)
    print(f"Timestamp: {datetime.fromtimestamp(data.get('timestamp', 0)/1000)}")
    print(f"Temperature: {data.get('temperature', 'N/A')}°C")
    print(f"Humidity: {data.get('humidity', 'N/A')}%")
    print(f"Motion: {'YES' if data.get('motion') else 'NO'}")
    print(f"Luminescence: {data.get('luminescence', 'N/A')}")
    print(f"WiFi RSSI: {data.get('wifi_rssi', 'N/A')} dBm")
    print(f"Free Memory: {data.get('free_heap', 'N/A')} bytes")
    print(f"Uptime: {data.get('uptime', 'N/A')} ms")
    print("="*50)

def continuous_monitoring(api, interval=5):
    """Continuously monitor sensor data"""
    print(f"Starting continuous monitoring (interval: {interval}s)")
    print("Press Ctrl+C to stop")
    
    try:
        while True:
            data = api.get_sensor_data()
            if data:
                print_sensor_data(data)
            else:
                print(f"[{datetime.now().strftime('%H:%M:%S')}] Device not responding")
            
            time.sleep(interval)
    
    except KeyboardInterrupt:
        print("\nMonitoring stopped")

def main():
    # Get device IP from command line or use default
    device_ip = sys.argv[1] if len(sys.argv) > 1 else "192.168.1.100"
    
    print(f"ESP8266 Sensor Network API Example")
    print(f"Device IP: {device_ip}")
    print("-" * 40)
    
    # Initialize API client
    api = ESP8266SensorAPI(device_ip)
    
    # Check if device is online
    if not api.check_device_status():
        print(f"❌ Device at {device_ip} is not responding")
        print("Please check:")
        print("1. Device is powered on and connected to WiFi")
        print("2. IP address is correct")
        print("3. Device is on the same network")
        return
    
    print(f"✅ Device at {device_ip} is online")
    
    # Show menu
    while True:
        print("\n" + "="*30)
        print("ESP8266 SENSOR API MENU")
        print("="*30)
        print("1. Get current sensor data")
        print("2. Start continuous monitoring")
        print("3. Restart device")
        print("4. Reset WiFi settings")
        print("5. Check device status")
        print("6. Exit")
        print("-"*30)
        
        choice = input("Enter your choice (1-6): ").strip()
        
        if choice == "1":
            data = api.get_sensor_data()
            print_sensor_data(data)
        
        elif choice == "2":
            interval = input("Enter monitoring interval in seconds (default 5): ").strip()
            try:
                interval = int(interval) if interval else 5
                continuous_monitoring(api, interval)
            except ValueError:
                print("Invalid interval, using default 5 seconds")
                continuous_monitoring(api)
        
        elif choice == "3":
            confirm = input("Are you sure you want to restart the device? (y/N): ").strip().lower()
            if confirm == 'y':
                api.restart_device()
            else:
                print("Restart cancelled")
        
        elif choice == "4":
            confirm = input("Are you sure you want to reset WiFi settings? (y/N): ").strip().lower()
            if confirm == 'y':
                api.reset_wifi()
            else:
                print("WiFi reset cancelled")
        
        elif choice == "5":
            if api.check_device_status():
                print("✅ Device is online")
            else:
                print("❌ Device is offline")
        
        elif choice == "6":
            print("Goodbye!")
            break
        
        else:
            print("Invalid choice. Please enter 1-6.")

if __name__ == "__main__":
    main() 