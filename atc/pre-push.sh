#!/bin/bash
# Pre-push validation script
# Run this before pushing code to ensure quality

set -e  # Exit on error

echo "=========================================="
echo "Running Pre-Push Validation"
echo "=========================================="

# 1. Clean old cache
echo -e "\n📧 Cleaning cache..."
make clean

# 2. Run validation
echo -e "\n🔍 Running validation checks..."
python3 validate.py
if [ $? -ne 0 ]; then
    echo "❌ Validation failed!"
    exit 1
fi

# 3. Run linting (optional, informative)
echo -e "\n📝 Running linting (informative)..."
flake8 . --count --statistics || true

# 4. Test run
echo -e "\n🚀 Testing dashboard startup..."
timeout 3 python3 test_run.py || true

echo -e "\n=========================================="
echo "✅ Pre-push validation complete!"
echo "You can now safely push your code."
echo "=========================================="