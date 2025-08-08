#ifndef DATA_FORMATTER_H
#define DATA_FORMATTER_H

#include <Arduino.h>
#include <stdint.h>

class DataFormatter {
public:
    static void formatHex(char* buffer, size_t buffer_size, uint32_t value, uint8_t digits = 0);
    static void formatBinary(char* buffer, size_t buffer_size, uint32_t value, uint8_t bits = 8);
    static void formatTime(char* buffer, size_t buffer_size, uint32_t millis);
    static void formatUptime(char* buffer, size_t buffer_size, uint32_t seconds);
    static void formatBytes(char* buffer, size_t buffer_size, uint32_t bytes);
    static void formatPercentage(char* buffer, size_t buffer_size, uint8_t percent);
    static void formatTemperature(char* buffer, size_t buffer_size, float celsius, bool use_fahrenheit = false);
    static void formatVoltage(char* buffer, size_t buffer_size, float volts);
    static void formatFrequency(char* buffer, size_t buffer_size, uint32_t hz);
    
    static const char* boolToString(bool value);
    static const char* errorToString(int error_code);
    
    static void padLeft(char* str, size_t total_width, char pad_char = ' ');
    static void padRight(char* str, size_t total_width, char pad_char = ' ');
    static void center(char* str, size_t total_width, char pad_char = ' ');
    
    static void truncate(char* str, size_t max_length, const char* suffix = "...");
    
    template<typename T>
    static void formatArray(char* buffer, size_t buffer_size, const T* array, size_t count, const char* separator = ", ");
    
    static void createProgressBar(char* buffer, size_t buffer_size, uint8_t percent, uint8_t width = 10);
    static void createBitmap(char* buffer, size_t buffer_size, uint32_t value, uint8_t bits = 8);
    
private:
    static const char HEX_CHARS[];
};

class TableFormatter {
public:
    struct Column {
        const char* header;
        uint8_t width;
        enum Alignment { LEFT, CENTER, RIGHT } alignment;
    };
    
    TableFormatter(uint8_t max_columns = 8);
    ~TableFormatter() = default;
    
    void addColumn(const Column& column);
    void printHeader(Print& output);
    void printSeparator(Print& output);
    void printRow(Print& output, const char** values, uint8_t count);
    
private:
    static const uint8_t MAX_COLUMNS = 8;
    Column columns[MAX_COLUMNS];
    uint8_t column_count;
    
    void printCell(Print& output, const char* value, const Column& column);
};

class LogFormatter {
public:
    static void formatLogEntry(char* buffer, size_t buffer_size, 
                               const char* level, const char* tag, 
                               const char* message, uint32_t timestamp = 0);
    
    static void formatDebugDump(char* buffer, size_t buffer_size,
                                const uint8_t* data, size_t length,
                                uint16_t offset = 0);
    
    static void formatStackTrace(char* buffer, size_t buffer_size,
                                 void** addresses, size_t count);
};

#endif