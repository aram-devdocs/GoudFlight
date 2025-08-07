#ifndef HAL_TYPES_H
#define HAL_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_OK = 0,
    HAL_ERROR = 1,
    HAL_BUSY = 2,
    HAL_TIMEOUT = 3,
    HAL_INVALID_PARAM = 4,
    HAL_NOT_SUPPORTED = 5,
    HAL_HARDWARE_ERROR = 6,
    HAL_NOT_INITIALIZED = 7
} hal_status_t;

typedef enum {
    HAL_BOARD_HANDHELD = 0,
    HAL_BOARD_DRONE = 1,
    HAL_BOARD_BASE_STATION = 2
} hal_board_type_t;

typedef struct {
    uint32_t magic_number;
    uint32_t version;
    hal_board_type_t board_type;
} hal_config_header_t;

typedef struct {
    uint32_t max_stack_bytes;
    uint32_t max_heap_bytes;
    uint32_t max_cpu_percent;
} hal_resource_constraints_t;

#define HAL_CONFIG_MAGIC 0xDEADBEEFU
#define HAL_CONFIG_VERSION 0x00010000U

#endif