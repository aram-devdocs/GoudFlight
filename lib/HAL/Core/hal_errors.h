#ifndef HAL_ERRORS_H
#define HAL_ERRORS_H

#include "hal_types.h"

typedef struct {
    hal_status_t code;
    const char* message;
    const char* file;
    uint32_t line;
    uint32_t timestamp;
} hal_error_info_t;

#define HAL_ERROR_RETURN(status) \
    do { \
        if ((status) != HAL_OK) { \
            hal_log_error((status), __FILE__, __LINE__); \
            return (status); \
        } \
    } while(0)

#define HAL_CHECK_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            return HAL_INVALID_PARAM; \
        } \
    } while(0)

#define HAL_CHECK_INITIALIZED(instance) \
    do { \
        if (!(instance)->initialized) { \
            return HAL_NOT_INITIALIZED; \
        } \
    } while(0)

void hal_log_error(hal_status_t status, const char* file, uint32_t line);
const char* hal_get_error_string(hal_status_t status);
hal_status_t hal_get_last_error(hal_error_info_t* error_info);
void hal_clear_errors(void);

#endif