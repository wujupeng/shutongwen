add_library(testing STATIC
    golden/GoldenPath.cpp
)

target_include_directories(testing PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/include
)

target_link_libraries(testing PRIVATE
    utils
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

install(TARGETS testing
    ARCHIVE DESTINATION lib
)