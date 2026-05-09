add_library(ShuTongWenIME SHARED
    IMEFramework.cpp
    IMEModule.cpp
    InputProcessor.cpp
    PinyinParser.cpp
    DictionaryManager.cpp
    ShuTongWenIME.cpp
    InputSession.cpp
    pipeline/InputPipeline.cpp
    tracing/InputTrace.cpp
    replay/InputReplay.cpp
    semantic/InputSemanticLayer.cpp
    cache/InputCache.cpp
    adaptive/AdaptivePipeline.cpp
    perception/LatencyOptimizer.cpp
)

target_include_directories(ShuTongWenIME
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${SQLITE3_INCLUDE_DIR}
)

target_link_libraries(ShuTongWenIME
    PRIVATE
        ShuTongWenCore
        ShuTongWenUtils
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        ${SQLITE3_LIBRARY}
)