add_library(ShuTongWenCore STATIC
    ShuTongWenCore.cpp
)

target_include_directories(ShuTongWenCore
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenCore
    PRIVATE
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)