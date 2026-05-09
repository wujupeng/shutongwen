add_executable(ShuTongWenTests
    IMEFrameworkTest.cpp
)

target_include_directories(ShuTongWenTests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${GTEST_INCLUDE_DIR}
)

target_link_libraries(ShuTongWenTests
    PRIVATE
        ShuTongWenCore
        ShuTongWenIME
        ShuTongWenUtils
        gtest
        gtest_main
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)

include(GoogleTest)
gtest_discover_tests(ShuTongWenTests)