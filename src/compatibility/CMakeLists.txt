add_library(compatibility STATIC
    CompatibilityTest.cpp
)

target_include_directories(compatibility PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/include
)

target_link_libraries(compatibility PRIVATE
    utils
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

install(TARGETS compatibility
    ARCHIVE DESTINATION lib
)