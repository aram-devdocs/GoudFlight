#ifndef LCD1602_I2C_DRIVER_H
#define LCD1602_I2C_DRIVER_H

#include "../display_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

display_instance_t* lcd1602_i2c_create_instance(uint8_t i2c_address,
                                                uint8_t sda_pin,
                                                uint8_t scl_pin,
                                                uint8_t columns,
                                                uint8_t rows);

#ifdef __cplusplus
}
#endif

#endif