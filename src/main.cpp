#include "Display.h"
#include "hsm.hpp"

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/i2c_master.h"
#include "esp_rom_sys.h"

static const char *TAG { "Debug" };

const int colorR { 80 };
const int colorG { 10 };
const int colorB { 200 };

uint32_t millis() {
    return esp_timer_get_time() / 1000;  // Convert microseconds to milliseconds
}

// Setup LCD with LCD_COLS characters and LCD_ROWS lines of show
#define RGB_ADDR 0x2D
#define LCD_COLS 16
#define LCD_ROWS 2

Display display(RGB_ADDR, LCD_COLS, LCD_ROWS);

/**
 *  @brief initialize the LCD and master IIC
 */
void setup() {
    display.init();
    display.setRGB(colorR, colorG, colorB);

    ESP_LOGD(TAG, "hello, world!");

    vTaskDelay(pdMS_TO_TICKS(100));
}

void scanning() {
    display.setCursor(0, 0);
    display.write("Scanning");
    
    for (int i = 9; i < 12; i++) {
        vTaskDelay(pdMS_TO_TICKS(700));
        display.setCursor(i, 0);
        display.write(".");
    }

    ESP_LOGI(TAG, "Display updated!");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void detected() {
    display.setCursor(0, 0);
    display.write("IR Detected");
    
    ESP_LOGI(TAG, "Display updated!");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief set cursor position
 * @param col columns optional range 0-15
 * @param row rows optional range 0-1，0 is the first row, 1 is the second row
 */
// extern "C" void app_main(void) {
//     ESP_LOGI(TAG, "Initializing LCD...");
//     display.init();
//     display.setRGB(colorR, colorG, colorB);
//     ESP_LOGI(TAG, "LCD initialized!");
//
//     while (true) {
//         scanning();
//         display.clear();
//         detected();
//         display.clear();
//     }
//
//     return;
// }

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Initializing LCD...");
    display.init();
    display.setRGB(colorR, colorG, colorB);
    ESP_LOGI(TAG, "LCD initialized!");
 
    // Hand the display to the state machine and run it forever.
    sm_init(display);
    while (true) {
        sm_run();
    }
}
