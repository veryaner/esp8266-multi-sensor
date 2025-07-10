#!/usr/bin/env python3
"""
ESP8266 Sensor Network Build Script

This script automates the build and upload process for the ESP8266 sensor project.
It handles building, uploading, and monitoring the device.

Usage:
    python build.py [command]

Commands:
    build     - Build the project
    upload    - Build and upload to device
    monitor   - Monitor serial output
    clean     - Clean build files
    all       - Build, upload, and monitor
"""

import subprocess
import sys
import time
import os

def run_command(command, description):
    """Run a PlatformIO command and handle errors"""
    print(f"\n{'='*50}")
    print(f"🔄 {description}")
    print(f"{'='*50}")
    
    try:
        result = subprocess.run(command, shell=True, check=True, capture_output=True, text=True)
        print("✅ Success!")
        if result.stdout:
            print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Error: {e}")
        if e.stdout:
            print("STDOUT:", e.stdout)
        if e.stderr:
            print("STDERR:", e.stderr)
        return False

def build_project():
    """Build the project"""
    return run_command("pio run --no-intellisense", "Building project...")

def upload_project():
    """Build and upload the project"""
    return run_command("pio run --target upload --no-intellisense", "Building and uploading...")

def monitor_device():
    """Monitor serial output"""
    print(f"\n{'='*50}")
    print("📡 Monitoring device (Press Ctrl+C to stop)")
    print(f"{'='*50}")
    
    try:
        subprocess.run("pio device monitor", shell=True)
    except KeyboardInterrupt:
        print("\nMonitoring stopped")

def clean_project():
    """Clean build files"""
    return run_command("pio run --target clean --no-intellisense", "Cleaning build files...")

def upload_spiffs():
    """Upload SPIFFS filesystem"""
    return run_command("pio run --target uploadfs --no-intellisense", "Uploading SPIFFS filesystem...")

def check_requirements():
    """Check if PlatformIO is installed"""
    try:
        subprocess.run("pio --version", shell=True, check=True, capture_output=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ PlatformIO not found!")
        print("Please install PlatformIO:")
        print("1. Install VS Code PlatformIO extension, or")
        print("2. Install via pip: pip install platformio")
        return False

def show_help():
    """Show help information"""
    print("""
ESP8266 Sensor Network Build Script

Usage:
    python build.py [command]

Commands:
    build     - Build the project
    upload    - Build and upload to device
    monitor   - Monitor serial output
    clean     - Clean build files
    spiffs    - Upload SPIFFS filesystem
    all       - Build, upload, and monitor
    help      - Show this help

Examples:
    python build.py build
    python build.py upload
    python build.py all
    """)

def main():
    if not check_requirements():
        sys.exit(1)
    
    if len(sys.argv) < 2:
        show_help()
        return
    
    command = sys.argv[1].lower()
    
    if command == "build":
        build_project()
    
    elif command == "upload":
        if build_project():
            upload_project()
    
    elif command == "monitor":
        monitor_device()
    
    elif command == "clean":
        clean_project()
    
    elif command == "spiffs":
        upload_spiffs()
    
    elif command == "all":
        if build_project():
            if upload_project():
                print("\n⏳ Waiting 3 seconds for device to restart...")
                time.sleep(3)
                monitor_device()
    
    elif command == "help":
        show_help()
    
    else:
        print(f"❌ Unknown command: {command}")
        show_help()

if __name__ == "__main__":
    main() 