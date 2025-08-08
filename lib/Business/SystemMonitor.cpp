#include "SystemMonitor.h"
#include "../Core/Logger.h"
#include "../SystemInfo/system_info.h"

SystemMonitor::SystemMonitor(const char* system_name, uint8_t led_pin)
    : system_name(system_name)
    , led_pin(led_pin)
    , heartbeat_enabled(true)
    , metrics_enabled(true)
    , health_check_enabled(true)
    , last_heartbeat_time(0)
    , last_metrics_time(0)
    , last_report_time(0)
    , led_state(false)
    , loop_time_accumulator(0)
    , loop_time_samples(0)
    , health_callback(nullptr) {
    
    memset(&metrics, 0, sizeof(metrics));
    metrics.health_status = true;
}

hal_status_t SystemMonitor::init() {
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);
    
    metrics.total_heap = SystemInfo::getTotalHeap();
    metrics.free_heap = SystemInfo::getFreeHeap();
    
    LOG_INFO(system_name, "System monitor initialized");
    LOG_INFO(system_name, "Total heap: %u bytes", (unsigned)metrics.total_heap);
    LOG_INFO(system_name, "Free heap: %u bytes", (unsigned)metrics.free_heap);
    
    return HAL_OK;
}

hal_status_t SystemMonitor::update(uint32_t delta_ms) {
    uint32_t current_time = millis();
    metrics.uptime_ms = current_time;
    metrics.loop_count++;
    
    if (heartbeat_enabled) {
        updateHeartbeat(current_time);
    }
    
    if (metrics_enabled) {
        updateMetrics(current_time);
    }
    
    if (health_check_enabled) {
        checkHealth();
    }
    
    if (current_time - last_report_time >= Constants::Timing::DEBUG_PRINT_INTERVAL_MS) {
        printReport();
        last_report_time = current_time;
    }
    
    return HAL_OK;
}

void SystemMonitor::updateHeartbeat(uint32_t current_time) {
    if (current_time - last_heartbeat_time >= Constants::Timing::HEARTBEAT_INTERVAL_MS) {
        led_state = !led_state;
        digitalWrite(led_pin, led_state);
        last_heartbeat_time = current_time;
    }
}

void SystemMonitor::updateMetrics(uint32_t current_time) {
    static uint32_t last_sample_time = 0;
    static const uint32_t SAMPLE_INTERVAL_MS = 100;
    
    if (current_time - last_sample_time >= SAMPLE_INTERVAL_MS) {
        metrics.free_heap = SystemInfo::getFreeHeap();
        
        if (metrics.total_heap > 0) {
            float used_heap = metrics.total_heap - metrics.free_heap;
            metrics.cpu_usage_percent = (used_heap / metrics.total_heap) * 100.0f;
        }
        
        if (loop_time_samples > 0) {
            metrics.loop_time_avg_us = loop_time_accumulator / loop_time_samples;
            loop_time_accumulator = 0;
            loop_time_samples = 0;
        }
        
        last_sample_time = current_time;
    }
}

void SystemMonitor::checkHealth() {
    bool health = true;
    
    if (metrics.free_heap < Constants::Memory::MIN_FREE_HEAP_CRITICAL) {
        LOG_CRITICAL(system_name, "Critical: Free heap below threshold: %u", (unsigned)metrics.free_heap);
        health = false;
    } else if (metrics.free_heap < Constants::Memory::MIN_FREE_HEAP_WARNING) {
        LOG_WARNING(system_name, "Warning: Low free heap: %u", (unsigned)metrics.free_heap);
    }
    
    if (metrics.loop_time_max_us > 100000) {
        LOG_WARNING(system_name, "Warning: Loop time exceeded 100ms: %u us", (unsigned)metrics.loop_time_max_us);
    }
    
    if (health_callback) {
        health = health && health_callback();
    }
    
    metrics.health_status = health;
}

void SystemMonitor::recordLoopTime(uint32_t time_us) {
    loop_time_accumulator += time_us;
    loop_time_samples++;
    
    if (time_us > metrics.loop_time_max_us) {
        metrics.loop_time_max_us = time_us;
    }
}

void SystemMonitor::printReport() const {
    uint32_t seconds = metrics.uptime_ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    LOG_INFO(system_name, "Uptime: %02u:%02u:%02u | Heap: %u/%u | Loops: %u | Health: %s",
             (unsigned)hours, (unsigned)(minutes % 60), (unsigned)(seconds % 60),
             (unsigned)metrics.free_heap, (unsigned)metrics.total_heap,
             (unsigned)metrics.loop_count,
             metrics.health_status ? "OK" : "DEGRADED");
}