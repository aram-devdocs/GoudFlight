#ifndef BASE_STATION_BSP_H
#define BASE_STATION_BSP_H

#include "../Core/hal_errors.h"
#include "../../Config/hardware_profiles.h"
#include "../Display/display_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

hal_status_t base_station_bsp_init(const hal_config_header_t* config);
hal_status_t base_station_bsp_deinit(void);
display_instance_t* base_station_bsp_get_display(void);

#ifdef __cplusplus
}
#endif

#endif