#pragma once

#include "FX.h"
#include "FastLED.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <driver/ledc.h>
#include <driver/gpio.h>


#define NUM_LEDS 1
#define LED_PIN_R GPIO_NUM_4
#define LED_PIN_G GPIO_NUM_16
#define LED_PIN_B GPIO_NUM_17

template <gpio_num_t PIN_R, gpio_num_t PIN_G, gpio_num_t PIN_B, EOrder RGB_ORDER = RGB>
class AnalogController : public CPixelLEDController<RGB_ORDER> {
	inline void writeLed(uint8_t r, uint8_t g, uint8_t b) __attribute__((always_inline)) {
        //printf("settin LED %u %u %u %u\n", brightness, r, g, b);
        gpio_set_level(PIN_R, r ? 0 : 1);
        gpio_set_level(PIN_G, g ? 0 : 1);
        gpio_set_level(PIN_B, b ? 0 : 1);

        //ledcWrite(LED_CHANNEL, 255); // max brightness = 0, min brightness = 255
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255 - r);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255 - g);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 255 - b);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
    }

public:
	AnalogController() {}

	virtual void init() {
        gpio_set_direction(PIN_R, GPIO_MODE_OUTPUT);
        gpio_set_direction(PIN_G, GPIO_MODE_OUTPUT);
        gpio_set_direction(PIN_B, GPIO_MODE_OUTPUT);

        ledc_timer_config(&ledc_timer);
        ledc_channel_config(&ledc_channel_r);
        ledc_channel_config(&ledc_channel_g);
        ledc_channel_config(&ledc_channel_b);

        writeLed(0, 0, 0);
	}

protected:

	virtual void showPixels(PixelController<RGB_ORDER> & pixels) {
		while(pixels.has(1)) {
			writeLed(pixels.loadAndScale0(), pixels.loadAndScale1(), pixels.loadAndScale2());
			pixels.advanceData();
			pixels.stepDithering();
		}
	}

private:
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,     // Use low-speed channel
        .duty_resolution = LEDC_TIMER_8_BIT,  // PWM resolution
        .timer_num = LEDC_TIMER_0,             // Use timer 0
        .freq_hz = 2000,                       // PWM frequency
        .clk_cfg = LEDC_AUTO_CLK,              // Use auto-select clock source
    };

    // Configure the LEDC channel for output on a specific GPIO pin
   ledc_channel_config_t ledc_channel_r = {
        .gpio_num = (int)PIN_R,               // Use GPIO 12
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
    };

    // Configure the LEDC channel for output on a specific GPIO pin
   ledc_channel_config_t ledc_channel_g = {
        .gpio_num = (int)PIN_G,               // Use GPIO 12
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
    };

    // Configure the LEDC channel for output on a specific GPIO pin
   ledc_channel_config_t ledc_channel_b = {
        .gpio_num = (int)PIN_B,               // Use GPIO 12
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_2,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
    };
};

class NeoPixel
{
  public:
    static NeoPixel &getInstance()
    {
        static NeoPixel instance;
        return instance;
    }

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
