add_library(ShuTongWenData STATIC
    UserPhrase.cpp
)

target_include_directories(ShuTongWenData
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(ShuTongWenData
    PRIVATE
        ShuTongWenUtils
        spdlog::spdlog
        ${SQLITE3_LIBRARY}
)