add_library(ShuTongWenServices STATIC
)

target_include_directories(ShuTongWenServices
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenServices
    PRIVATE
        ShuTongWenUtils
        spdlog::spdlog
)