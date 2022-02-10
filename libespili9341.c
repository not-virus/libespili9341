/**
 * libesp9341.c
 * 
 * Written by Cameron Krueger
 * 8 August 2021
 * 
 * Implementation file for libespili9341, an ILI9341 display driver library
 * for the esp8266 microcontroller written purely in C
 */

#include "libespili9341.h"

#define ILI9341_PIN_DC CONFIG_ILI9341_PIN_DC

static const char* TAG = "libespili9341";

/**
 * Initializes the display with default settings
 * 
 * @return ESP_OK if initialization successful, else ESP_FAIL
 */
esp_err_t init_display() {

    // Ensure SPI initializes correctly
    if (init_spi() != ESP_OK) {
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "configure D/C GPIO");
    gpio_config_t gpio_cfg;
    gpio_cfg.pin_bit_mask = (1<<ILI9341_PIN_DC); // Pin 4
    gpio_cfg.mode = GPIO_MODE_OUTPUT;
    gpio_cfg.pull_down_en = GPIO_FLOATING;
    gpio_cfg.pull_up_en = GPIO_FLOATING;
    gpio_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&gpio_cfg);

    ESP_LOGD(TAG, "init display");

    ESP_LOGD(TAG, "getting display info");

    disp_id_info_t disp_id;

    ili_get_info(&disp_id);

    return ESP_OK;
}

/**
 * Gets basic ID info from the ILI9341 display
 * 
 * @param disp_info an object of disp_id_info_t that will be populated with info
 * from the display
 * @return ESP_OK
 */
esp_err_t ili_get_info(disp_id_info_t* disp_info) {
    // FIXME extract byte count to global constant

    spi_send_command(ILI9341_RDDID);
    unsigned char* disp_info_vect = (unsigned char*) malloc(4 * sizeof(char));
    spi_read(disp_info_vect, 4);
    disp_info->mfgr_id = (uint8_t) disp_info_vect[1];
    disp_info->driver_ver_id = (uint8_t) disp_info_vect[2];
    disp_info->driver_id = (uint8_t) disp_info_vect[3];
    return ESP_OK;
}