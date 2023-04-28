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

    constexpr static const uint8_t unlocked_mode[45] = {
        FX_MODE_BREATH,              // 2
        FX_MODE_COLOR_WIPE,          // 3
        FX_MODE_RANDOM_COLOR,        // 5
        FX_MODE_RAINBOW_CYCLE,       // 9
        FX_MODE_FADE,                // 12
        FX_MODE_THEATER_CHASE,       // 13
        FX_MODE_RUNNING_LIGHTS,      // 15
        FX_MODE_HYPER_SPARKLE,       // 22
        FX_MODE_CHASE_RAINBOW,       // 30
        FX_MODE_CHASE_RAINBOW_WHITE, // 33
        FX_MODE_RUNNING_COLOR,       // 37
        FX_MODE_LARSON_SCANNER,      // 40
        FX_MODE_COMET,               // 41
        FX_MODE_POLICE_ALL,          // 49
        FX_MODE_TWO_AREAS,           // 51
        FX_MODE_CIRCUS_COMBUSTUS,    // 52
        FX_MODE_TRICOLOR_FADE,       // 56
        FX_MODE_MULTI_COMET,         // 59
        FX_MODE_OSCILLATE,           // 62
        FX_MODE_PRIDE_2015,          // 63
        FX_MODE_PALETTE,             // 65
        FX_MODE_FIRE_2012,           // 66
        FX_MODE_COLORWAVES,          // 67
        FX_MODE_BPM,                 // 68
        FX_MODE_NOISE16_1,           // 70
        FX_MODE_NOISE16_2,           // 71
        FX_MODE_NOISE16_3,           // 72
        FX_MODE_NOISE16_4,           // 73
        FX_MODE_LAKE,                // 75
        FX_MODE_RAILWAY,             // 78
        FX_MODE_TWINKLEFOX,          // 80
        FX_MODE_SPOTS_FADE,          // 86
        FX_MODE_GLITTER,             // 87
        FX_MODE_STARBURST,           // 89
        FX_MODE_EXPLODING_FIREWORKS, // 90
        FX_MODE_BOUNCINGBALLS,       // 91
        FX_MODE_SINELON,             // 92
        FX_MODE_SINELON_RAINBOW,     // 94
        FX_MODE_POPCORN,             // 95
        FX_MODE_DRIP,                // 96
        FX_MODE_PLASMA,              // 97
        FX_MODE_PACIFICA,            // 101
        FX_MODE_SINEWAVE,            // 108
        FX_MODE_CHUNCHUN,            // 111
        FX_MODE_DANCING_SHADOWS,     // 112
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
