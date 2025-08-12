#!/usr/bin/env python3
"""
Validation script to catch runtime errors before deployment
"""

import sys
import importlib
import traceback
from typing import List, Tuple


def validate_imports() -> Tuple[bool, List[str]]:
    """Validate all imports work correctly"""
    errors = []
    modules_to_check = [
        'main',
        'dashboard',
        'components.base_component',
        'components.telemetry_panel',
        'components.log_panel',
        'components.status_panel',
        'components.command_panel',
        'components.metrics_panel',
        'state.state_manager',
        'state.actions',
        'state.types'
    ]
    
    for module in modules_to_check:
        try:
            importlib.import_module(module)
            print(f"✓ {module}")
        except Exception as e:
            errors.append(f"✗ {module}: {str(e)}")
            print(f"✗ {module}: {str(e)}")
    
    return len(errors) == 0, errors


def validate_dashboard_creation() -> Tuple[bool, List[str]]:
    """Validate dashboard can be created without errors"""
    errors = []
    
    try:
        from state.state_manager import StateManager
        from dashboard import Dashboard
        
        # Create state manager
        state_manager = StateManager()
        print("✓ StateManager created")
        
        # Create dashboard
        dashboard = Dashboard(state_manager, keyboard_enabled=False)
        print("✓ Dashboard created")
        
        # Test component creation
        dashboard._init_components()
        print("✓ Components initialized")
        
        # Test layout creation
        layout = dashboard._create_layout()
        print("✓ Layout created")
        
        # Test rendering (without actually displaying)
        try:
            dashboard._update_layout()
            print("✓ Layout update successful")
        except Exception as e:
            errors.append(f"Layout update failed: {str(e)}")
            
    except Exception as e:
        errors.append(f"Dashboard validation failed: {str(e)}")
        traceback.print_exc()
    
    return len(errors) == 0, errors


def validate_serial_mock() -> Tuple[bool, List[str]]:
    """Validate serial communication can be mocked"""
    errors = []
    
    try:
        from main import BaseStationMonitor
        
        # Create monitor without actually connecting
        monitor = BaseStationMonitor(port="/dev/null", baudrate=115200)
        print("✓ BaseStationMonitor created")
        
        # Test state manager exists
        if hasattr(monitor, 'state_manager'):
            print("✓ State manager attached")
        else:
            errors.append("State manager not found")
            
    except Exception as e:
        errors.append(f"Serial mock validation failed: {str(e)}")
    
    return len(errors) == 0, errors


def main():
    """Run all validations"""
    print("=" * 60)
    print("ATC Dashboard Validation")
    print("=" * 60)
    
    all_passed = True
    all_errors = []
    
    # Validate imports
    print("\n1. Validating imports...")
    passed, errors = validate_imports()
    all_passed = all_passed and passed
    all_errors.extend(errors)
    
    # Validate dashboard
    print("\n2. Validating dashboard creation...")
    passed, errors = validate_dashboard_creation()
    all_passed = all_passed and passed
    all_errors.extend(errors)
    
    # Validate serial
    print("\n3. Validating serial interface...")
    passed, errors = validate_serial_mock()
    all_passed = all_passed and passed
    all_errors.extend(errors)
    
    # Summary
    print("\n" + "=" * 60)
    if all_passed:
        print("✅ All validations passed!")
        print("The application should run without import or initialization errors.")
    else:
        print("❌ Validation failed with errors:")
        for error in all_errors:
            print(f"  - {error}")
        sys.exit(1)
    
    print("=" * 60)


if __name__ == "__main__":
    main()