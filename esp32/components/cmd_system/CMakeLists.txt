idf_component_register(SRCS "cmd_system.cpp"
                       INCLUDE_DIRS .
                       REQUIRES console spi_flash driver save
)
