#ifndef HANDHELD_BSP_H
#define HANDHELD_BSP_H

#include "../Core/hal_types.h"
#include "../Display/display_interface.h"
#include "../Input/input_interface.h"

typedef struct {
    display_instance_t* display;
    input_instance_t* input;
} handheld_hardware_t;

hal_status_t handheld_bsp_init(handheld_hardware_t* const hardware);
hal_status_t handheld_bsp_deinit(handheld_hardware_t* const hardware);
hal_status_t handheld_bsp_reset(handheld_hardware_t* const hardware);

const display_instance_t* handheld_get_display(const handheld_hardware_t* const hw);
const input_instance_t* handheld_get_input(const handheld_hardware_t* const hw);

hal_status_t handheld_get_last_error(char* const error_msg, const uint8_t max_len);
void handheld_log_system_info(void);

#endif