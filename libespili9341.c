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

#define WIRE_DEBUG // DEBUGGING, please remove

#define ILI9341_PIN_MISO CONFIG_ILI9341_PIN_MISO
#define ILI9341_PIN_MOSI CONFIG_ILI9341_PIN_MOSI
#define ILI9341_PIN_CLK CONFIG_ILI9341_PIN_CLK
#define ILI9341_PIN_DC CONFIG_ILI9341_PIN_DC

static const char* TAG = "libespili9341";

typedef enum {
    SPI_SEND = 0,
    SPI_RECV
} spi_master_mode_t;

esp_err_t conv_arr_uint8_uint32_right(uint8_t* s, uint32_t len, uint32_t* d);
esp_err_t conv_arr_uint8_uint32_left(uint8_t* s, uint32_t len, uint32_t* d);
void reverse_uint8_array(uint8_t* arr, uint32_t len);
void reverse_uint8_array(uint8_t* arr, uint32_t len);
void reverse_uint16_array(uint16_t* arr, uint32_t len);

/**
 * Initializes the ESP8266 hardware SPI interface with the following settings:
 *  - //FIXME list settings
 */
esp_err_t init_spi() {
    ESP_LOGI(TAG, "init spi");
    spi_config_t spi_config;
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    spi_config.interface.byte_tx_order = SPI_BYTE_ORDER_MSB_FIRST;
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    spi_config.mode = SPI_MASTER_MODE;
    spi_config.clk_div = SPI_2MHz_DIV;     // Math is okay, but may be too fast
    spi_config.event_cb = NULL;
    esp_err_t tmp = spi_init(HSPI_HOST, &spi_config);

    if (tmp == ESP_OK) {
        ESP_LOGI(TAG, "spi init successful");
    } else {
        ESP_LOGE(TAG, "spi init failed");
    }

    ESP_LOGI(TAG, "configure D/C GPIO");
    gpio_config_t gpio_cfg;
    gpio_cfg.pin_bit_mask = (1<<ILI9341_PIN_DC); // Pin 4
    gpio_cfg.mode = GPIO_MODE_OUTPUT;
    gpio_cfg.pull_down_en = GPIO_FLOATING;
    gpio_cfg.pull_up_en = GPIO_FLOATING;
    gpio_cfg.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&gpio_cfg);

    #ifdef WIRE_DEBUG   // DEBUGGING, please remove
    ESP_LOGI(TAG, "configure trig GPIO");
    gpio_config_t trig_gpio_cfg;
    trig_gpio_cfg.intr_type = GPIO_INTR_DISABLE;
    trig_gpio_cfg.mode = GPIO_MODE_OUTPUT;
    trig_gpio_cfg.pin_bit_mask = (1 << 2);
    trig_gpio_cfg.pull_down_en = 0;
    trig_gpio_cfg.pull_up_en = 0;
    gpio_config(&trig_gpio_cfg);
    gpio_set_level(2, 1);
    #endif

    return tmp;
}

/**
 * Sends a 1-byte command followed by len bytes of data over the SPI interface
 * 
 * NOTICE: init_spi() must be called once to initialize the SPI interface before
 * this function can be used!
 * 
 * @param cmd a 1-byte command for the SPI device
 * @param data an array of bytes of data for the SPI device
 * @param len the length of the data array
 * @return ESP_OK if transmission successful, else ESP_FAIL
 */
esp_err_t spi_send_command_with_data(uint8_t cmd, uint8_t* data, uint32_t len) {

    ESP_LOGI(TAG, "allocate memory for spi_trans_t and clear");
    // Allocate memory for the spi transmission parameter structure
    spi_trans_t trans;
    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0; // Clear all bits

    ESP_LOGI(TAG, "set spi command");
    // Set command to write to peripheral
    uint16_t cmd16 = cmd;
    // 8 bits for command
    trans.bits.cmd = 8;
    trans.cmd = &cmd16;

    ESP_LOGI(TAG, "set address value");
    // When writing a command, the address isn't used
    trans.bits.addr = 0;
    trans.addr = NULL;

    //ESP_LOGI(TAG, "reverse data array");
    //reverse_uint8_array(data, len);

    ESP_LOGI(TAG, "allocate memory for uint32_t array");
    // Convert data to uint32_t array
    uint32_t* new_data = (uint32_t*) malloc(len * sizeof(*new_data));

    ESP_LOGI(TAG, "copy uint8_t data into new uint32_t array");
    conv_arr_uint8_uint32_left(data, len, new_data);

    ESP_LOGI(TAG, "set length of data and data in spi_trans_t");
    // Convert len to reflect new length
    len = (uint32_t) 8 * len;
    ESP_LOGI(TAG, "length is: %u", len);
    trans.bits.mosi = len;
    trans.mosi = new_data;

    ESP_LOGI(TAG, "set D/C to LOW");
    gpio_set_level(ILI9341_PIN_DC, 0);  // Set low for command

    ESP_LOGI(TAG, "transmit spi data to device");
    #ifdef WIRE_DEBUG   // DEBUGGING, please remove
    gpio_set_level(2, 0);
    #endif
    esp_err_t res = spi_trans(HSPI_HOST, &trans);
    #ifdef WIRE_DEBUG   // DEBUGGING, please remove
    gpio_set_level(2, 1);
    #endif

    ESP_LOGI(TAG, "free temporary uint32_t pointer");
    free(new_data);

    if (res == ESP_OK) {
        ESP_LOGI(TAG, "transmission successful");
    } else {
        ESP_LOGE(TAG, "spi transmission failed");
    }

    ESP_LOGI(TAG, "return");
    return res;
}

/**
 * Sends a 1-byte command over the SPI interface.
 * 
 * NOTICE: init_spi() must be called once to initialize the SPI interface before
 * this function can be used!
 * 
 * @param cmd a 1-byte command for the SPI device
 * @return ESP_OK if transmission successful, else ESP_FAIL
 */
esp_err_t spi_send_command(uint8_t cmd) {

    ESP_LOGI(TAG, "allocate memory for spi_trans_t and clear");
    // Allocate memory for the spi transmission parameter structure
    spi_trans_t trans;
    memset(&trans, 0x0, sizeof(trans));
    trans.bits.val = 0; // Clear all bits

    ESP_LOGI(TAG, "set spi command");
    // Set command to write to peripheral
    uint16_t cmd16 = cmd;
    // 8 bits for command
    trans.bits.cmd = 8;
    trans.cmd = &cmd16;

    ESP_LOGI(TAG, "set address value (NULL)");
    // When writing a command, the address isn't used
    trans.bits.addr = 0;
    trans.addr = NULL;

    ESP_LOGI(TAG, "set data value (NULL)");
    // When writing a command without data, the data isn't used
    trans.bits.mosi = 0;
    trans.mosi = NULL;

    ESP_LOGI(TAG, "set D/C to LOW");
    gpio_set_level(ILI9341_PIN_DC, 0);  // Set low for command

    ESP_LOGI(TAG, "transmit spi data to device");
    #ifdef WIRE_DEBUG   // DEBUGGING, please remove
    gpio_set_level(2, 0);
    #endif
    esp_err_t res = spi_trans(HSPI_HOST, &trans);
    #ifdef WIRE_DEBUG   // DEBUGGING, please remove
    gpio_set_level(2, 1);
    #endif

    if (res == ESP_OK) {
        ESP_LOGI(TAG, "transmission successful");
    } else {
        ESP_LOGE(TAG, "spi transmission failed");
    }

    ESP_LOGI(TAG, "return");
    return res;
}

/**
 * Reverses a uint8_t array
 * 
 * @param arr a uint8_t array
 * @param len the number of uint8_t elements (bytes) in arr
 */
void reverse_uint8_array(uint8_t* arr, uint32_t len) {
    uint32_t start = 0;
    uint32_t end = len - 1;
    while (start < end) {
        uint8_t tmp = *(arr + start);
        *(arr + start) = *(arr + end);
        *(arr + end) = tmp;
        start++;
        end--;
    }
}

/**
 * Reverses a uint16_t array
 * 
 * @param arr a uint16_t array
 * @param len the number of uint16_t elements in arr
 */
void reverse_uint16_array(uint16_t* arr, uint32_t len) {
    size_t size = sizeof(*arr);
    uint32_t start = 0;
    uint32_t end = len - 1;
    while (start < end) {
        uint16_t tmp = *(arr + size * start);
        *(arr + size * start) = *(arr + size * end);
        *(arr + size * end) = tmp;
        start++;
        end--;
    }
}

/**
 * Converts a uint8_t array to a uint32_t array and aligns the source data to
 * the right of the destination array
 *   i.e. {0x01, 0x02, 0x03} -> {0x00010203}
 *
 * NOTICE: This function uses pointers and pointer/offset arithmetic. It is
 * absolutely not safe. However many elements (in this case bytes) of the source
 * array are specified by len to be copied is how many this function will
 * attempt to copy. If len is larger than the actual size of s, undefined
 * behavior and/or a segfault is/are absolutely possible. USE WITH CARE.
 * 
 * @param s source array
 * @param len len of source array
 * @param d destination array
 * @return ESP_OK
 */
esp_err_t conv_arr_uint8_uint32_right(uint8_t* s, uint32_t len, uint32_t* d) {

    // FIXME Possibly hardcode for speed?
    uint8_t sizeof_s = sizeof(*s);
    uint8_t sizeof_d = sizeof(*d);

    // Get ratio of destination data width to source data width
    uint32_t type_ratio = sizeof_d / sizeof_s;

    // Size of destination array in bytes
    size_t dest_size = (size_t) sizeof_d * ceil(len / (double) type_ratio);

    // Clear all memory in array range
    memset(d, 0x0, dest_size);

    // Offset determines number of empty leading bytes for right alignment
    size_t offset = dest_size - len;
    
    // Copy source array into destination array
    size_t src_index;
    for (size_t dest_index = 0; dest_index < len / (double) type_ratio;
            dest_index++) {
        size_t m = dest_index * type_ratio;
        if (dest_index == 0) {
            src_index = offset;
        } else {
            src_index = 0;
        }
        for (; src_index < type_ratio; src_index++) {
            *(d + dest_index)|= (*(s + ((src_index - offset) + m))
                << (((type_ratio - src_index) - 1) * sizeof_s * 8));
        }
    }

    return ESP_OK;
}

/**
 * Converts a uint8_t array s to a uint32_t array d and left-aligns the
 * data in d
 *   i.e. {0x01, 0x02, 0x03} -> {0x01020300} 
 * 
 * NOTICE: This function uses pointers and pointer/offset arithmetic. It is
 * absolutely not safe. However many elements (in this case bytes) of the source
 * array are specified by len to be copied is how many this function will
 * attempt to copy. If len is larger than the actual size of s, undefined
 * behavior and/or a segfault is/are absolutely possible. USE WITH CARE.
 * 
 * @param s source array
 * @param len len of source array
 * @param d destination array
 * @return ESP_OK
 */
esp_err_t conv_arr_uint8_uint32_left(uint8_t* s, uint32_t len, uint32_t* d) {

    // FIXME Possibly hardcode for speed?
    uint8_t sizeof_s = sizeof(*s);
    uint8_t sizeof_d = sizeof(*d);

    // Get ratio of destination data width to source data width
    uint32_t type_ratio = sizeof_d / sizeof_s;

    // Size of destination array in bytes
    size_t dest_size = (size_t) sizeof_d * ceil(len / (double) type_ratio);

    // Clear all memory in array range
    memset(d, 0x0, dest_size);

    // Copy source array into destination array
    for (size_t dest_index = 0; dest_index < len / (double) type_ratio;
            dest_index++) {
        size_t m = dest_index * type_ratio;
        for (size_t src_index = 0; src_index < type_ratio; src_index++) {
            if (m + src_index >= len) {
                break;
            }
            *(d + dest_index) |= (*(s + (src_index + m))
                << (((type_ratio - src_index) - 1) * sizeof_s * 8));
        }
    }

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
    //FIXME not implemented
    return ESP_OK;
}

/**
 * Initializes the display with default settings
 * 
 * @return ESP_OK if initialization successful, else ESP_FAIL
 */
esp_err_t init_display() {

    //FIXME not implement

    ESP_LOGI(TAG, "init spi");

    // Ensure SPI initializes correctly
    if (init_spi() != ESP_OK) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "spi init successful");

    ESP_LOGI(TAG, "getting display info");

    disp_id_info_t disp_id;

    ili_get_info(&disp_id);

    return ESP_OK;
}