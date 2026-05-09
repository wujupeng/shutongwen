add_library(ShuTongWenAI STATIC
)

target_include_directories(ShuTongWenAI
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenAI
    PRIVATE
        ShuTongWenUtils
        spdlog::spdlog
)