add_library(ShuTongWenUI STATIC
    CandidateWindow.cpp
    CompositionWindow.cpp
    IMEWindow.cpp
)

target_include_directories(ShuTongWenUI
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenUI
    PRIVATE
        ShuTongWenCore
        spdlog::spdlog
)