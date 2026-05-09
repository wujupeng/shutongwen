add_library(ShuTongWenPlatform STATIC
    Win11Platform.cpp
    caret/CaretLocator.cpp
    caret/DpiTransformer.cpp
    caret/WindowTracker.cpp
    profiles/WindowProfile.cpp
)

target_include_directories(ShuTongWenPlatform
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenPlatform
    PRIVATE
        ShuTongWenUtils
        spdlog::spdlog
)