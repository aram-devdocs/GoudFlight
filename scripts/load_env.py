#!/usr/bin/env python3
"""
Load environment variables from .env file and inject them as build flags.
This script is called by PlatformIO before building.
"""

import os
import sys
from pathlib import Path

Import("env")

def load_env_file():
    """Load .env file and return as dictionary."""
    env_file = Path(".env")
    env_template = Path(".env.template")
    
    # Check if .env exists
    if not env_file.exists():
        print("\n" + "="*60)
        print("ERROR: .env file not found!")
        print("="*60)
        print("\nPlease create a .env file with your MAC addresses.")
        print("You can copy .env.template as a starting point:")
        print("\n  cp .env.template .env")
        print("\nThen edit .env and add your device MAC addresses:")
        print("  HANDHELD_MAC_ADDRESS=XX:XX:XX:XX:XX:XX")
        print("  BASE_STATION_MAC_ADDRESS=XX:XX:XX:XX:XX:XX")
        print("\nTo find your ESP32 MAC addresses, upload a test sketch")
        print("that prints WiFi.macAddress()")
        print("="*60 + "\n")
        sys.exit(1)
    
    env_vars = {}
    with open(env_file, 'r') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                # Split on first = and remove comments
                if '=' in line:
                    key, value = line.split('=', 1)
                    # Remove inline comments
                    if '#' in value:
                        value = value.split('#')[0]
                    env_vars[key.strip()] = value.strip()
    
    return env_vars

def validate_mac_address(mac_str):
    """Validate MAC address format."""
    if not mac_str:
        return False
    
    parts = mac_str.split(':')
    if len(parts) != 6:
        return False
    
    try:
        for part in parts:
            if len(part) != 2:
                return False
            int(part, 16)  # Check if valid hex
        return True
    except ValueError:
        return False

def mac_to_c_array(mac_str):
    """Convert MAC address string to C array format."""
    parts = mac_str.split(':')
    return '{' + ', '.join(f'0x{part}' for part in parts) + '}'

# Load environment variables
env_vars = load_env_file()

# Check required MAC addresses
required_macs = ['HANDHELD_MAC_ADDRESS', 'BASE_STATION_MAC_ADDRESS']
missing = []
invalid = []

for mac_key in required_macs:
    if mac_key not in env_vars or not env_vars[mac_key]:
        missing.append(mac_key)
    elif not validate_mac_address(env_vars[mac_key]):
        invalid.append(f"{mac_key}={env_vars[mac_key]}")

if missing or invalid:
    print("\n" + "="*60)
    print("ERROR: Invalid .env configuration!")
    print("="*60)
    if missing:
        print("\nMissing MAC addresses:")
        for key in missing:
            print(f"  - {key}")
    if invalid:
        print("\nInvalid MAC address format:")
        for item in invalid:
            print(f"  - {item}")
    print("\nMAC addresses must be in format: XX:XX:XX:XX:XX:XX")
    print("Example: 30:ED:A0:A8:B5:70")
    print("="*60 + "\n")
    sys.exit(1)

# Convert MAC addresses to C array format and add as build flags
handheld_mac = env_vars['HANDHELD_MAC_ADDRESS']
base_mac = env_vars['BASE_STATION_MAC_ADDRESS']

# Add build flags (need to escape properly for C preprocessor)
handheld_array = mac_to_c_array(handheld_mac)
base_array = mac_to_c_array(base_mac)

env.Append(
    BUILD_FLAGS=[
        f'-D HANDHELD_MAC_STRING=\\"{handheld_mac}\\"',
        f'-D BASE_STATION_MAC_STRING=\\"{base_mac}\\"',
        f'-D "HANDHELD_MAC_ARRAY={handheld_array}"',
        f'-D "BASE_STATION_MAC_ARRAY={base_array}"'
    ]
)

print(f"\n✓ Loaded MAC addresses from .env:")
print(f"  Handheld:     {handheld_mac}")
print(f"  Base Station: {base_mac}\n")