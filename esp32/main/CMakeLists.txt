idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)
