set(SOURCES
    "badge/mesh/config.cpp"
    "badge/mesh/host.cpp"
    "badge/mesh/main.cpp"
    "badge/mesh/models.cpp"
    "badge/mesh/network.cpp"
    "badge/mesh/cmd/provision.cpp"
    "badge/mesh/ops.cpp"
    "badge/mesh/ops/ping.cpp"
    "badge/mesh/ops/realtimeled.cpp"
    "badge/mesh/ops/set_name.cpp"
    "badge/mesh/ops/census.cpp"
    "badge/mesh/ops/ui_message.cpp"
    "badge/mesh/ops/info.cpp"
    "badge/mesh/ops/neopixel.cpp"
    "badge/mesh/ops/time.cpp"
    "badge/mesh/ops/partyline.cpp"
    "badge/mesh/ops/flag.cpp"
)

if(CONFIG_BADGE_MESH_ADMIN_COMMANDS)
    list(APPEND SOURCES "badge/mesh/cmd/admin.cpp")
endif()


idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."
    REQUIRES driver bt mbedtls console nvs_flash display cmd
)
