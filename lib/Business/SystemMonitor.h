#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <Arduino.h>
#include "../HAL/Core/hal_types.h"
#include "../Core/Constants.h"

class SystemMonitor {
public:
    struct SystemMetrics {
        uint32_t uptime_ms;
        uint32_t free_heap;
        uint32_t total_heap;
        float cpu_usage_percent;
        uint32_t loop_count;
        uint32_t loop_time_avg_us;
        uint32_t loop_time_max_us;
        bool health_status;
    };
    
    SystemMonitor(const char* system_name, uint8_t led_pin = Constants::Hardware::DEFAULT_LED_PIN);
    ~SystemMonitor() = default;
    
    hal_status_t init();
    hal_status_t update(uint32_t delta_ms);
    
    void enableHeartbeat(bool enable) { heartbeat_enabled = enable; }
    void enableMetrics(bool enable) { metrics_enabled = enable; }
    void enableHealthCheck(bool enable) { health_check_enabled = enable; }
    
    const SystemMetrics& getMetrics() const { return metrics; }
    bool isHealthy() const { return metrics.health_status; }
    uint32_t getUptime() const { return metrics.uptime_ms; }
    
    void recordLoopTime(uint32_t time_us);
    void setHealthCallback(std::function<bool()> callback) { health_callback = callback; }
    
    void printReport() const;
    
private:
    const char* system_name;
    uint8_t led_pin;
    bool heartbeat_enabled;
    bool metrics_enabled;
    bool health_check_enabled;
    
    SystemMetrics metrics;
    uint32_t last_heartbeat_time;
    uint32_t last_metrics_time;
    uint32_t last_report_time;
    bool led_state;
    
    uint32_t loop_time_accumulator;
    uint32_t loop_time_samples;
    
    std::function<bool()> health_callback;
    
    void updateHeartbeat(uint32_t current_time);
    void updateMetrics(uint32_t current_time);
    void checkHealth();
};

#endif