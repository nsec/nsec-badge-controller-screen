#include <esp_system.h>
#include <esp_log.h>

#include "buzzer.h"
#include "sounds.h"

static const char *TAG = "buzzer";

TaskHandle_t Buzzer::_taskHandle = NULL;

void Buzzer::init()
{
    xTaskCreate((TaskFunction_t)&(Buzzer::task), "buzzer", 1024, this, 5, &Buzzer::_taskHandle);
}

void Buzzer::play(Sounds sound)
{
    if (_sound != Sounds::None) {
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
    _buzz(frequency, length, 0);
}

void Buzzer::_buzz(long frequency, long length, long silenceLength)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = (uint32_t)frequency, // LEDC uses half the frequency for square waves
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_channel_config_t ledc_channel = {
        .gpio_num = BUZZER_GPIO_NUM,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_3,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = (uint32_t)(1 << ledc_timer.duty_resolution) / 2, // 50% duty cycle
        .hpoint = 0,
        .flags = {
            .output_invert = 0,
        },
    };

    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);

    // pause during sound
    vTaskDelay(pdMS_TO_TICKS(length));

    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_3, 0);
    gpio_set_level(BUZZER_GPIO_NUM, 0);

    if(silenceLength > 0) {
        vTaskDelay(pdMS_TO_TICKS(silenceLength));
    }
}
void Buzzer::_bendTones(float initFrequency, float finalFrequency, float prop, long noteDuration, int silentDuration)
{
  if (initFrequency < finalFrequency) {
    for (int i=initFrequency; i<finalFrequency; i=i*prop) {
      _buzz(i, noteDuration, silentDuration);
    }
  } else {
    for (int i=initFrequency; i>finalFrequency; i=i/prop) {
      _buzz(i, noteDuration, silentDuration);
    }
  }
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
                    _buzz(400, 100);
                    break;
                }
                case Buzzer::Sounds::Buzz:
                {
                    _buzz(_buzzFrequency, _buzzLength);
                    break;
                }
                case Buzzer::Sounds::Connection:
                {
                    _buzz(NOTE_E5,50,30);
                    _buzz(NOTE_E6,55,25);
                    _buzz(NOTE_A6,60,10);
                    break;
                }

                case Buzzer::Sounds::Disconnection:
                    _buzz(NOTE_E5,50,30);
                    _buzz(NOTE_A6,55,25);
                    _buzz(NOTE_E6,50,60);
                    break;

                case Buzzer::Sounds::ButtonPushed:
                    _bendTones (NOTE_E6, NOTE_G6, 1.03, 20, 2);
                    vTaskDelay(pdMS_TO_TICKS(30));
                    _bendTones (NOTE_E6, NOTE_D7, 1.04, 10, 2);
                    break;

                case Buzzer::Sounds::Mode1:
                    _bendTones (NOTE_E6, NOTE_A6, 1.02, 30, 10);    //1318.51 to 1760
                    break;

                case Buzzer::Sounds::Mode2:
                    _bendTones (NOTE_G6, NOTE_D7, 1.03, 30, 10);    //1567.98 to 2349.32
                    break;

                case Buzzer::Sounds::Mode3:
                    _buzz(NOTE_E6,50,100);   //D6
                    _buzz(NOTE_G6,50,80);    //E6
                    _buzz(NOTE_D7,300,0);    //G6
                    break;

                case Buzzer::Sounds::Surprise:
                    _bendTones(800, 2150, 1.02, 10, 1);
                    _bendTones(2149, 800, 1.03, 7, 1);
                    break;

                case Buzzer::Sounds::OhOoh:
                    _bendTones(1880, 3000, 1.03, 8, 3);
                    vTaskDelay(pdMS_TO_TICKS(200));

                    for (int i=1880; i<3000; i=i*1.03) {
                        _buzz(NOTE_C6,10,10);
                    }
                    break;

                case Buzzer::Sounds::Cuddly:
                    _bendTones(700, 900, 1.03, 16, 4);
                    _bendTones(899, 650, 1.01, 18, 7);
                    break;

                case Buzzer::Sounds::Sleeping:
                    _bendTones(100, 500, 1.04, 10, 10);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    _bendTones(400, 100, 1.04, 10, 1);
                    break;

                case Buzzer::Sounds::Happy:
                    _bendTones(1500, 2500, 1.05, 20, 8);
                    _bendTones(2499, 1500, 1.05, 25, 8);
                    break;

                case Buzzer::Sounds::SuperHappy:
                    _bendTones(2000, 6000, 1.05, 8, 3);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    _bendTones(5999, 2000, 1.05, 13, 2);
                    break;

                case Buzzer::Sounds::HappyShort:
                    _bendTones(1500, 2000, 1.05, 15, 8);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    _bendTones(1900, 2500, 1.05, 10, 8);
                    break;

                case Buzzer::Sounds::Sad:
                    _bendTones(880, 669, 1.02, 20, 200);
                    break;
                case Buzzer::Sounds::ImportantNotice:
                {
                    _buzz(NOTE_D8, 300, 300);
                    _buzz(NOTE_D8, 300, 300);
                    _buzz(NOTE_D8, 300, 300);
                    break;
                }
                case Buzzer::Sounds::LevelUp:
                {
                    _bendTones(NOTE_A4, NOTE_A5, 1.05, 60, 10);
                    break;
                }
                case Buzzer::Sounds::LevelDown:
                {
                    _bendTones(NOTE_A5, NOTE_A4, 1.05, 60, 10);
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
