#include "Display.h"
#include "hsm.hpp"
#include "config.h"
#include "Motor.h"

#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG { "Debug" };

const int colorR { 80 };
const int colorG { 10 };
const int colorB { 200 };

/* @brief Convert microseconds to milliseconds */
uint32_t millis() {
    return esp_timer_get_time() / 1000;  
}

Display display(RGB_ADDR, LCD_COLS, LCD_ROWS);

/* @brief initialize the LCD and master IIC */
void setup() {
    display.init();
    display.setRGB(colorR, colorG, colorB);

    ESP_LOGD(TAG, "hello, world!");

    vTaskDelay(pdMS_TO_TICKS(100));
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Initializing LCD...");
    display.init();
    display.setRGB(colorR, colorG, colorB);
    ESP_LOGI(TAG, "LCD initialized!");
 
    // Hand the display to the state machine and run it forever.
    hsm_init(display);
    while (true) {
        hsm_run();
    }
}
