add_library(ShuTongWenUtils STATIC
    logger.cpp
    config.cpp
    string_utils.cpp
    win32_utils.cpp
)

target_include_directories(ShuTongWenUtils
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenUtils
    PRIVATE
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)