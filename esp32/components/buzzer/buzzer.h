#pragma once

#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdio.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#define BUZZER_GPIO_NUM GPIO_NUM_26

#ifdef __cplusplus
extern "C" {
#endif

class Buzzer
{
  public:
    static Buzzer &getInstance()
    {
        static Buzzer instance;
        return instance;
    }

    enum Sounds {
        None = 0,
        Beep,
        Buzz, // same as beep but configurable frequency and length via buzz(freq, length);

        ListCount,
    };

  private:
    Buzzer()
    {
    }

    int _sound;
    static TaskHandle_t _taskHandle;

    void _buzz(long frequency, long length);
    long _buzzFrequency;
    long _buzzLength;

    ledc_timer_config_t _ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = 100 * 2, // LEDC uses half the frequency for square waves
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_channel_config_t _ledc_channel = {
        .gpio_num = BUZZER_GPIO_NUM,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_3,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = (1 << LEDC_TIMER_13_BIT) / 2, // 50% duty cycle
        .hpoint = 0,
        .flags = 0,
    };

  public:
    Buzzer(Buzzer const &) = delete;
    void operator=(Buzzer const &) = delete;

    void init();
    static void task(Buzzer *instance) {
        instance->taskHandler();
    }
    void taskHandler();
    void play(Sounds sound);
    void beep() { play(Sounds::Beep); }
    void buzz(long frequency, long length);
    void stopPlaying();
    int getSound() { return _sound; }
};

#ifdef __cplusplus
}
#endif

void buzzer_beep();
