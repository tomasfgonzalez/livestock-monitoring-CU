/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : oled.c
  * @brief          : Interface for OLED display
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 Tomas Gonzalez & Brian Morris
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  */

/* Includes ------------------------------------------------------------ */
#include "oled.h"

#include "driver/i2c_master.h"
#include "esp_lcd_panel_sh1106.h"	
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"

/* Private variables --------------------------------------------------------- */
static const char *OLED_TAG = "OLED";
static const char *heartbeat_icon = "<3";

lv_disp_t *disp;

/* Function implementations ------------------------------------------------- */
void oled_init() {
  ESP_LOGI(OLED_TAG, "Initialize I2C bus");
  i2c_master_bus_handle_t i2c_bus = NULL;
  i2c_master_bus_config_t bus_config = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .i2c_port = I2C_BUS_PORT,
    .sda_io_num = LCD_PIN_NUM_SDA,
    .scl_io_num = LCD_PIN_NUM_SCL,
    .flags.enable_internal_pullup = true,
  };
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

  ESP_LOGI(OLED_TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i2c_config_t io_config = {
    .dev_addr = LCD_I2C_HW_ADDR,
    .scl_speed_hz = LCD_PIXEL_CLOCK_HZ,
    .control_phase_bytes = 1,
    .dc_bit_offset = 6,
    .lcd_cmd_bits = LCD_CMD_BITS,
    .lcd_param_bits = LCD_PARAM_BITS,
    .flags = {
      .disable_control_phase = 0,
    }
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

  ESP_LOGI(OLED_TAG, "Install SSD1306 panel driver");
  esp_lcd_panel_handle_t panel_handle = NULL;

  esp_lcd_panel_dev_config_t panel_config = {
    .bits_per_pixel = 1,
    .reset_gpio_num = LCD_PIN_NUM_RST,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(io_handle, &panel_config, &panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  ESP_LOGI(OLED_TAG, "Initialize LVGL");
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  lvgl_port_init(&lvgl_cfg);

  const lvgl_port_display_cfg_t disp_cfg = {
    .io_handle = io_handle,
    .panel_handle = panel_handle,
    .buffer_size = LCD_H_RES * LCD_V_RES,
    .double_buffer = true,
    .hres = LCD_H_RES,
    .vres = LCD_V_RES,
    .monochrome = true,
    .rotation = {
      .swap_xy = false,
      .mirror_x = false,
      .mirror_y = true,
    }
  };
  disp = lvgl_port_add_disp(&disp_cfg);
}

void oled_welcome() {
  lv_obj_clean(lv_scr_act());

  lv_obj_t *label1 = lv_label_create(lv_scr_act());
  lv_label_set_text(label1, "Welcome to");
  lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 5, 0);

  lv_obj_t *label2 = lv_label_create(lv_scr_act());
  lv_label_set_text(label2, "Livestock");
  lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 5, 20);

  lv_obj_t *label3 = lv_label_create(lv_scr_act());
  lv_label_set_text(label3, "Monitoring!");
  lv_obj_align(label3, LV_ALIGN_TOP_LEFT, 5, 40);
}

void oled_status(char *wifi_status, char *mqtt_status, char *lsu_status, bool heartbeat_active) {
  // ESP_LOGI(OLED_TAG, "Status update");
  lv_obj_clean(lv_scr_act());

  // Create a label for WiFi status
  lv_obj_t *wifi_label = lv_label_create(lv_scr_act());
  lv_label_set_text(wifi_label, wifi_status);
  lv_obj_align(wifi_label, LV_ALIGN_TOP_LEFT, 10, 0);

  // Create a label for MQTT status
  lv_obj_t *mqtt_label = lv_label_create(lv_scr_act());
  lv_label_set_text(mqtt_label, mqtt_status);
  lv_obj_align(mqtt_label, LV_ALIGN_TOP_LEFT, 10, 20);

  // Create a label for LSU count
  lv_obj_t *lsu_label = lv_label_create(lv_scr_act());
  lv_label_set_text(lsu_label, lsu_status);
  lv_obj_align(lsu_label, LV_ALIGN_TOP_LEFT, 10, 40);

  // Draw heartbeat icon (small heart) at bottom left
  if (heartbeat_active) {
    // Create a simple filled circle as a heart indicator
    lv_obj_t *heart_indicator = lv_label_create(lv_scr_act());
    lv_label_set_text(heart_indicator, heartbeat_icon);
    lv_obj_align(heart_indicator, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
  }
}
