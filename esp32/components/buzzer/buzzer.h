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

        // From https://github.com/GypsyRobot/CuteBuzzerSounds/blob/master/src/CuteBuzzerSounds.cpp
        Connection,
        Disconnection,
        ButtonPushed,
        Mode1,
        Mode2,
        Mode3,
        Surprise,
        OhOoh,
        Cuddly,
        Sleeping,
        Happy,
        SuperHappy,
        HappyShort,
        Sad,

        ImportantNotice,
        LevelUp,
        LevelDown,

        ListCount,
    };

  private:
    Buzzer()
    {
    }

    void _buzz(long frequency, long length);
    void _buzz(long frequency, long length, long silenceLength);
    long _buzzFrequency;
    long _buzzLength;
    void _bendTones(float initFrequency, float finalFrequency, float prop, long noteDuration, int silentDuration);

  public:
    Buzzer(Buzzer const &) = delete;
    void operator=(Buzzer const &) = delete;

    void init();
    void play(Sounds sound);
    void beep() { play(Sounds::Beep); }
    void buzz(long frequency, long length);
    void stopPlaying();
};

#ifdef __cplusplus
}
#endif

void buzzer_beep();
