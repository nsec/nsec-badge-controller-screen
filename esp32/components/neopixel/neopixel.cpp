#include "neopixel.h"

#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdio.h>

#include "FX.h"
#include "FastLED.h"
#include "save.h"

WS2812FX NeoPixel::_ws2812fx;
TaskHandle_t NeoPixel::_displayTaskHandle;
bool NeoPixel::is_on;
AnalogController<LED_PIN_R, LED_PIN_G, LED_PIN_B> NeoPixel::_controller;

void NeoPixel::init()
{
    FastLED.addLeds(&NeoPixel::_controller, &_led, 1, 0);
    NeoPixel::_ws2812fx.init(NUM_LEDS, &_led, false);

    if (Save::save_data.neopixel_is_on) {
        NeoPixel::start();
    }
}

void NeoPixel::start() {
    printf("Starting neopixel");
    setColor(Save::save_data.neopixel_color);
    setBrightness(Save::save_data.neopixel_brightness);
    setMode(Save::save_data.neopixel_mode);
}
void NeoPixel::stop()
{
    if (NeoPixel::_displayTaskHandle) {
        NeoPixel::is_on = false;
    }
    NeoPixel::_ws2812fx.setBrightness(0);
    FastLED.setBrightness(0);
    Save::save_data.neopixel_is_on = false;
}

void NeoPixel::setColor(int color)
{
    CRGB c = CRGB(color);
    NeoPixel::setColor(c);
}

void NeoPixel::setColor(CRGB color)
{
    int _rgb = (color.r << 16) | (color.g << 8) | color.b;
    NeoPixel::_ws2812fx.setColor(0, _rgb);
    _led = color;
    Save::save_data.neopixel_color = _rgb;
}

void NeoPixel::setBrightness(uint8_t brightness)
{
    if (brightness <= 0xff && brightness >= 0) {
        FastLED.setBrightness(brightness);
        NeoPixel::_ws2812fx.setBrightness(brightness);
        _brightness = brightness;
        Save::save_data.neopixel_brightness = brightness;
    }
}

void NeoPixel::setMode(uint8_t mode)
{
    if (NeoPixel::_displayTaskHandle) {
        NeoPixel::is_on = false;
    }
    Save::save_data.neopixel_mode = mode;
    NeoPixel::_ws2812fx.setMode(0, mode);
    NeoPixel::is_on = true;
    xTaskCreate(&(NeoPixel::displayPatterns), "display_patterns", 1024, NULL, 5,
                &NeoPixel::_displayTaskHandle);

    _mode = mode;
}

uint8_t NeoPixel::getBrightness()
{
    return _brightness;
}

uint16_t NeoPixel::getMode()
{
    return _mode;
}

int NeoPixel::getColor()
{
    return _led;
}

void neopixel_set_brightness(uint8_t brightness)
{
    NeoPixel::getInstance().setBrightness(brightness);
}
void neopixel_set_mode(uint16_t mode)
{
    NeoPixel::getInstance().setMode(mode);
}
void neopixel_set_color(CRGB color)
{
    NeoPixel::getInstance().setColor(color);
}
