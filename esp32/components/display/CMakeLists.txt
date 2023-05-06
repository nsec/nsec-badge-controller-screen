idf_component_register(SRCS "display.cpp" "screens/splash.cpp" "screens/debug.cpp" "screens/assistant.cpp" "lv_utils.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver lvgl lvgl_esp32_drivers esp_event lv_examples esp_timer lvgl_tft lvgl_touch buzzer save badge_mesh neopixel disk fatfs wifi
)
