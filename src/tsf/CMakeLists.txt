add_library(ShuTongWenTSF SHARED
    TSFTextInputProcessor.cpp
    TSFComInterfaces.cpp
    TextLayoutTracker.cpp
)

target_include_directories(ShuTongWenTSF
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenTSF
    PRIVATE
        ShuTongWenCore
        ShuTongWenUtils
        ShuTongWenPlatform
        spdlog::spdlog
)