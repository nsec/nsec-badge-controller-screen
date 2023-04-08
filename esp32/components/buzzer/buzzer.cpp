#include <esp_system.h>
#include <esp_log.h>

#include "buzzer.h"

static const char *TAG = "buzzer";

TaskHandle_t Buzzer::_taskHandle = NULL;

void Buzzer::init()
{
    xTaskCreate((TaskFunction_t)&(Buzzer::task), "buzzer", 4096, this, 5, &Buzzer::_taskHandle);
}

void Buzzer::play(Sounds sound)
{
    if (_sound != Sounds::None) {
        ESP_LOGE(TAG, "Not playing %u, there is already a sound playing", sound);
        return;
    }
    if (sound >= Sounds::ListCount || sound < 0) {
        ESP_LOGE(TAG, "Not playing %u, there are only %u sounds", sound, Sounds::ListCount);
        return;
    }

    _sound = sound;
}

void Buzzer::stopPlaying()
{
    if(_sound != Sounds::None) {
        _sound = Sounds::None;
    }
}

void Buzzer::buzz(long frequency, long length)
{
    _buzzFrequency = frequency;
    _buzzLength = length;

    play(Sounds::Buzz);
}

void Buzzer::_buzz(long frequency, long length)
{
    _ledc_timer.freq_hz = frequency * 2;

    ledc_timer_config(&_ledc_timer);
    ledc_channel_config(&_ledc_channel);

    // Wait for 1 second
    vTaskDelay(pdMS_TO_TICKS(length));

    // Stop LEDC channel
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_3, 0);

    // Turn off buzzer
    gpio_set_level(BUZZER_GPIO_NUM, 0);
}

void Buzzer::taskHandler()
{
    gpio_set_direction(BUZZER_GPIO_NUM, GPIO_MODE_OUTPUT);


    while(true) {
        int sound = getInstance().getSound();
        if (sound != Buzzer::Sounds::None) {
            switch(sound) {
                case Buzzer::Sounds::Beep:
                {
                    _buzz(880, 100);
                    break;
                }
                case Buzzer::Sounds::Buzz:
                {
                    _buzz(_buzzFrequency, _buzzLength);
                    break;
                }
            }
            getInstance().stopPlaying();
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(Buzzer::_taskHandle);
}

void buzzer_beep()
{
    Buzzer::getInstance().beep();
}
