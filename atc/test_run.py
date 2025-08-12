#!/usr/bin/env python3
"""
Test run the dashboard for a few seconds to ensure it starts properly
"""

import sys
import time
import threading
from state.state_manager import StateManager
from dashboard import Dashboard
from state.actions import log_message


def test_dashboard():
    """Test dashboard starts and runs without errors"""
    try:
        # Create state manager
        state_manager = StateManager()
        
        # Add some test data
        state_manager.dispatch(log_message("INFO", "Test run started", "TestRunner"))
        
        # Create dashboard without keyboard
        dashboard = Dashboard(state_manager, keyboard_enabled=False)
        
        # Run for 2 seconds then stop
        def stop_after_delay():
            time.sleep(2)
            dashboard.stop()
            print("\n✅ Dashboard ran successfully for 2 seconds")
        
        stopper = threading.Thread(target=stop_after_delay, daemon=True)
        stopper.start()
        
        # Run dashboard
        dashboard.run()
        
        return True
        
    except Exception as e:
        print(f"\n❌ Dashboard failed to run: {e}")
        return False


if __name__ == "__main__":
    print("Testing dashboard startup...")
    success = test_dashboard()
    sys.exit(0 if success else 1)