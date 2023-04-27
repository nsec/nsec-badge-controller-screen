#pragma once

#include "FX.h"
#include "FastLED.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <driver/gpio.h>

#include "fastled_analog_controller.h"

#define NUM_LEDS 1
#define LED_PIN_R GPIO_NUM_4
#define LED_PIN_G GPIO_NUM_16
#define LED_PIN_B GPIO_NUM_17

class NeoPixel
{
  public:
    static NeoPixel &getInstance()
    {
        static NeoPixel instance;
        return instance;
    }

    constexpr static const uint8_t unlocked_mode[10] = {
        FX_MODE_BREATH,
        FX_MODE_RAINBOW,
        FX_MODE_RANDOM_COLOR,
        FX_MODE_CHASE_RAINBOW,
        FX_MODE_RAIN,
        FX_MODE_PRIDE_2015,
        FX_MODE_COLORWAVES,
        FX_MODE_SPARKLE,
        FX_MODE_RUNNING_COLOR,
        FX_MODE_RAINBOW_CYCLE
    };

  private:
    NeoPixel()
    {
    }

    CRGB _led;
    uint8_t _brightness;
    uint16_t _mode;
    static bool is_on;
    static WS2812FX _ws2812fx;
    static AnalogController<LED_PIN_R, LED_PIN_G, LED_PIN_B> _controller;
    static TaskHandle_t _displayTaskHandle;

  public:
    NeoPixel(NeoPixel const &) = delete;
    void operator=(NeoPixel const &) = delete;

    static void displayPatterns(void *pvParameters)
    {
        while (NeoPixel::is_on) {
            NeoPixel::_ws2812fx.service();
            vTaskDelay(10 / portTICK_PERIOD_MS); /*10ms*/
        }
        vTaskDelete(NULL);
    }
    void init();
    void stop();
    void start();
    void setBrightness(uint8_t brightness);
    void setMode(uint8_t mode);
    void setColor(int color);
    void setColor(CRGB color);
    uint8_t getBrightness();
    uint16_t getMode();
    int getColor();
};
