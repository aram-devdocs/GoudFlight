#include "Logger.h"

LogLevel Logger::current_level = LogLevel::LOG_LEVEL_INFO;
bool Logger::initialized = false;
uint32_t Logger::init_time = 0;

void Logger::init(uint32_t baud_rate, uint32_t timeout_ms) {
    if (initialized) {
        return;
    }
    
    Serial.begin(baud_rate);
    
    uint32_t start = millis();
    while (!Serial && (millis() - start) < timeout_ms) {
        ;
    }
    
    delay(100);
    
    init_time = millis();
    initialized = true;
    
    Serial.println(F("===================================="));
    Serial.println(F("       Logger Initialized"));
    Serial.println(F("===================================="));
}

void Logger::setLevel(LogLevel level) {
    current_level = level;
}

LogLevel Logger::getLevel() {
    return current_level;
}

void Logger::log(LogLevel level, const char* tag, const char* format, ...) {
    if (!initialized || level < current_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(level, tag, format, args);
    va_end(args);
}

void Logger::logRaw(const char* message) {
    if (!initialized) {
        return;
    }
    Serial.println(message);
}

void Logger::debug(const char* tag, const char* format, ...) {
    if (!initialized || LogLevel::LOG_LEVEL_DEBUG < current_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(LogLevel::LOG_LEVEL_DEBUG, tag, format, args);
    va_end(args);
}

void Logger::info(const char* tag, const char* format, ...) {
    if (!initialized || LogLevel::LOG_LEVEL_INFO < current_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(LogLevel::LOG_LEVEL_INFO, tag, format, args);
    va_end(args);
}

void Logger::warning(const char* tag, const char* format, ...) {
    if (!initialized || LogLevel::LOG_LEVEL_WARNING < current_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(LogLevel::LOG_LEVEL_WARNING, tag, format, args);
    va_end(args);
}

void Logger::error(const char* tag, const char* format, ...) {
    if (!initialized || LogLevel::LOG_LEVEL_ERROR < current_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(LogLevel::LOG_LEVEL_ERROR, tag, format, args);
    va_end(args);
}

void Logger::critical(const char* tag, const char* format, ...) {
    if (!initialized || LogLevel::LOG_LEVEL_CRITICAL < current_level) {
        return;
    }
    
    va_list args;
    va_start(args, format);
    vlog(LogLevel::LOG_LEVEL_CRITICAL, tag, format, args);
    va_end(args);
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::LOG_LEVEL_DEBUG:    return "DEBUG";
        case LogLevel::LOG_LEVEL_INFO:     return "INFO";
        case LogLevel::LOG_LEVEL_WARNING:  return "WARN";
        case LogLevel::LOG_LEVEL_ERROR:    return "ERROR";
        case LogLevel::LOG_LEVEL_CRITICAL: return "CRIT";
        default:                           return "UNKNOWN";
    }
}

void Logger::vlog(LogLevel level, const char* tag, const char* format, va_list args) {
    printTimestamp();
    
    Serial.print(F(" ["));
    Serial.print(levelToString(level));
    Serial.print(F("] "));
    
    if (tag != nullptr) {
        Serial.print(F("["));
        Serial.print(tag);
        Serial.print(F("] "));
    }
    
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
}

void Logger::printTimestamp() {
    uint32_t elapsed = millis() - init_time;
    uint32_t seconds = elapsed / 1000;
    uint32_t ms = elapsed % 1000;
    
    char timestamp[16];
    snprintf(timestamp, sizeof(timestamp), "%6u.%03u", (unsigned)seconds, (unsigned)ms);
    Serial.print(timestamp);
}