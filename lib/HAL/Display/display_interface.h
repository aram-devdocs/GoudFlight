#ifndef DISPLAY_INTERFACE_H
#define DISPLAY_INTERFACE_H

#include "../Core/hal_types.h"

typedef enum {
    DISPLAY_CAP_MONOCHROME = 0x01,
    DISPLAY_CAP_COLOR = 0x02,
    DISPLAY_CAP_BACKLIGHT = 0x04,
    DISPLAY_CAP_TOUCH = 0x08,
    DISPLAY_CAP_GRAYSCALE = 0x10
} display_capabilities_t;

typedef enum {
    DISPLAY_COLOR_BLACK = 0x00000000,
    DISPLAY_COLOR_WHITE = 0xFFFFFFFF,
    DISPLAY_COLOR_INVERT = 0x80000000
} display_color_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t color_depth;
    uint32_t capabilities;
} display_info_t;

typedef struct {
    int16_t x;
    int16_t y;
} display_point_t;

typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} display_rect_t;

typedef struct display_instance_s display_instance_t;

typedef struct {
    hal_status_t (*init)(display_instance_t* const instance);
    hal_status_t (*deinit)(display_instance_t* const instance);
    hal_status_t (*clear)(display_instance_t* const instance);
    hal_status_t (*refresh)(display_instance_t* const instance);
    hal_status_t (*set_pixel)(display_instance_t* const instance, 
                             const display_point_t* const point, 
                             const uint32_t color);
    hal_status_t (*draw_rect)(display_instance_t* const instance, 
                             const display_rect_t* const rect, 
                             const uint32_t color, 
                             const bool filled);
    hal_status_t (*draw_line)(display_instance_t* const instance,
                             const display_point_t* const start,
                             const display_point_t* const end,
                             const uint32_t color);
    hal_status_t (*draw_circle)(display_instance_t* const instance,
                               const display_point_t* const center,
                               const uint16_t radius,
                               const uint32_t color,
                               const bool filled);
    hal_status_t (*set_text_cursor)(display_instance_t* const instance, 
                                   const display_point_t* const point);
    hal_status_t (*write_text)(display_instance_t* const instance, 
                              const char* const text, 
                              const uint8_t font_size);
    hal_status_t (*get_info)(const display_instance_t* const instance, 
                            display_info_t* const info);
    hal_status_t (*set_brightness)(display_instance_t* const instance, 
                                  const uint8_t brightness);
    hal_status_t (*set_contrast)(display_instance_t* const instance,
                                const uint8_t contrast);
} display_interface_t;

struct display_instance_s {
    const display_interface_t* interface;
    void* driver_data;
    const hal_resource_constraints_t* constraints;
    bool initialized;
};

typedef display_instance_t* (*display_factory_func_t)(const void* const config);

#endif