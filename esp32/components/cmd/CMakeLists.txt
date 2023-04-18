idf_component_register(SRCS "console.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES console driver fatfs cmd_nvs cmd_system buzzer badge_mesh
)
