#!/bin/bash
# Install pre-commit hooks for the ATC dashboard

echo "Installing pre-commit hooks for ATC dashboard..."

# Check if pre-commit is installed
if ! command -v pre-commit &> /dev/null; then
    echo "Installing pre-commit..."
    pip install pre-commit
fi

# Install the pre-commit hooks
pre-commit install

echo "✅ Pre-commit hooks installed successfully!"
echo "The following checks will run before each commit:"
echo "  - Dashboard validation"
echo "  - Dashboard startup test"
echo "  - Cache file check"
echo "  - Code formatting checks"
echo ""
echo "To run hooks manually: pre-commit run --all-files"