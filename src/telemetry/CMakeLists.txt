add_library(telemetry STATIC
    LatencyTracker.cpp
    CrashReporter.cpp
    PerformanceCounter.cpp
)

target_include_directories(telemetry PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/include
)

target_link_libraries(telemetry PRIVATE
    utils
    spdlog::spdlog
)

install(TARGETS telemetry
    ARCHIVE DESTINATION lib
)