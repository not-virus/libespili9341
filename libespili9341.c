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

static const char* TAG = "libespili9341"

esp_err_t init_display() {

    // Ensure SPI initializes correctly
    if (init_spi() != ESP_OK) {
        return ESP_FAIL;
    }

    

    return ESP_OK;
}