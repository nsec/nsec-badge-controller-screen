idf_component_register(SRCS "badge/mesh/config.cpp" "badge/mesh/host.cpp" "badge/mesh/main.cpp" "badge/mesh/models.cpp" "badge/mesh/network.cpp" "badge/mesh/ops.cpp" "badge/mesh/ops/ping.cpp" "badge/mesh/ops/pong.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES driver bt mbedtls
)
