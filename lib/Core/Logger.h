#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

enum class LogLevel {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_CRITICAL = 4,
    LOG_LEVEL_NONE = 5
};

class Logger {
public:
    static void init(uint32_t baud_rate = 115200, uint32_t timeout_ms = 5000);
    static void setLevel(LogLevel level);
    static LogLevel getLevel();
    
    static void log(LogLevel level, const char* tag, const char* format, ...);
    static void logRaw(const char* message);
    
    static void debug(const char* tag, const char* format, ...);
    static void info(const char* tag, const char* format, ...);
    static void warning(const char* tag, const char* format, ...);
    static void error(const char* tag, const char* format, ...);
    static void critical(const char* tag, const char* format, ...);
    
    static const char* levelToString(LogLevel level);
    
private:
    static LogLevel current_level;
    static bool initialized;
    static uint32_t init_time;
    
    static void vlog(LogLevel level, const char* tag, const char* format, va_list args);
    static void printTimestamp();
};

#define LOG_DEBUG(tag, ...) Logger::debug(tag, __VA_ARGS__)
#define LOG_INFO(tag, ...) Logger::info(tag, __VA_ARGS__)
#define LOG_WARNING(tag, ...) Logger::warning(tag, __VA_ARGS__)
#define LOG_ERROR(tag, ...) Logger::error(tag, __VA_ARGS__)
#define LOG_CRITICAL(tag, ...) Logger::critical(tag, __VA_ARGS__)

#define LOG_DEBUG_RAW(msg) Logger::logRaw(msg)

#endif