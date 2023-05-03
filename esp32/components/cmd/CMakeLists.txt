idf_component_register(SRCS "console.cpp" "cmd_debug.cpp" "cmd_neopixel.cpp" "cmd_flag.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES console driver fatfs cmd_nvs cmd_system buzzer badge_mesh save neopixel
)
