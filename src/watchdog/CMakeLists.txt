add_library(watchdog STATIC
    Watchdog.cpp
)

target_include_directories(watchdog PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/include
)

target_link_libraries(watchdog PRIVATE
    utils
    spdlog::spdlog
)

install(TARGETS watchdog
    ARCHIVE DESTINATION lib
)