#include "DataFormatter.h"
#include <string.h>

const char DataFormatter::HEX_CHARS[] = "0123456789ABCDEF";

void DataFormatter::formatHex(char* buffer, size_t buffer_size, uint32_t value, uint8_t digits) {
    if (buffer_size < 3) return;
    
    buffer[0] = '0';
    buffer[1] = 'x';
    
    if (digits == 0) {
        digits = (value > 0xFFFFFF) ? 8 : (value > 0xFFFF) ? 6 : (value > 0xFF) ? 4 : 2;
    }
    
    size_t pos = 2;
    for (int i = digits - 1; i >= 0 && pos < buffer_size - 1; i--) {
        buffer[pos++] = HEX_CHARS[(value >> (i * 4)) & 0xF];
    }
    buffer[pos] = '\0';
}

void DataFormatter::formatBinary(char* buffer, size_t buffer_size, uint32_t value, uint8_t bits) {
    if (buffer_size < bits + 3) return;
    
    buffer[0] = '0';
    buffer[1] = 'b';
    
    size_t pos = 2;
    for (int i = bits - 1; i >= 0 && pos < buffer_size - 1; i--) {
        buffer[pos++] = ((value >> i) & 1) ? '1' : '0';
    }
    buffer[pos] = '\0';
}

void DataFormatter::formatTime(char* buffer, size_t buffer_size, uint32_t millis) {
    uint32_t seconds = millis / 1000;
    uint32_t ms = millis % 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    if (hours > 0) {
        snprintf(buffer, buffer_size, "%02u:%02u:%02u.%03u", 
                 (unsigned)hours, (unsigned)(minutes % 60), (unsigned)(seconds % 60), (unsigned)ms);
    } else if (minutes > 0) {
        snprintf(buffer, buffer_size, "%02u:%02u.%03u", 
                 (unsigned)minutes, (unsigned)(seconds % 60), (unsigned)ms);
    } else {
        snprintf(buffer, buffer_size, "%u.%03u s", (unsigned)seconds, (unsigned)ms);
    }
}

void DataFormatter::formatUptime(char* buffer, size_t buffer_size, uint32_t seconds) {
    uint32_t days = seconds / 86400;
    uint32_t hours = (seconds % 86400) / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    if (days > 0) {
        snprintf(buffer, buffer_size, "%ud %02uh %02um %02us", 
                 (unsigned)days, (unsigned)hours, (unsigned)minutes, (unsigned)secs);
    } else if (hours > 0) {
        snprintf(buffer, buffer_size, "%02u:%02u:%02u", 
                 (unsigned)hours, (unsigned)minutes, (unsigned)secs);
    } else {
        snprintf(buffer, buffer_size, "%02u:%02u", (unsigned)minutes, (unsigned)secs);
    }
}

void DataFormatter::formatBytes(char* buffer, size_t buffer_size, uint32_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    uint8_t unit_index = 0;
    float size = bytes;
    
    while (size >= 1024 && unit_index < 3) {
        size /= 1024;
        unit_index++;
    }
    
    if (unit_index == 0) {
        snprintf(buffer, buffer_size, "%u %s", (unsigned)bytes, units[0]);
    } else {
        snprintf(buffer, buffer_size, "%.2f %s", size, units[unit_index]);
    }
}

void DataFormatter::formatPercentage(char* buffer, size_t buffer_size, uint8_t percent) {
    if (percent > 100) percent = 100;
    snprintf(buffer, buffer_size, "%u%%", percent);
}

void DataFormatter::formatTemperature(char* buffer, size_t buffer_size, float celsius, bool use_fahrenheit) {
    if (use_fahrenheit) {
        float fahrenheit = (celsius * 9.0 / 5.0) + 32.0;
        snprintf(buffer, buffer_size, "%.1f°F", fahrenheit);
    } else {
        snprintf(buffer, buffer_size, "%.1f°C", celsius);
    }
}

void DataFormatter::formatVoltage(char* buffer, size_t buffer_size, float volts) {
    if (volts < 1.0) {
        snprintf(buffer, buffer_size, "%.0f mV", volts * 1000);
    } else {
        snprintf(buffer, buffer_size, "%.2f V", volts);
    }
}

void DataFormatter::formatFrequency(char* buffer, size_t buffer_size, uint32_t hz) {
    if (hz >= 1000000) {
        snprintf(buffer, buffer_size, "%.2f MHz", hz / 1000000.0);
    } else if (hz >= 1000) {
        snprintf(buffer, buffer_size, "%.2f kHz", hz / 1000.0);
    } else {
        snprintf(buffer, buffer_size, "%u Hz", (unsigned)hz);
    }
}

const char* DataFormatter::boolToString(bool value) {
    return value ? "true" : "false";
}

const char* DataFormatter::errorToString(int error_code) {
    switch (error_code) {
        case 0: return "OK";
        case 1: return "ERROR";
        case 2: return "BUSY";
        case 3: return "TIMEOUT";
        case 4: return "INVALID_PARAM";
        case 5: return "NOT_SUPPORTED";
        case 6: return "HARDWARE_ERROR";
        case 7: return "NOT_INITIALIZED";
        default: return "UNKNOWN";
    }
}

void DataFormatter::padLeft(char* str, size_t total_width, char pad_char) {
    size_t len = strlen(str);
    if (len >= total_width) return;
    
    size_t pad_count = total_width - len;
    memmove(str + pad_count, str, len + 1);
    memset(str, pad_char, pad_count);
}

void DataFormatter::padRight(char* str, size_t total_width, char pad_char) {
    size_t len = strlen(str);
    if (len >= total_width) return;
    
    memset(str + len, pad_char, total_width - len);
    str[total_width] = '\0';
}

void DataFormatter::center(char* str, size_t total_width, char pad_char) {
    size_t len = strlen(str);
    if (len >= total_width) return;
    
    size_t pad_left = (total_width - len) / 2;
    size_t pad_right = total_width - len - pad_left;
    
    memmove(str + pad_left, str, len + 1);
    memset(str, pad_char, pad_left);
    memset(str + pad_left + len, pad_char, pad_right);
    str[total_width] = '\0';
}

void DataFormatter::truncate(char* str, size_t max_length, const char* suffix) {
    size_t len = strlen(str);
    if (len <= max_length) return;
    
    size_t suffix_len = strlen(suffix);
    if (max_length > suffix_len) {
        strcpy(str + max_length - suffix_len, suffix);
    } else {
        str[max_length] = '\0';
    }
}

template<typename T>
void DataFormatter::formatArray(char* buffer, size_t buffer_size, const T* array, size_t count, const char* separator) {
    buffer[0] = '[';
    size_t pos = 1;
    
    for (size_t i = 0; i < count && pos < buffer_size - 2; i++) {
        if (i > 0) {
            size_t sep_len = strlen(separator);
            if (pos + sep_len < buffer_size - 2) {
                strcpy(buffer + pos, separator);
                pos += sep_len;
            }
        }
        
        char temp[32];
        snprintf(temp, sizeof(temp), "%d", array[i]);
        size_t temp_len = strlen(temp);
        
        if (pos + temp_len < buffer_size - 2) {
            strcpy(buffer + pos, temp);
            pos += temp_len;
        }
    }
    
    buffer[pos++] = ']';
    buffer[pos] = '\0';
}

void DataFormatter::createProgressBar(char* buffer, size_t buffer_size, uint8_t percent, uint8_t width) {
    if (buffer_size < width + 3) return;
    if (percent > 100) percent = 100;
    
    buffer[0] = '[';
    uint8_t filled = (width * percent) / 100;
    
    for (uint8_t i = 0; i < width; i++) {
        buffer[i + 1] = (i < filled) ? '=' : '-';
    }
    
    buffer[width + 1] = ']';
    buffer[width + 2] = '\0';
}

void DataFormatter::createBitmap(char* buffer, size_t buffer_size, uint32_t value, uint8_t bits) {
    if (buffer_size < bits + 1) return;
    
    for (uint8_t i = 0; i < bits && i < buffer_size - 1; i++) {
        buffer[i] = (value & (1 << (bits - 1 - i))) ? '#' : '.';
    }
    buffer[bits] = '\0';
}

TableFormatter::TableFormatter(uint8_t max_columns)
    : column_count(0) {
}

void TableFormatter::addColumn(const Column& column) {
    if (column_count < MAX_COLUMNS) {
        columns[column_count++] = column;
    }
}

void TableFormatter::printHeader(Print& output) {
    output.print("|");
    for (uint8_t i = 0; i < column_count; i++) {
        printCell(output, columns[i].header, columns[i]);
        output.print("|");
    }
    output.println();
}

void TableFormatter::printSeparator(Print& output) {
    output.print("+");
    for (uint8_t i = 0; i < column_count; i++) {
        for (uint8_t j = 0; j < columns[i].width; j++) {
            output.print("-");
        }
        output.print("+");
    }
    output.println();
}

void TableFormatter::printRow(Print& output, const char** values, uint8_t count) {
    output.print("|");
    for (uint8_t i = 0; i < column_count && i < count; i++) {
        printCell(output, values[i], columns[i]);
        output.print("|");
    }
    output.println();
}

void TableFormatter::printCell(Print& output, const char* value, const Column& column) {
    char buffer[64];
    strncpy(buffer, value, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    DataFormatter::truncate(buffer, column.width, "..");
    
    switch (column.alignment) {
        case Column::LEFT:
            DataFormatter::padRight(buffer, column.width);
            break;
        case Column::RIGHT:
            DataFormatter::padLeft(buffer, column.width);
            break;
        case Column::CENTER:
            DataFormatter::center(buffer, column.width);
            break;
    }
    
    output.print(buffer);
}

void LogFormatter::formatLogEntry(char* buffer, size_t buffer_size, 
                                  const char* level, const char* tag, 
                                  const char* message, uint32_t timestamp) {
    if (timestamp > 0) {
        char time_str[16];
        DataFormatter::formatTime(time_str, sizeof(time_str), timestamp);
        snprintf(buffer, buffer_size, "%s [%s] [%s] %s", time_str, level, tag, message);
    } else {
        snprintf(buffer, buffer_size, "[%s] [%s] %s", level, tag, message);
    }
}

void LogFormatter::formatDebugDump(char* buffer, size_t buffer_size,
                                   const uint8_t* data, size_t length,
                                   uint16_t offset) {
    size_t pos = 0;
    
    for (size_t i = 0; i < length && pos < buffer_size - 1; i += 16) {
        pos += snprintf(buffer + pos, buffer_size - pos, "%04X: ", offset + i);
        
        for (size_t j = 0; j < 16 && i + j < length && pos < buffer_size - 1; j++) {
            pos += snprintf(buffer + pos, buffer_size - pos, "%02X ", data[i + j]);
        }
        
        for (size_t j = length - i; j < 16 && pos < buffer_size - 1; j++) {
            pos += snprintf(buffer + pos, buffer_size - pos, "   ");
        }
        
        pos += snprintf(buffer + pos, buffer_size - pos, " |");
        
        for (size_t j = 0; j < 16 && i + j < length && pos < buffer_size - 1; j++) {
            char c = data[i + j];
            if (c >= 32 && c < 127) {
                buffer[pos++] = c;
            } else {
                buffer[pos++] = '.';
            }
        }
        
        if (i + 16 < length && pos < buffer_size - 1) {
            pos += snprintf(buffer + pos, buffer_size - pos, "|\n");
        }
    }
    
    buffer[pos] = '\0';
}

void LogFormatter::formatStackTrace(char* buffer, size_t buffer_size,
                                    void** addresses, size_t count) {
    size_t pos = snprintf(buffer, buffer_size, "Stack trace:\n");
    
    for (size_t i = 0; i < count && pos < buffer_size - 1; i++) {
        pos += snprintf(buffer + pos, buffer_size - pos, "  #%zu: %p\n", i, addresses[i]);
    }
}