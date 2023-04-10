idf_component_register(SRCS "display.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver lvgl lvgl_esp32_drivers esp_event lv_examples esp_timer lvgl_tft lvgl_touch
)
