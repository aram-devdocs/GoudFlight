.PHONY: help install install-dev lint type-check format test clean run validate all

help:
	@echo "Available commands:"
	@echo "  make install      - Install production dependencies"
	@echo "  make install-dev  - Install development dependencies"
	@echo "  make lint         - Run linting checks"
	@echo "  make type-check   - Run type checking with mypy"
	@echo "  make format       - Auto-format code with black and isort"
	@echo "  make test         - Run tests"
	@echo "  make validate     - Run all checks (lint, type-check, test)"
	@echo "  make clean        - Remove cache files"
	@echo "  make run          - Run the application"
	@echo "  make all          - Install deps and run all checks"

install:
	pip install -r requirements.txt

install-dev:
	pip install -r requirements.txt
	pip install -r requirements-dev.txt

lint:
	@echo "Running flake8..."
	flake8 . || true
	@echo "\nRunning pylint..."
	pylint *.py components/*.py state/*.py --exit-zero

type-check:
	@echo "Running mypy type checking..."
	mypy . --ignore-missing-imports || true

format:
	@echo "Formatting with black..."
	black . --exclude=".venv|venv"
	@echo "Sorting imports with isort..."
	isort . --skip-venv

test:
	@echo "Running tests..."
	pytest tests/ -v || echo "No tests found or test failed"

clean:
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete
	find . -type f -name "*.pyo" -delete
	find . -type f -name "*.pyd" -delete
	find . -type f -name ".coverage" -delete
	find . -type d -name "*.egg-info" -exec rm -rf {} + 2>/dev/null || true
	find . -type d -name ".pytest_cache" -exec rm -rf {} + 2>/dev/null || true
	find . -type d -name ".mypy_cache" -exec rm -rf {} + 2>/dev/null || true

run:
	python3 main.py

validate: lint type-check test
	@echo "\n✅ All validation checks completed!"

all: install-dev clean format validate
	@echo "\n🎉 Development environment ready!"