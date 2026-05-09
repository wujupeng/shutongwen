add_library(ShuTongWenIPC STATIC
    PipeServer.cpp
    PipeClient.cpp
    IPCMessage.cpp
)

target_include_directories(ShuTongWenIPC
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ShuTongWenIPC
    PRIVATE
        ShuTongWenUtils
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)